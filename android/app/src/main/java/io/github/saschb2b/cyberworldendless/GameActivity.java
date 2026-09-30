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

    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL2", "main" };
    }

    @Override
    protected String[] getArguments() {
        return new String[] { "--rom-dir", RomActivity.romDir(this).getPath(), "--data-dir", RomActivity.dataDir(this).getPath() };
    }
}
