package io.github.saschb2b.cyberworldendless;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.ClipData;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.UriPermission;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.StateListDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.provider.DocumentsContract;
import android.provider.DocumentsContract.Document;
import android.provider.OpenableColumns;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;

/**
 * The start: the game when the player's ROM is kept, else a page that asks
 * for the folder the ROMs are in (Android's folder picker: no permission to
 * read storage), or the files themselves in one picker. Each .gba there is
 * told by its header's game code and checked by its SHA-1: Mega Man Battle
 * Network 6: Cybeast Gregar (USA) is needed, Battle Network 5: Team Colonel
 * (USA) is optional, and those two alone are copied into the app's own
 * files, where the game finds BN5 beside BN6 (rom.c, xrom_find_beside).
 * The folder is kept, read-only, and looked in again at each start while
 * BN5 is missing, so BN5 put there later comes in by itself; the app icon's
 * ROMs shortcut opens the page again. Nothing leaves the device.
 */
public class RomActivity extends Activity {
    static final String TAG = "Cyberworld";
    static final long ROM_SIZE = 8L * 1024 * 1024;
    /** The app icon's shortcut (res/xml/shortcuts.xml): the page again, BN6 kept or not. */
    static final String ACTION_ROMS = "io.github.saschb2b.cyberworldendless.ROMS";
    static final int PICK_FOLDER = 1, PICK_FILES = 2;

    /** A ROM the game takes: its header's game code, its SHA-1, its copy's name. */
    static final class Rom {
        final String code, sha1, file, name, tag;
        Rom(String code, String sha1, String file, String name, String tag) {
            this.code = code; this.sha1 = sha1; this.file = file; this.name = name; this.tag = tag;
        }
        String changed() { return tag + ", but changed: patched, trimmed or a bad dump"; }
    }
    static final Rom[] ROMS = {
        new Rom("BR5E", "89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6", "bn6g.gba", "Mega Man Battle Network 6: Cybeast Gregar (USA)", "BN6 Cybeast Gregar (USA)"),
        new Rom("BRKE", "5f472f78d8de2df01d5039e045c043cb40969a39", "bn5c.gba", "Mega Man Battle Network 5: Team Colonel (USA)", "BN5 Team Colonel (USA)"),
    };
    static final int BN6 = 0, BN5 = 1;
    /** Battle Network 6's and 5's other versions, by their header's game
     *  code: a player who has one is told which it is (rom.c names BN6's
     *  the same way). */
    static final String[][] OTHERS = {
        { "BR6E", "BN6 Cybeast Falzar, not Gregar" },
        { "BR6P", "BN6 Cybeast Falzar (Europe), not Gregar (USA)" },
        { "BR6J", "Rockman EXE 6 Falzar (Japan), not BN6 Gregar (USA)" },
        { "BR5P", "BN6 Cybeast Gregar (Europe), not the USA version" },
        { "BR5J", "Rockman EXE 6 Gregar (Japan), not the USA version" },
        { "BRBE", "BN5 Team ProtoMan, not Team Colonel" },
        { "BRBP", "BN5 Team ProtoMan (Europe), not Team Colonel (USA)" },
        { "BRBJ", "Rockman EXE 5 Team of Blues (Japan), not BN5 Team Colonel (USA)" },
        { "BRKP", "BN5 Team Colonel (Europe), not the USA version" },
        { "BRKJ", "Rockman EXE 5 Team of Colonel (Japan), not the USA version" },
    };
    static final String OTHER_GAME = "not BN6 Gregar or BN5 Team Colonel";
    static final String[] COLUMNS = {
        Document.COLUMN_DOCUMENT_ID, Document.COLUMN_DISPLAY_NAME, Document.COLUMN_MIME_TYPE, Document.COLUMN_SIZE, Document.COLUMN_LAST_MODIFIED,
    };

    static final int NAVY = Color.rgb(7, 24, 58), INK = Color.rgb(16, 54, 74), GOLD = Color.rgb(255, 214, 16),
        PALE = Color.rgb(255, 247, 165), TEXT = Color.rgb(214, 244, 255), SOFT = Color.rgb(150, 186, 214), NOTE = Color.rgb(247, 165, 0);

    static File romDir(Context c) { return new File(c.getFilesDir(), "rom"); }
    static File dataDir(Context c) { return new File(c.getFilesDir(), "data"); }
    static File rom(Context c, int i) { return new File(romDir(c), ROMS[i].file); }
    static boolean kept(Context c, int i) { return rom(c, i).length() == ROM_SIZE; }

