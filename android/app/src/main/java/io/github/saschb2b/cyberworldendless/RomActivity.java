package io.github.saschb2b.cyberworldendless;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

/**
 * The app's start: the game at once (GameActivity, in a process of its
 * own), whose launcher (src/launcher/) asks for the ROMs where it has none
 * and shows the ROMs again from the title's R. The app icon's ROMs shortcut
 * (res/xml/shortcuts.xml) starts the game with its launcher open.
 */
public class RomActivity extends Activity {
    /** The app icon's shortcut: the launcher open, the ROMs kept or not. */
    static final String ACTION_ROMS = "io.github.saschb2b.cyberworldendless.ROMS";

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        RomLook.dataDir(this).mkdirs();
        Intent game = new Intent(this, GameActivity.class);
        if (ACTION_ROMS.equals(getIntent().getAction())) game.putExtra(GameActivity.EXTRA_LAUNCHER, true);
        startActivity(game);
        finish();
    }
}
