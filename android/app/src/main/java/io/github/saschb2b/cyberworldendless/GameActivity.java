package io.github.saschb2b.cyberworldendless;

import android.os.Bundle;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;

/** The game: SDL's activity running libmain.so on the ROM RomActivity keeps. */
public class GameActivity extends SDLActivity {
    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
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
