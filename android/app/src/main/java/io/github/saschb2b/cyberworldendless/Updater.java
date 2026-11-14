package io.github.saschb2b.cyberworldendless;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInstaller;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.SystemClock;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * The game's own update on Android (src/update/update_android.c,
 * docs/engine/updates.md): the release's update.json fetched for the game
 * to read, and on A the APK it names downloaded, its size and SHA-256
 * checked against update.json, written into a PackageInstaller session and
 * committed. From Android 12 on, once the player has let the app install
 * apps (Android asks the first time), the update goes in without a tap:
 * the app updates itself (UPDATE_PACKAGES_WITHOUT_USER_ACTION); before
 * that, or where Android wants the player's word all the same, its own
 * prompt comes up over the game. Android ends the game as it puts the new
 * version in place (from Android 10 on the player opens it again: an app
 * may not open itself from the background; UpdateReceiver does before).
 * The game polls from its thread (GameActivity's update* methods); the
 * work runs on a thread of its own, one thing at a time.
 */
final class Updater {
    /** updateProgress beyond a download's share: the installer has it, or failed. */
    static final int INSTALLING = 101, FAILED = -1;
    /** The marker UpdateReceiver reads after the new version is in place. */
    static final String MARK = "update-started";
    private static final String STATUS = "io.github.saschb2b.cyberworldendless.UPDATE_STATUS";
    private static final String TAG = "Cyberworld";
    private static final int LOOK_MAX = 16 * 1024;
    /* how long the line's note shows before Android ends the game */
    private static final long NOTE_MS = 2500;

    private final Activity activity;
    private final ExecutorService work = Executors.newSingleThreadExecutor();
    /* the look's answer once ("1\nupdate.json" or "0\n"), the download's
     * share or INSTALLING or FAILED, and a stop asked for */
    private volatile String looked;
    private volatile int progress;
    private volatile boolean stop;
    /* the SHA-256 of the APK checked in this start, to install it again
     * without a new download (the prompt said no, and A once more) */
    private volatile String checked;
    private BroadcastReceiver receiver;

    Updater(Activity activity) {
        this.activity = activity;
    }

    private File apk() { return new File(activity.getCacheDir(), "update.apk"); }

    /** A start's tidying: an APK and sessions an earlier start left. */
    void tidy() {
        work.execute(() -> {
            apk().delete();
            new File(activity.getFilesDir(), MARK).delete();
            PackageInstaller pi = activity.getPackageManager().getPackageInstaller();
            for (PackageInstaller.SessionInfo s : pi.getMySessions()) {
                try {
                    pi.abandonSession(s.getSessionId());
                } catch (Exception e) {
                    /* (one the system already ended) */
                }
            }
        });
    }

    /** update.json fetched from `url`: false where it was not begun. */
    boolean look(String url, String agent) {
        looked = null;
        stop = false;
        work.execute(() -> {
            String text = get(url, agent);
            looked = text != null ? "1\n" + text : "0\n";
        });
        return true;
    }

    /** The look's answer once, null while it is under way. */
    String looked() {
        String r = looked;
        if (r != null) looked = null;
        return r;
    }

    /** The APK at `url` downloaded, checked whole and handed to Android's
     *  installer: false where it was not begun. */
    boolean fetch(String url, String agent, long size, String sha256) {
        if (size <= 0 || sha256 == null) return false;
        progress = 0;
        stop = false;
        work.execute(() -> {
            boolean ok = (sha256.equals(checked) && apk().length() == size) || download(url, agent, size, sha256);
            if (ok) checked = sha256;
            ok = ok && !stop && install(apk());
            if (!ok) progress = FAILED;
        });
        return true;
    }

    int progress() { return progress; }

    /** The updates switched off: a download under way stops. */
    void stop() { stop = true; }

    void close() {
        if (receiver != null) {
            try {
                activity.unregisterReceiver(receiver);
            } catch (Exception e) {
                /* (not registered after all) */
            }
            receiver = null;
        }
        work.shutdownNow();
    }

    // ---- the work ----

    private static HttpURLConnection open(String url, String agent) throws Exception {
        HttpURLConnection c = (HttpURLConnection) new URL(url).openConnection();
        c.setConnectTimeout(10000);
        c.setReadTimeout(30000);
        c.setUseCaches(false);
        c.setRequestProperty("User-Agent", agent);
        return c;
    }

    /** A short text file, null where none came */
    private String get(String url, String agent) {
        HttpURLConnection c = null;
        try {
            c = open(url, agent);
            if (c.getResponseCode() != 200) return null;
            ByteArrayOutputStream text = new ByteArrayOutputStream();
            byte[] b = new byte[4096];
            try (InputStream in = c.getInputStream()) {
                for (int n; (n = in.read(b)) > 0; ) {
                    if (stop || text.size() + n >= LOOK_MAX) return null;
                    text.write(b, 0, n);
                }
            }
            return new String(text.toByteArray(), StandardCharsets.UTF_8);
        } catch (Exception e) {
            Log.i(TAG, "update: no update.json (" + e + ")");
            return null;
        } finally {
            if (c != null) c.disconnect();
        }
    }

    /** The APK into the cache, its digest taken as it comes: kept only whole
     *  and the release's own */
    private boolean download(String url, String agent, long size, String sha256) {
        File to = apk();
        HttpURLConnection c = null;
        boolean ok = false;
        try {
            c = open(url, agent);
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            long got = 0;
            if (c.getResponseCode() == 200) {
                byte[] b = new byte[64 * 1024];
                try (InputStream in = c.getInputStream(); OutputStream out = new FileOutputStream(to)) {
                    for (int n; (n = in.read(b)) > 0 && !stop && got + n <= size; ) {
                        out.write(b, 0, n);
                        digest.update(b, 0, n);
                        got += n;
                        progress = (int) (got * 100 / size);
                    }
                }
            }
            ok = got == size && !stop && hex(digest.digest()).equals(sha256);
            Log.i(TAG, "update: " + got + " of " + size + " bytes, " + (ok ? "checked" : "not the release's"));
        } catch (Exception e) {
            Log.i(TAG, "update: the download failed (" + e + ")");
        } finally {
            if (c != null) c.disconnect();
        }
        if (!ok) to.delete();
        return ok;
    }

    private static String hex(byte[] bytes) {
        StringBuilder s = new StringBuilder();
        for (byte b : bytes) s.append(String.format("%02x", b));
        return s.toString();
    }

    /** The APK written into a session of Android's installer and committed:
     *  without a tap where Android allows it (from 12 on, the app updating
     *  itself, installs from it allowed), else Android's prompt */
    private boolean install(File file) {
        PackageInstaller pi = activity.getPackageManager().getPackageInstaller();
        int id = -1;
        try {
            PackageInstaller.SessionParams p = new PackageInstaller.SessionParams(PackageInstaller.SessionParams.MODE_FULL_INSTALL);
            p.setAppPackageName(activity.getPackageName());
            p.setSize(file.length());
            if (Build.VERSION.SDK_INT >= 26) p.setInstallReason(PackageManager.INSTALL_REASON_USER);
            if (Build.VERSION.SDK_INT >= 31) p.setRequireUserAction(PackageInstaller.SessionParams.USER_ACTION_NOT_REQUIRED);
            id = pi.createSession(p);
            try (PackageInstaller.Session s = pi.openSession(id)) {
                try (InputStream in = new FileInputStream(file); OutputStream out = s.openWrite("base.apk", 0, file.length())) {
                    byte[] b = new byte[64 * 1024];
                    for (int n; (n = in.read(b)) > 0; ) out.write(b, 0, n);
                    s.fsync(out);
                }
                /* (where Android takes it without a tap it ends the game a
                 * second after the commit: the line's note first, for a moment) */
                progress = INSTALLING;
                if (silent()) SystemClock.sleep(NOTE_MS);
                if (stop) throw new IllegalStateException("the updates switched off");
                listen();
                Intent status = new Intent(STATUS).setPackage(activity.getPackageName());
                int flags = PendingIntent.FLAG_UPDATE_CURRENT | (Build.VERSION.SDK_INT >= 31 ? PendingIntent.FLAG_MUTABLE : 0);
                /* (the new version, once in place, opens the game again: UpdateReceiver) */
                new FileOutputStream(new File(activity.getFilesDir(), MARK)).close();
                s.commit(PendingIntent.getBroadcast(activity, id, status, flags).getIntentSender());
            }
            Log.i(TAG, "update: handed to Android's installer");
            return true;
        } catch (Exception e) {
            Log.i(TAG, "update: the installer refused it (" + e + ")");
            new File(activity.getFilesDir(), MARK).delete();
            if (id >= 0) {
                try {
                    pi.abandonSession(id);
                } catch (Exception f) {
                    /* (already gone) */
                }
            }
            return false;
        }
    }

    /** Whether Android will likely put it in place without a tap: from 12
     *  on, installs from the game allowed (it may still ask: its prompt then) */
    private boolean silent() {
        return Build.VERSION.SDK_INT >= 31 && activity.getPackageManager().canRequestPackageInstalls();
    }

    /** The installer's word on a session: its prompt shown over the game, or
     *  a failure (the player said no) for the game's line */
    private synchronized void listen() {
        if (receiver != null) return;
        receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                int status = intent.getIntExtra(PackageInstaller.EXTRA_STATUS, PackageInstaller.STATUS_FAILURE);
                Log.i(TAG, "update: the installer says " + status + " " + intent.getStringExtra(PackageInstaller.EXTRA_STATUS_MESSAGE));
                if (status == PackageInstaller.STATUS_PENDING_USER_ACTION) {
                    Intent ask = Build.VERSION.SDK_INT >= 33 ? intent.getParcelableExtra(Intent.EXTRA_INTENT, Intent.class)
                        : legacyIntent(intent);
                    try {
                        if (ask != null) activity.startActivity(ask);
                    } catch (Exception e) {
                        Log.i(TAG, "update: no prompt (" + e + ")");
                        failed();
                    }
                } else if (status != PackageInstaller.STATUS_SUCCESS) {
                    failed();
                }
            }
        };
        IntentFilter filter = new IntentFilter(STATUS);
        if (Build.VERSION.SDK_INT >= 33) activity.registerReceiver(receiver, filter, Context.RECEIVER_NOT_EXPORTED);
        else activity.registerReceiver(receiver, filter);
    }

    @SuppressWarnings("deprecation")
    private static Intent legacyIntent(Intent intent) { return intent.getParcelableExtra(Intent.EXTRA_INTENT); }

    private void failed() {
        new File(activity.getFilesDir(), MARK).delete();
        progress = FAILED;
    }
}
