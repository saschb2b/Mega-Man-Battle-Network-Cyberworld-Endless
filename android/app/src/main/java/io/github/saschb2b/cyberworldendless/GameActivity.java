package io.github.saschb2b.cyberworldendless;

import android.content.res.Configuration;
import android.os.Bundle;
import android.view.HapticFeedbackConstants;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;

/** The game: SDL's activity running libmain.so on the ROM RomActivity keeps. */
public class GameActivity extends SDLActivity {
    /* the layer's map on a display beside this one, where there is one */
    private SecondScreen second;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        if (!mBrokenLibraries) second = SecondScreen.open(this);
    }

    /** A short tick under the thumb as a touch control is pressed (touch.c
     *  calls it): the view's own key feedback, as Android's keyboard gives,
     *  under the system's touch feedback setting. */
    public void haptic() {
        runOnUiThread(() -> getWindow().getDecorView().performHapticFeedback(HapticFeedbackConstants.VIRTUAL_KEY));
    }

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

    @Override
    protected String[] getArguments() {
        return new String[] { "--rom-dir", RomActivity.romDir(this).getPath(), "--data-dir", RomActivity.dataDir(this).getPath() };
    }
}