    private SharedPreferences prefs;
    private boolean manage, busy, skipLook;
    private LinearLayout column;
    private TextView status;
    private Button folderButton, filesButton, playButton;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        prefs = getSharedPreferences("roms", MODE_PRIVATE);
        manage = ACTION_ROMS.equals(getIntent().getAction());
        if (kept(this, BN6) && !manage) {
            /* (BN5 put in the folder since: a look first, as quick as a
             * start can afford: only new 8 MB files are opened) */
            if (folder() != null && !kept(this, BN5)) look(folder(), folderName(), null, false);
            else play();
            return;
        }
        page();
    }

    /* Back from a file manager, the folder chosen but BN6 not there: it is
     * looked in again, so a ROM moved there meanwhile comes in by itself. */
    @Override
    protected void onResume() {
        super.onResume();
        boolean skip = skipLook;
        skipLook = false;
        if (column != null && !busy && !skip && !kept(this, BN6) && folder() != null) look(folder(), folderName(), null, false);
    }

    @Override
    public void onConfigurationChanged(Configuration c) {
        super.onConfigurationChanged(c);
        if (column != null) pad();
    }

    // ---- the page ----

    private void page() {
        column = new LinearLayout(this);
        column.setOrientation(LinearLayout.VERTICAL);
        column.setGravity(Gravity.CENTER);
        pad();
        column.addView(text("Cyberworld Endless", 26, GOLD, true));
        column.addView(text("A roguelike on Mega Man Battle Network 6. It runs from your own copy of "
            + ROMS[BN6].name + ", an unmodified .gba file.", 16, TEXT, false));
        column.addView(text("Optional beside it: " + ROMS[BN5].name + ". Its net then joins ours: BN5's areas, "
            + "their battles in BN5's own engine.", 14, SOFT, false));
        if (manage) column.addView(text(keptWords(), 14, TEXT, false));
        folderButton = button(folder() != null ? "Choose another folder" : "Choose the folder with your ROMs", true, v -> pickFolder());
        column.addView(folderButton, place(20));
        column.addView(text("It is looked in again at each start, so BN5 can come later."
            + (Build.VERSION.SDK_INT >= 30 ? " Android lets no app use Download itself: a folder in it, such as Download/ROMs, works." : ""),
            13, SOFT, false));
        filesButton = button("Choose the files instead", false, v -> pickFiles());
        column.addView(filesButton, place(12));
        column.addView(text("Hold a file to choose more than one.", 13, SOFT, false));
        if (manage) {
            playButton = button("Play", false, v -> play());
            column.addView(playButton, place(12));
        }
        status = text("", 15, NOTE, false);
        status.setVisibility(View.GONE);
        column.addView(status);
        column.addView(text("Only the .gba files there are opened, and only these two ROMs are copied, into the app. "
            + "Nothing leaves this device.", 12, SOFT, false), place(16));
        ScrollView scroll = new ScrollView(this);
        scroll.setFillViewport(true);
        scroll.setBackgroundColor(NAVY);
        scroll.addView(column, new ScrollView.LayoutParams(ScrollView.LayoutParams.MATCH_PARENT, ScrollView.LayoutParams.WRAP_CONTENT));
        setContentView(scroll);
        folderButton.requestFocus();
    }

    /* (a column a line can be read across on a tablet or a phone on its side) */
    private void pad() {
        int width = getResources().getDisplayMetrics().widthPixels, side = Math.max(dp(28), (width - dp(560)) / 2);
        column.setPadding(side, dp(24), side, dp(24));
    }

    private String keptWords() {
        String s = "Kept: " + ROMS[BN6].tag + (kept(this, BN5) ? " and " + ROMS[BN5].tag + "." : ". " + ROMS[BN5].tag + ": not yet.");
        return folder() != null ? s + " Folder: " + folderName() + "." : s;
    }

    private int dp(int v) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, v, getResources().getDisplayMetrics());
    }

    private LinearLayout.LayoutParams place(int top) {
        LinearLayout.LayoutParams at = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        at.topMargin = dp(top);
        at.gravity = Gravity.CENTER_HORIZONTAL;
        return at;
    }

    private TextView text(String s, int sp, int color, boolean bold) {
        TextView t = new TextView(this);
        t.setText(s);
        t.setTextSize(TypedValue.COMPLEX_UNIT_SP, sp);
        t.setTextColor(color);
        t.setGravity(Gravity.CENTER);
        t.setPadding(0, dp(6), 0, dp(6));
        if (bold) t.setTypeface(Typeface.DEFAULT_BOLD);
        return t;
    }

    private GradientDrawable plate(int fill, int edge, int width) {
        GradientDrawable d = new GradientDrawable();
        d.setColor(fill);
        d.setCornerRadius(dp(6));
        d.setStroke(dp(width), edge);
        return d;
    }

    /* The folder's button gold, the files' quieter; a controller's focus
     * shows as a white edge (a handheld moves between them with its D-pad) */
    private Button button(String label, boolean main, View.OnClickListener click) {
        Button b = new Button(this);
        b.setText(label);
        b.setAllCaps(false);
        b.setTextColor(main ? INK : GOLD);
        b.setTypeface(Typeface.DEFAULT_BOLD);
        b.setTextSize(TypedValue.COMPLEX_UNIT_SP, main ? 17 : 15);
        b.setPadding(dp(24), dp(12), dp(24), dp(12));
        StateListDrawable look = new StateListDrawable();
        look.addState(new int[] { android.R.attr.state_pressed }, plate(main ? PALE : INK, PALE, 2));
        look.addState(new int[] { android.R.attr.state_focused }, plate(main ? GOLD : INK, Color.WHITE, 3));
        look.addState(new int[0], plate(main ? GOLD : NAVY, main ? PALE : GOLD, 2));
        b.setBackground(look);
        b.setOnClickListener(click);
        return b;
    }

    private void say(String s) {
        status.setText(s);
        status.setVisibility(s.isEmpty() ? View.GONE : View.VISIBLE);
    }

    private void enable(boolean on) {
        for (Button b : new Button[] { folderButton, filesButton, playButton }) {
            if (b == null) continue;
            b.setEnabled(on);
            b.setAlpha(on ? 1f : 0.5f);
        }
    }

    /* (a handheld's A presses the button its D-pad is on) */
    @Override
    public boolean onKeyDown(int code, KeyEvent e) {
        if (code == KeyEvent.KEYCODE_BUTTON_A && folderButton != null) {
            View on = getCurrentFocus();
            if (busy) return true;
            (on instanceof Button ? on : folderButton).performClick();
            return true;
        }
        return super.onKeyDown(code, e);
    }

    // ---- the pickers ----

    private void pickFolder() {
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        if (Build.VERSION.SDK_INT >= 26 && folder() != null) i.putExtra(DocumentsContract.EXTRA_INITIAL_URI, folder());
        try {
            startActivityForResult(i, PICK_FOLDER);
        } catch (ActivityNotFoundException e) {
            say("This device has no folder picker: choose the files instead.");
        }
    }

    private void pickFiles() {
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        i.addCategory(Intent.CATEGORY_OPENABLE);
        i.setType("*/*");
        i.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
        try {
            startActivityForResult(i, PICK_FILES);
        } catch (ActivityNotFoundException e) {
            say("This device has no file picker.");
        }
    }

    @Override
    protected void onActivityResult(int request, int result, Intent data) {
        super.onActivityResult(request, result, data);
        skipLook = true;
        if (result != RESULT_OK || data == null) {
            if (request == PICK_FOLDER) say(Build.VERSION.SDK_INT >= 30
                ? "No folder chosen. If your ROMs are in Download itself, choose the files instead." : "No folder chosen.");
            return;
        }
        if (request == PICK_FOLDER && data.getData() != null) {
            Uri tree = data.getData();
            String name = nameOf(tree);
            keepFolder(tree, name);
            look(tree, name, null, true);
        } else if (request == PICK_FILES) {
            List<Uri> files = new ArrayList<>();
            ClipData clip = data.getClipData();
            if (clip != null) for (int k = 0; k < clip.getItemCount(); ++k) files.add(clip.getItemAt(k).getUri());
            else if (data.getData() != null) files.add(data.getData());
            if (!files.isEmpty()) look(null, null, files, true);
        }
    }

    // ---- the folder kept ----

    private Uri folder() {
        String s = prefs.getString("folder", null);
        return s == null ? null : Uri.parse(s);
    }

    private String folderName() { return prefs.getString("folderName", "your ROM folder"); }

    /* The grant kept, read-only, for the looks at later starts (one held
     * before for another folder is given back). */
    private void keepFolder(Uri tree, String name) {
        Uri old = folder();
        try {
            getContentResolver().takePersistableUriPermission(tree, Intent.FLAG_GRANT_READ_URI_PERMISSION);
        } catch (SecurityException e) {
            /* (a provider whose grants can't be kept: this look only) */
            Log.w(TAG, "the folder's access could not be kept: " + e.getMessage());
            return;
        }
        if (old != null && !old.equals(tree)) {
            try { getContentResolver().releasePersistableUriPermission(old, Intent.FLAG_GRANT_READ_URI_PERMISSION); } catch (SecurityException e) { /* (gone already) */ }
        }
        prefs.edit().putString("folder", tree.toString()).putString("folderName", name).putBoolean("lostTold", false).apply();
    }

    private String nameOf(Uri tree) {
        try (Cursor c = getContentResolver().query(DocumentsContract.buildDocumentUriUsingTree(tree, DocumentsContract.getTreeDocumentId(tree)),
                new String[] { Document.COLUMN_DISPLAY_NAME }, null, null, null)) {
            if (c != null && c.moveToFirst() && !c.isNull(0)) return c.getString(0);
        } catch (Exception e) { /* (named generically) */ }
        return "your ROM folder";
    }

    private boolean granted(Uri tree) {
        for (UriPermission p : getContentResolver().getPersistedUriPermissions())
            if (p.getUri().equals(tree) && p.isReadPermission()) return true;
        return false;
    }

    // ---- looking ----

    /** What one look found: the ROMs it copied in, and each file it refused, with why. */
    static final class Look {
        String where;                                   // the folder's name; null for files picked
        String lost;                                    // the folder could not be opened: why
        int files, gba, battleNetwork;                  // files picked; .gba looked at; Battle Network ROMs refused
        final boolean[] kept = new boolean[ROMS.length];   // copied in now
        final boolean[] had = new boolean[ROMS.length];    // offered, a copy kept already
        final List<String> refused = new ArrayList<>();    // "name: why", Battle Network ROMs first
        final List<String> bnWhy = new ArrayList<>();      // why each Battle Network ROM was refused
        final List<String> zipped = new ArrayList<>();

        void refuse(String name, String why, boolean bn) {
            if (bn) {
                refused.add(battleNetwork++, name + ": " + why);
                bnWhy.add(why);
            } else refused.add(name + ": " + why);
        }
        boolean any() { for (boolean k : kept) if (k) return true; return false; }
    }

    /**
     * A look at the folder (tree, called where) or the files picked, off
     * the screen's thread. One that no player asked for, with BN6 kept, is
     * quick: only an 8 MB .gba not settled before (its document, size and
     * date) is opened, so a start stays as quick as ever.
     */
    private void look(final Uri tree, final String where, final List<Uri> files, final boolean pick) {
        busy = true;
        final boolean quick = kept(this, BN6) && !pick;
        if (status != null) {
            enable(false);
            say(files != null ? "Checking…" : "Looking in " + where + "…");
        } else {
            /* (a start's look that takes its time says what it is doing) */
            new Handler(getMainLooper()).postDelayed(() -> {
                if (!busy || column != null || isFinishing()) return;
                TextView t = text("Cyberworld Endless\n\nLooking in " + where + " for BN5…", 16, TEXT, false);
                t.setBackgroundColor(NAVY);
                setContentView(t);
            }, 400);
        }
        new Thread(() -> {
            Look l = new Look();
            Set<String> seen = new HashSet<>(prefs.getStringSet("seen", new HashSet<>()));
            int before = seen.size();
            if (files != null) {
                l.files = files.size();
                for (Uri u : files) offerPicked(u, l);
            } else {
                l.where = where;
                try {
                    if (!granted(tree) && !pick) throw new SecurityException("its access is no longer held");
                    walk(tree, DocumentsContract.getTreeDocumentId(tree), 1, quick, seen, l);
                } catch (Exception e) {
                    l.lost = e.getClass().getSimpleName() + ": " + e.getMessage();
                }
            }
            if (seen.size() != before) {
                if (seen.size() > 400) seen.clear();
                prefs.edit().putStringSet("seen", seen).apply();
            }
            runOnUiThread(() -> after(l));
        }).start();
    }

    /* The folder's files, and those of the folders directly in it. */
    private void walk(Uri tree, String doc, int depth, boolean quick, Set<String> seen, Look l) throws FileNotFoundException {
        List<String> dirs = new ArrayList<>();
        try (Cursor c = getContentResolver().query(DocumentsContract.buildChildDocumentsUriUsingTree(tree, doc), COLUMNS, null, null, null)) {
            if (c == null) throw new FileNotFoundException("its files could not be listed");
            while (c.moveToNext()) {
                String id = c.getString(0), name = c.getString(1);
                if (Document.MIME_TYPE_DIR.equals(c.getString(2))) {
                    if (depth > 0) dirs.add(id);
                    continue;
                }
                if (name == null || zipped(name, l) || !name.toLowerCase(Locale.ROOT).endsWith(".gba")) continue;
                long size = c.isNull(3) ? -1 : c.getLong(3), date = c.isNull(4) ? 0 : c.getLong(4);
                String key = id + "|" + size + "|" + date;
                ++l.gba;
                if (quick && (size != ROM_SIZE || seen.contains(key))) continue;
                if (offer(DocumentsContract.buildDocumentUriUsingTree(tree, id), name, size, l)) seen.add(key);
            }
        }
        for (String d : dirs) walk(tree, d, depth - 1, quick, seen, l);
    }

    private static boolean zipped(String name, Look l) {
        String n = name.toLowerCase(Locale.ROOT);
        if (!n.endsWith(".zip") && !n.endsWith(".7z") && !n.endsWith(".rar")) return false;
        l.zipped.add(name);
        return true;
    }

    /* A file picked: whatever its name, it is looked at (the player chose it). */
    private void offerPicked(Uri u, Look l) {
        String name = "a file";
        long size = -1;
        try (Cursor c = getContentResolver().query(u, new String[] { OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE }, null, null, null)) {
            if (c != null && c.moveToFirst()) {
                if (!c.isNull(0)) name = c.getString(0);
                if (!c.isNull(1)) size = c.getLong(1);
            }
        } catch (Exception e) { /* (named generically) */ }
        if (zipped(name, l)) return;
        ++l.gba;
        offer(u, name, size, l);
    }

    /**
     * One file: told by its header's game code; BN6's or BN5's checked by
     * its SHA-1 and copied in. True when it is settled (taken, refused, or
     * the same as a copy kept), so a quick look need not open it again;
     * false when it could not be read this time.
     */
    private boolean offer(Uri uri, String name, long size, Look l) {
        byte[] head = new byte[0xB0];
        int got = readHead(uri, head);
        if (got < 0) { l.refuse(name, "could not be read", false); return false; }
        if (got < head.length) { l.refuse(name, "not a GBA ROM", false); return true; }
        String code = new String(head, 0xAC, 4, StandardCharsets.US_ASCII);
        for (int i = 0; i < ROMS.length; ++i) {
            if (!ROMS[i].code.equals(code)) continue;
            if (kept(this, i)) { l.had[i] = true; return true; }
            String why = size >= 0 && size != ROM_SIZE ? ROMS[i].changed() : copy(uri, i);
            if (why == null) l.kept[i] = true;
            else l.refuse(name, why, true);
            return true;
        }
        for (String[] o : OTHERS) if (o[0].equals(code)) { l.refuse(name, o[1], true); return true; }
        l.refuse(name, OTHER_GAME, false);
        return true;
    }

    private int readHead(Uri uri, byte[] head) {
        try (InputStream in = getContentResolver().openInputStream(uri)) {
            if (in == null) return -1;
            int n = 0;
            for (int k; n < head.length && (k = in.read(head, n, head.length - n)) > 0; ) n += k;
            return n;
        } catch (Exception e) {
            return -1;
        }
    }

    /** Copies ROM i in from uri, its SHA-1 checked on the way; null when it is that ROM, unchanged. */
    private String copy(Uri uri, int i) {
        File dir = romDir(this), part = new File(dir, ROMS[i].file + ".part");
        if (!dir.isDirectory() && !dir.mkdirs()) return "the app could not make its ROM folder";
        try (InputStream in = getContentResolver().openInputStream(uri); OutputStream out = new FileOutputStream(part)) {
            if (in == null) return "could not be read";
            MessageDigest sha1 = MessageDigest.getInstance("SHA-1");
            byte[] buf = new byte[1 << 16];
            long n = 0;
            for (int k; (k = in.read(buf)) > 0; ) {
                out.write(buf, 0, k);
                sha1.update(buf, 0, k);
                n += k;
                if (n > ROM_SIZE) break;
            }
            StringBuilder hex = new StringBuilder();
            for (byte b : sha1.digest()) hex.append(String.format("%02x", b));
            if (n != ROM_SIZE || !ROMS[i].sha1.equals(hex.toString())) {
                part.delete();
                return ROMS[i].changed();
            }
        } catch (Exception e) {
            part.delete();
            return "could not be read: " + e.getMessage();
        }
        if (!part.renameTo(rom(this, i))) {
            part.delete();
            return "the app could not keep it";
        }
        return null;
    }

    // ---- what a look comes to ----

    private void after(Look l) {
        busy = false;
        if (isFinishing() || isDestroyed()) return;   // (backed out of meanwhile)
        if (status != null) enable(true);
        StringBuilder kept = new StringBuilder();
        for (int i = 0; i < ROMS.length; ++i) if (l.kept[i]) kept.append(kept.length() > 0 ? ", " : "").append(ROMS[i].tag);
        Log.i(TAG, (l.where != null ? "looked in " + l.where : "looked at " + l.files + " files picked") + ": " + l.gba + " .gba"
            + (kept.length() > 0 ? ", kept " + kept : "") + (l.refused.isEmpty() ? "" : ", refused " + l.refused)
            + (l.lost != null ? ", could not open it (" + l.lost + ")" : ""));
        if (l.lost != null) {
            if (status == null) {
                /* (a start: the ROMs kept play on; said once, with the way
                 * back, in the two lines Android 12 on shows of a toast) */
                if (!prefs.getBoolean("lostTold", false)) {
                    prefs.edit().putBoolean("lostTold", true).apply();
                    toast("Can't open the folder " + folderName() + "."
                        + (Build.VERSION.SDK_INT >= 25 ? " Hold the app's icon, ROMs, to choose another." : ""));
                }
                play();
                return;
            }
            say("The folder " + folderName() + " can't be opened any more: it was moved or removed, or its access was taken back. Choose it again.");
            return;
        }
        if (l.where != null) prefs.edit().putBoolean("lostTold", false).apply();
        if (folderButton != null && folder() != null) folderButton.setText("Choose another folder");
        boolean bn6 = kept(this, BN6);
        if (bn6 && (status == null || !manage || l.any())) {
            /* (a Battle Network ROM refused beside it, BN5 Team ProtoMan
             * say, is said as the game starts: the title says nothing of it) */
            if (l.battleNetwork > 0) toast("Not taken: " + l.bnWhy.get(0) + (l.battleNetwork > 1 ? " (and " + (l.battleNetwork - 1) + " more)" : ""));
            play();
            return;
        }
        say(bn6 ? nothingNew(l) : noBn6(l));
    }

    /* No BN6 yet: what was there, and why each .gba was refused. */
    private String noBn6(Look l) {
        StringBuilder s = new StringBuilder();
        if (l.where != null) s.append(l.gba == 0 ? "No .gba file in " + l.where + " or the folders in it." : "No " + ROMS[BN6].name + " in " + l.where + ".");
        else s.append(l.files == 1 ? "That is not " : "None of those is ").append(ROMS[BN6].name).append('.');
        reasons(l, s);
        if (kept(this, BN5)) s.append('\n').append(ROMS[BN5].tag).append(" is kept, for when BN6 is here.");
        return s.toString();
    }

    /* BN6 kept (the shortcut's page) and nothing new: why. */
    private String nothingNew(Look l) {
        if (kept(this, BN5)) return "Both ROMs are kept already: " + ROMS[BN6].tag + " and " + ROMS[BN5].tag + ".";
        StringBuilder s = new StringBuilder("No " + ROMS[BN5].name + (l.where != null ? " in " + l.where + " yet." : " there."));
        reasons(l, s);
        if (l.where != null) s.append("\nPut it there and the next start takes it.");
        return s.toString();
    }

    private void reasons(Look l, StringBuilder s) {
        int shown = 0;
        for (String r : l.refused) {
            if (shown == 5) break;
            s.append('\n').append(r);
            ++shown;
        }
        if (l.refused.size() > shown) s.append("\nand ").append(l.refused.size() - shown).append(" more .gba files.");
        for (int k = 0; k < l.zipped.size() && k < 2; ++k) s.append('\n').append(l.zipped.get(k)).append(" is zipped: unzip it first.");
    }

    private void toast(String s) {
        Toast.makeText(getApplicationContext(), s, Toast.LENGTH_LONG).show();
    }

    private void play() {
        dataDir(this).mkdirs();
        startActivity(new Intent(this, GameActivity.class));
        finish();
    }
}
