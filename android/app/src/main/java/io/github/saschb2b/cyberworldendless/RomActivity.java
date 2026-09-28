package io.github.saschb2b.cyberworldendless;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.MessageDigest;

/**
 * The start: the game when the player's ROM is kept, else a page that asks
 * for it. The ROM is chosen with Android's own file picker (no permission to
 * read storage), checked (size and SHA-1: Mega Man Battle Network 6: Cybeast
 * Gregar (USA), unmodified) and copied into the app's own files. It never
 * leaves the device.
 */
public class RomActivity extends Activity {
    static final long ROM_SIZE = 8L * 1024 * 1024;
    static final String ROM_SHA1 = "89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6";
    static final int PICK = 1;

    static File romDir(Context c) { return new File(c.getFilesDir(), "rom"); }
    static File dataDir(Context c) { return new File(c.getFilesDir(), "data"); }
    static File rom(Context c) { return new File(romDir(c), "bn6g.gba"); }

    private TextView status;
    private Button choose;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        if (rom(this).length() == ROM_SIZE) {
            play();
            return;
        }
        LinearLayout page = new LinearLayout(this);
        page.setOrientation(LinearLayout.VERTICAL);
        page.setGravity(Gravity.CENTER);
        page.setBackgroundColor(Color.rgb(7, 24, 58));
        int pad = dp(28);
        page.setPadding(pad, pad, pad, pad);
        page.addView(text("Cyberworld Endless", 26, Color.rgb(255, 214, 16), true));
        page.addView(text("A roguelike on Mega Man Battle Network 6. It runs from your own copy of "
            + "Mega Man Battle Network 6: Cybeast Gregar (USA), an unmodified .gba file. "
            + "It stays on this device and is never uploaded.", 16, Color.rgb(214, 244, 255), false));
        choose = new Button(this);
        choose.setText("Choose ROM file");
        choose.setAllCaps(false);
        choose.setTextColor(Color.rgb(16, 54, 74));
        choose.setTypeface(Typeface.DEFAULT_BOLD);
        choose.setTextSize(TypedValue.COMPLEX_UNIT_SP, 17);
        choose.setPadding(dp(24), dp(12), dp(24), dp(12));
        GradientDrawable plate = new GradientDrawable();
        plate.setColor(Color.rgb(255, 214, 16));
        plate.setCornerRadius(dp(6));
        plate.setStroke(dp(2), Color.rgb(255, 247, 165));
        choose.setBackground(plate);
        choose.setOnClickListener(v -> pick());
        LinearLayout.LayoutParams at = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        at.topMargin = dp(24);
        page.addView(choose, at);
        status = text("", 15, Color.rgb(255, 214, 16), false);
        page.addView(status);
        setContentView(page);
        choose.requestFocus();
    }

    private int dp(int v) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, v, getResources().getDisplayMetrics());
    }

    private TextView text(String s, int sp, int color, boolean bold) {
        TextView t = new TextView(this);
        t.setText(s);
        t.setTextSize(TypedValue.COMPLEX_UNIT_SP, sp);
        t.setTextColor(color);
        t.setGravity(Gravity.CENTER);
        t.setPadding(0, dp(8), 0, dp(8));
        if (bold) t.setTypeface(Typeface.DEFAULT_BOLD);
        return t;
    }

    /* (a handheld's A presses the button too) */
    @Override
    public boolean onKeyDown(int code, KeyEvent e) {
        if (code == KeyEvent.KEYCODE_BUTTON_A && choose != null) {
            pick();
            return true;
        }
        return super.onKeyDown(code, e);
    }

    private void pick() {
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        i.addCategory(Intent.CATEGORY_OPENABLE);
        i.setType("*/*");
        startActivityForResult(i, PICK);
    }

    @Override
    protected void onActivityResult(int request, int result, Intent data) {
        super.onActivityResult(request, result, data);
        if (request == PICK && result == RESULT_OK && data != null && data.getData() != null) take(data.getData());
    }

    private void take(final Uri uri) {
        status.setText("Checking…");
        choose.setEnabled(false);
        new Thread(() -> {
            String problem = copy(uri);
            runOnUiThread(() -> {
                if (problem == null) {
                    play();
                    return;
                }
                status.setText(problem);
                choose.setEnabled(true);
            });
        }).start();
    }

    /** Copies the ROM in and checks it; null when it is the right one. */
    private String copy(Uri uri) {
        File dir = romDir(this), part = new File(dir, "bn6g.part");
        if (!dir.isDirectory() && !dir.mkdirs()) return "The app could not make its ROM folder.";
        try (InputStream in = getContentResolver().openInputStream(uri); OutputStream out = new FileOutputStream(part)) {
            if (in == null) return "That file could not be read.";
            MessageDigest sha1 = MessageDigest.getInstance("SHA-1");
            byte[] buf = new byte[1 << 16];
            long n = 0;
            for (int k; (k = in.read(buf)) > 0; ) {
                out.write(buf, 0, k);
                sha1.update(buf, 0, k);
                n += k;
                if (n > ROM_SIZE) break;
            }
            if (n != ROM_SIZE) {
                part.delete();
                return "That is not an 8 MB GBA ROM. You need Mega Man Battle Network 6: Cybeast Gregar (USA).";
            }
            StringBuilder hex = new StringBuilder();
            for (byte b : sha1.digest()) hex.append(String.format("%02x", b));
            if (!ROM_SHA1.equals(hex.toString())) {
                part.delete();
                return "That is a different version or a changed ROM. Only the unmodified Cybeast Gregar (USA) works.";
            }
        } catch (Exception e) {
            part.delete();
            return "That file could not be read: " + e.getMessage();
        }
        if (!part.renameTo(rom(this))) return "The app could not keep the ROM.";
        return null;
    }

    private void play() {
        dataDir(this).mkdirs();
        startActivity(new Intent(this, GameActivity.class));
        finish();
    }
}
