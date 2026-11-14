package io.github.saschb2b.cyberworldendless;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.util.Log;

import java.io.File;

/**
 * The new version in place (Android's MY_PACKAGE_REPLACED, sent to the app
 * itself after an update): where the game updated itself (Updater's mark,
 * a few minutes old at most), the game opened again, as the other systems'
 * updates start the new version. Up to Android 9 alone: from Android 10 on
 * an app may not open an activity from the background (Android 14 in the
 * emulator refused it, BAL_BLOCK), and the player opens the game from its
 * icon.
 */
public class UpdateReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        if (!Intent.ACTION_MY_PACKAGE_REPLACED.equals(intent.getAction())) return;
        File mark = new File(context.getFilesDir(), Updater.MARK);
        long age = System.currentTimeMillis() - mark.lastModified();
        if (!mark.delete() || age > 10 * 60 * 1000 || Build.VERSION.SDK_INT >= 29) return;
        Log.i("Cyberworld", "update: in place, the game opened again");
        try {
            context.startActivity(new Intent(context, RomActivity.class).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK));
        } catch (Exception e) {
            Log.i("Cyberworld", "update: the game not opened (" + e + ")");
        }
    }
}
