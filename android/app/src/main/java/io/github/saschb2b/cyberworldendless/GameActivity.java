package io.github.saschb2b.cyberworldendless;

import android.content.ActivityNotFoundException;
import android.content.ClipData;
import android.content.Intent;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.view.HapticFeedbackConstants;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.WindowManager;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.libsdl.app.SDLActivity;

/**
 * The game: SDL's activity running libmain.so, its ROMs in the app's own
 * ROM folder (RomLook). The launcher before the game (src/launcher/) is the
 * game's own screen; it calls in here (src/launcher/pick_android.c) for
 * Android's folder and file pickers, the folder kept looked in again, and
 * the saves' copy written into that folder.
 */
public class GameActivity extends SDLActivity {
    /** Starts the launcher open (RomActivity's: the icon's ROMs shortcut). */
    static final String EXTRA_LAUNCHER = "launcher";
    static final int PICK_FOLDER = 1, PICK_FILES = 2;

    /* the layer's map on a display beside this one, where there is one */
    private SecondScreen second;
    private RomLook roms;
    /* a picker's end, for romsResult: "status\nsaves\nwords" */
    private volatile String picked;
    private volatile boolean picking;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        if (!mBrokenLibraries) second = SecondScreen.open(this);
    }

    private RomLook roms() {
        if (roms == null) roms = new RomLook(getApplicationContext());
        return roms;
    }

    /** A left Joy-Con's arrow buttons, sent on as the D-pad's (issue #37).
     *  Linux's hid-nintendo reports them as BTN_DPAD_UP..RIGHT (scan codes
     *  0x220-0x223), which Android's generic key layout (Generic.kl) gives
     *  no key code, so they came as KEYCODE_UNKNOWN and SDL 2's Android
     *  driver dropped them (SDL 3 reads their scan codes instead,
     *  libsdl-org/SDL#15508); the mapping pads.c gives the Joy-Con names
     *  SDL's D-pad buttons. */
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int dpad = event.getKeyCode() == KeyEvent.KEYCODE_UNKNOWN ? dpadOf(event) : 0;
        if (dpad != 0) {
            event = new KeyEvent(event.getDownTime(), event.getEventTime(), event.getAction(), dpad,
                event.getRepeatCount(), event.getMetaState(), event.getDeviceId(), event.getScanCode(),
                event.getFlags(), event.getSource());
        }
        return super.dispatchKeyEvent(event);
    }

    private static int dpadOf(KeyEvent event) {
        if ((event.getSource() & InputDevice.SOURCE_GAMEPAD) != InputDevice.SOURCE_GAMEPAD
            && (event.getSource() & InputDevice.SOURCE_JOYSTICK) != InputDevice.SOURCE_JOYSTICK) return 0;
        switch (event.getScanCode()) {
        case 0x220: return KeyEvent.KEYCODE_DPAD_UP;
        case 0x221: return KeyEvent.KEYCODE_DPAD_DOWN;
        case 0x222: return KeyEvent.KEYCODE_DPAD_LEFT;
        case 0x223: return KeyEvent.KEYCODE_DPAD_RIGHT;
        default: return 0;
        }
    }

    /** A short tick under the thumb as a touch control is pressed (touch.c
     *  calls it): the view's own key feedback, as Android's keyboard gives,
     *  under the system's touch feedback setting. */
    public void haptic() {
        runOnUiThread(() -> getWindow().getDecorView().performHapticFeedback(HapticFeedbackConstants.VIRTUAL_KEY));
    }

    // ---- the launcher's calls (src/launcher/pick_android.c, the game's thread) ----

    /** Opens Android's picker: the folder the ROMs are in (0), or the files
     *  themselves, several at once (1). False where one is open already. */
    public boolean romsPick(int kind) {
        if (picking) return false;
        picking = true;
        picked = null;
        runOnUiThread(() -> {
            Intent i;
            if (kind == 1) {
                i = new Intent(Intent.ACTION_OPEN_DOCUMENT);
                i.addCategory(Intent.CATEGORY_OPENABLE);
                i.setType("*/*");
                i.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
            } else {
                i = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
                Uri folder = roms().folder();
                if (Build.VERSION.SDK_INT >= 26 && folder != null) i.putExtra(DocumentsContract.EXTRA_INITIAL_URI, folder);
                i.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
            }
            try {
                startActivityForResult(i, kind == 1 ? PICK_FILES : PICK_FOLDER);
            } catch (ActivityNotFoundException e) {
                done(0, false, kind == 1 ? "This device has no file picker." : "This device has no folder picker: choose the files instead.");
            }
        });
        return true;
    }

    /** A picker's end, once: "status\nsaves\nwhat its look found" (status 1
     *  looked, 0 nothing chosen; saves 1 where the folder held their copy),
     *  null while it is open. */
    public String romsResult() {
        String r = picked;
        if (r != null) picked = null;
        return r;
    }

    /** The folder kept looked in again (a quick look) for the ROMs the app
     *  lacks: "bits copied in (1 BN6, 2 BN5)\nwhat it found", "-1\nwhy" where
     *  it cannot be opened, null for no folder kept. */
    public String romsLook() {
        RomLook r = roms();
        Uri folder = r.folder();
        if (folder == null) return null;
        if (RomLook.kept(this, RomLook.BN6) && RomLook.kept(this, RomLook.BN5)) return "0\n";
        RomLook.Look l = r.lookFolder(folder, r.folderName(), false);
        if (l.lost != null) return "-1\n" + r.words(l);
        return l.bits() + "\n" + (l.bits() != 0 || !l.refused.isEmpty() ? r.words(l) : "");
    }

    /** The folder kept: its name, null for none. */
    public String romsFolder() {
        return roms().folder() != null ? roms().folderName() : null;
    }

    /** The saves' copy at `from` written into the folder kept (mirror.c). */
    public boolean savesPut(String from) {
        return roms().putSaves(new File(from));
    }

    private void done(int status, boolean saves, String words) {
        picking = false;
        picked = status + "\n" + (saves ? 1 : 0) + "\n" + (words == null ? "" : words);
    }

    @Override
    protected void onActivityResult(int request, int result, Intent data) {
        super.onActivityResult(request, result, data);
        if (request != PICK_FOLDER && request != PICK_FILES) return;
        if (result != RESULT_OK || data == null) {
            done(0, false, request == PICK_FOLDER && Build.VERSION.SDK_INT >= 30
                ? "No folder chosen. If your ROMs are in Download itself, choose the files instead." : "");
            return;
        }
        RomLook r = roms();
        if (request == PICK_FOLDER && data.getData() != null) {
            Uri tree = data.getData();
            int flags = data.getFlags();
            /* (off the screen's thread: a big folder takes its time) */
            new Thread(() -> {
                String name = r.nameOf(tree);
                r.keepFolder(tree, name, flags);
                RomLook.Look l = r.lookFolder(tree, name, true);
                done(1, l.saves, r.words(l));
            }).start();
            return;
        }
        List<Uri> files = new ArrayList<>();
        ClipData clip = data.getClipData();
        if (clip != null) for (int k = 0; k < clip.getItemCount(); ++k) files.add(clip.getItemAt(k).getUri());
        else if (data.getData() != null) files.add(data.getData());
        new Thread(() -> {
            RomLook.Look l = r.lookFiles(files);
            done(1, false, r.words(l));
        }).start();
    }

    // ---- the second screen ----

    /* The second screen comes and goes with the game's own thread, as SDL
     * runs and pauses it: at onStart and onStop from Android 7 on, at
     * onResume and onPause before. (onPause alone also comes with a system
     * dialog over the game, where taking the map down made the lower screen
     * flash: tmc-android, on the Thor.) */
    @Override
    protected void resumeNativeThread() {
        super.resumeNativeThread();
        if (second != null) second.start();
    }

    @Override
    protected void pauseNativeThread() {
        if (second != null) second.stop();
        super.pauseNativeThread();
    }

    /* (the game's window moved to another display, or turned) */
    @Override
    public void onConfigurationChanged(Configuration config) {
        super.onConfigurationChanged(config);
        if (second != null) second.pick();
    }

    /** The second screen's next picture, w x h in its pixels: second_android.c
     *  calls it on the game's thread. */
    public void secondFrame(int w, int h) {
        if (second != null) second.frame(w, h);
    }

    /** ... or the second screen black. */
    public void secondDark() {
        if (second != null) second.dark();
    }

    /* A session ends with its process (the manifest's ":game", apart from
     * RomActivity's): SDL runs the game's C main again in a process Android
     * kept, with the last session's statics, its quit among them, and a game
     * opened again after a quit (or Play after the icon's ROMs shortcut)
     * closed at once. */
    @Override
    protected void onDestroy() {
        if (second != null) second.close();
        super.onDestroy();
        if (isFinishing()) System.exit(0);
    }

    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL2", "main" };
    }

    /* (the launcher where it is wanted, or open: the ROMs shortcut) */
    @Override
    protected String[] getArguments() {
        boolean open = getIntent() != null && getIntent().getBooleanExtra(EXTRA_LAUNCHER, false);
        return new String[] { "--rom-dir", RomLook.romDir(this).getPath(), "--data-dir", RomLook.dataDir(this).getPath(),
            "--launcher", open ? "open" : "auto" };
    }
}
