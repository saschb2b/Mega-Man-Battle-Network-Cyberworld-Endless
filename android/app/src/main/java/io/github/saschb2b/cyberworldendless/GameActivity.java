package io.github.saschb2b.cyberworldendless;

import android.os.Bundle;
import android.view.HapticFeedbackConstants;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;

/** The game: SDL's activity running libmain.so on the ROM RomActivity keeps. */
public class GameActivity extends SDLActivity {
    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    /** A short tick under the thumb as a touch control is pressed (touch.c
     *  calls it): the view's own key feedback, as Android's keyboard gives,
     *  under the system's touch feedback setting. */
    public void haptic() {
        runOnUiThread(() -> getWindow().getDecorView().performHapticFeedback(HapticFeedbackConstants.VIRTUAL_KEY));
    }

    /* A session ends with its process (the manifest's ":game", apart from
     * RomActivity's): SDL runs the game's C main again in a process Android
     * kept, with the last session's statics, its quit among them, and a game
     * opened again after a quit (or Play after the icon's ROMs shortcut)
     * closed at once. */
    @Override
    protected void onDestroy() {
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
