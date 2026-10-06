package io.github.saschb2b.cyberworldendless;

import android.app.Activity;
import android.app.Presentation;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.hardware.display.DeviceProductInfo;
import android.hardware.display.DisplayManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * The second screen: the layer's map on a display beside the game's (the
 * AYN Thor's lower screen), as the 3DS's bottom screen shows it. The game
 * draws it every fifth frame (second_android.c) into pixels this copies
 * into a Bitmap; a Presentation on that display shows it at a whole scale,
 * black round it. The Presentation never takes focus: the game's keys,
 * Back and touch controls stay on the game's screen. A touch on the map
 * does nothing, as on the 3DS, whose map has no use for one.
 */
final class SecondScreen {
    private static final String TAG = RomLook.TAG;

    private final Activity activity;
    private final DisplayManager displays;
    private final Handler main = new Handler(Looper.getMainLooper());
    /* the game's pixels, the first w * h * 4 bytes each picture's */
    private final ByteBuffer pixels = nativePixels();
    private Panel panel;        // shown, null none
    private Picture view;       // its picture's view
    private boolean started, slept;
    private Bitmap picture;     // the game's last, kept across panels
    private boolean dark = true;
    private volatile int nextW, nextH;   // the picture the game handed, taken on the main thread

    private SecondScreen(Activity activity) {
        this.activity = activity;
        displays = (DisplayManager) activity.getSystemService(Context.DISPLAY_SERVICE);
        if (displays == null) throw new IllegalStateException("no DisplayManager");
        activity.registerReceiver(screenOff, new IntentFilter(Intent.ACTION_SCREEN_OFF));
    }

    /** The second screen, or null where it cannot be: it is never worth the game. */
    static SecondScreen open(Activity activity) {
        try {
            return new SecondScreen(activity);
        } catch (Throwable e) {
            Log.w(TAG, "second screen: none, " + e);
            return null;
        }
    }

    /** On, with the game's thread running (GameActivity.resumeNativeThread). */
    void start() {
        if (started) return;
        started = true;
        try {
            displays.registerDisplayListener(listener, main);
            pick();
            if (slept) {
                slept = false;
                main.postDelayed(renew, 700);
            }
        } catch (RuntimeException e) {
            Log.w(TAG, "second screen: " + e);
        }
    }

    /** Off, the game's thread paused (pauseNativeThread), and at the end. */
    void stop() {
        if (!started) return;
        started = false;
        main.removeCallbacks(renew);
        try {
            displays.unregisterDisplayListener(listener);
        } catch (RuntimeException ignored) {
        }
        hide();
    }

    void close() {
        stop();
        try {
            activity.unregisterReceiver(screenOff);
        } catch (RuntimeException ignored) {
        }
    }

    /** The display chosen again: displays come, go and change, and the game's window may move. */
    void pick() {
        if (!started || activity.isFinishing()) return;
        try {
            List<Display> all = candidates();
            if (panel != null && panel.isShowing()) {
                int on = panel.getDisplay().getDisplayId();
                for (Display d : all) if (d.getDisplayId() == on) return;
            }
            hide();
            for (Display d : all) if (show(d)) return;
        } catch (RuntimeException e) {
            Log.w(TAG, "second screen: " + e);
        }
    }

    /** A picture of w x h in the pixels, from the game's thread (GameActivity.secondFrame). */
    void frame(int w, int h) {
        nextW = w;
        nextH = h;
        main.post(take);
    }

    /** The screen black, from the game's thread (GameActivity.secondDark). */
    void dark() {
        main.post(blank);
    }

    /* The displays the map can go on, best first: the system's presentation
     * displays, in its order (wireless, cabled, overlay, virtual, then
     * built-in ones), but not the game's own or the phone's, not one off or
     * dozing, and from Android 12 not one that tells its place on an HDMI
     * chain (a TV or monitor on a cable), which goes on mirroring the game.
     * One named as the phone's own screen comes last: some devices keep a
     * virtual display under that name (Azahar's pull request #1667), and
     * some name their real second screen so. */
    private List<Display> candidates() {
        int game = gameDisplay();
        Display phone = displays.getDisplay(Display.DEFAULT_DISPLAY);
        String phoneName = phone != null ? phone.getName() : null;
        List<Display> first = new ArrayList<>(), last = new ArrayList<>();
        for (Display d : displays.getDisplays(DisplayManager.DISPLAY_CATEGORY_PRESENTATION)) {
            int id = d.getDisplayId();
            if (id == game || id == Display.DEFAULT_DISPLAY || !d.isValid() || !lit(d) || cabled(d)) continue;
            (phoneName != null && phoneName.equalsIgnoreCase(d.getName()) ? last : first).add(d);
        }
        first.addAll(last);
        return first;
    }

    @SuppressWarnings("deprecation")
    private int gameDisplay() {
        Display d = Build.VERSION.SDK_INT >= 30 ? activity.getDisplay() : activity.getWindowManager().getDefaultDisplay();
        return d != null ? d.getDisplayId() : Display.DEFAULT_DISPLAY;
    }

    private static boolean lit(Display d) {
        int s = d.getState();
        return s != Display.STATE_OFF && s != Display.STATE_DOZE && s != Display.STATE_DOZE_SUSPEND;
    }

    /* (Android reads the place from the EDID's HDMI block, an HDMI address
     * one step or more down a chain; a built-in panel, a DisplayPort monitor
     * or a virtual display has none) */
    private static boolean cabled(Display d) {
        if (Build.VERSION.SDK_INT < 31) return false;
        DeviceProductInfo info = d.getDeviceProductInfo();
        if (info == null) return false;
        int c = info.getConnectionToSinkType();
        return c == DeviceProductInfo.CONNECTION_TO_SINK_DIRECT || c == DeviceProductInfo.CONNECTION_TO_SINK_TRANSITIVE;
    }

    private boolean show(Display d) {
        Panel p = new Panel(d);
        try {
            p.show();
        } catch (RuntimeException e) {
            // (the display gone meanwhile, or one that refuses windows:
            // WindowManager's BadTokenException or InvalidDisplayException)
            Log.w(TAG, "second screen: display " + d.getDisplayId() + " refused the map, " + e);
            return false;
        }
        panel = p;
        Log.i(TAG, "second screen: the map on display " + d.getDisplayId() + " (" + d.getName() + ")");
        return true;
    }

    private void hide() {
        Panel p = panel;
        panel = null;   // (first: the dismissal's listener then knows it for ours)
        view = null;
        nativeDisplay(0, 0);
        if (p == null) return;
        Log.i(TAG, "second screen: off");
        try {
            p.dismiss();
        } catch (RuntimeException ignored) {
        }
    }

    private final DisplayManager.DisplayListener listener = new DisplayManager.DisplayListener() {
        @Override public void onDisplayAdded(int id) { pick(); }
        @Override public void onDisplayRemoved(int id) { pick(); }
        @Override public void onDisplayChanged(int id) { pick(); }
    };

    /* (a panel the system dismissed, as a display's metrics changed: shown
     * again, if its display is still there) */
    private final DialogInterface.OnDismissListener dismissed = dialog -> {
        if (dialog != panel) return;
        Log.i(TAG, "second screen: dismissed by the system");
        panel = null;
        view = null;
        nativeDisplay(0, 0);
        main.post(this::pick);
    };

    /* A panel shown as the screen woke can stay black for good, its display
     * ready before its surface (chrono-duo found it on a Thor Lite): after a
     * screen turned off, it is made again once the screen has settled. */
    private final BroadcastReceiver screenOff = new BroadcastReceiver() {
        @Override public void onReceive(Context c, Intent i) { slept = true; }
    };
    private final Runnable renew = () -> {
        hide();
        pick();
    };

    private final Runnable take = new Runnable() {
        @Override public void run() {
            try {
                int w = nextW, h = nextH;
                if (picture == null || picture.getWidth() != w || picture.getHeight() != h)
                    picture = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
                pixels.rewind();
                picture.copyPixelsFromBuffer(pixels);
                dark = false;
                if (view != null) view.invalidate();
            } catch (RuntimeException e) {
                Log.w(TAG, "second screen: " + e);
            } finally {
                nativeCopied();
            }
        }
    };

    private final Runnable blank = () -> {
        dark = true;
        if (view != null) view.invalidate();
    };

    /* The map's window on its display: never focused (a focused one took
     * the gamepad's keys from the game, and Back, a gamepad's B too,
     * cancelled it: tmc-android, on the Thor), never cancelled, with the
     * system's bars hidden on it (the Thor's lower screen keeps a band for
     * its navigation bar otherwise). */
    private final class Panel extends Presentation {
        Panel(Display d) {
            super(activity, d);
            setCancelable(false);
            setOnDismissListener(dismissed);
        }

        @Override
        protected void onCreate(Bundle state) {
            super.onCreate(state);
            Window w = getWindow();
            if (w != null) {
                w.addFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                        | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                hideBars(w.getDecorView());
            }
            Picture v = new Picture(getContext());
            view = v;
            setContentView(v);
        }
    }

    @SuppressWarnings("deprecation")
    private static void hideBars(View decor) {
        decor.setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
    }

    /* The picture at the largest whole scale the view takes, its pixels
     * sharp, in the middle; shrunk smoothly into a view smaller than it.
     * Its size is the game's to draw at (nativeDisplay). */
    private final class Picture extends View {
        private final Paint sharp = new Paint(), soft = new Paint(Paint.FILTER_BITMAP_FLAG);
        private final Rect to = new Rect();

        Picture(Context c) {
            super(c);
            sharp.setFilterBitmap(false);
            sharp.setAntiAlias(false);
            setBackgroundColor(Color.BLACK);
        }

        @Override
        protected void onSizeChanged(int w, int h, int oldW, int oldH) {
            if (view == this) nativeDisplay(w, h);
        }

        @Override
        protected void onDraw(Canvas c) {
            Bitmap b = picture;
            if (b == null || dark) return;
            int vw = getWidth(), vh = getHeight(), bw = b.getWidth(), bh = b.getHeight();
            int k = Math.min(vw / bw, vh / bh);
            float s = k >= 1 ? k : Math.min((float) vw / bw, (float) vh / bh);
            int w = Math.round(bw * s), h = Math.round(bh * s), x = (vw - w) / 2, y = (vh - h) / 2;
            to.set(x, y, x + w, y + h);
            c.drawBitmap(b, null, to, k >= 1 ? sharp : soft);
        }
    }

    private static native ByteBuffer nativePixels();
    private static native void nativeDisplay(int w, int h);
    private static native void nativeCopied();
}
