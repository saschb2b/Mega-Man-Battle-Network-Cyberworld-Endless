package io.github.saschb2b.cyberworldendless;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.UriPermission;
import android.database.Cursor;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.provider.DocumentsContract.Document;
import android.provider.OpenableColumns;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
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
 * The ROMs from the folder the player chose in the launcher (src/launcher/),
 * or the files picked there. Each .gba is told by its header's game code;
 * Mega Man Battle Network 6: Cybeast Gregar (USA)'s and 5: Team Colonel
 * (USA)'s are checked by their SHA-1 and those two alone copied into the
 * app's own ROM folder, where the game reads them; every other .gba is named
 * with why it was refused. The folder is kept (its access persisted, read
 * and write) and looked in again at each start, so BN5 put there later comes
 * in by itself; the saves' copy the game keeps in it (cyberworld-endless.cwsave,
 * src/launcher/mirror.h) is read and written here. Nothing leaves the device.
 */
final class RomLook {
    static final String TAG = "Cyberworld";
    static final long ROM_SIZE = 8L * 1024 * 1024;
    /** The saves' copy in the folder: backup.h's BACKUP_NAME. */
    static final String SAVES = "cyberworld-endless.cwsave";
    static final long SAVES_MAX = 16L << 20;

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
     *  code: a player who has one is told which it is (src/launcher/launcher_text.c
     *  names them the same way). */
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

    static File romDir(Context c) { return new File(c.getFilesDir(), "rom"); }
    static File dataDir(Context c) { return new File(c.getFilesDir(), "data"); }
    static File rom(Context c, int i) { return new File(romDir(c), ROMS[i].file); }
    static boolean kept(Context c, int i) { return rom(c, i).length() == ROM_SIZE; }

    private final Context ctx;
    private final SharedPreferences prefs;
    private final SharedPreferences savesPrefs;

    RomLook(Context ctx) {
        this.ctx = ctx;
        prefs = ctx.getSharedPreferences("roms", Context.MODE_PRIVATE);
        savesPrefs = ctx.getSharedPreferences("saves", Context.MODE_PRIVATE);
    }

    // ---- the folder kept ----

    Uri folder() {
        String s = prefs.getString("folder", null);
        return s == null ? null : Uri.parse(s);
    }

    String folderName() { return prefs.getString("folderName", "your ROM folder"); }

    Uri savesFolder() {
        String s = savesPrefs.getString("folder", null);
        return s == null ? folder() : Uri.parse(s);
    }

    String savesFolderName() {
        return savesPrefs.contains("folder") ? savesPrefs.getString("folderName", "your transfer folder") : folderName();
    }

    boolean keepSavesFolder(Uri tree, String name, int flags) {
        int keep = flags & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        try {
            ctx.getContentResolver().takePersistableUriPermission(tree, keep != 0 ? keep : Intent.FLAG_GRANT_READ_URI_PERMISSION);
        } catch (SecurityException e) { return false; }
        String previous = savesPrefs.getString("folder", null);
        Uri old = previous == null ? null : Uri.parse(previous);
        if (old != null && !old.equals(tree) && !old.equals(folder())) release(old);
        savesPrefs.edit().putString("folder", tree.toString()).putString("folderName", name).apply();
        return true;
    }

    private void release(Uri tree) {
        try {
            ctx.getContentResolver().releasePersistableUriPermission(tree,
                Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        } catch (SecurityException e) { /* (gone already, or held read-only) */ }
    }

    /** The grant kept for the looks at later starts and the saves' copy:
     *  read, and write where the picker gave it (one held before for another
     *  folder is given back). */
    void keepFolder(Uri tree, String name, int flags) {
        Uri old = folder();
        int keep = flags & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        try {
            ctx.getContentResolver().takePersistableUriPermission(tree, keep != 0 ? keep : Intent.FLAG_GRANT_READ_URI_PERMISSION);
        } catch (SecurityException e) {
            /* (a provider whose grants can't be kept: this look only) */
            Log.w(TAG, "the folder's access could not be kept: " + e.getMessage());
            return;
        }
        String saveTree = savesPrefs.getString("folder", null);
        if (old != null && !old.equals(tree) && !old.toString().equals(saveTree)) release(old);
        prefs.edit().putString("folder", tree.toString()).putString("folderName", name).apply();
    }

    String nameOf(Uri tree) {
        try (Cursor c = ctx.getContentResolver().query(DocumentsContract.buildDocumentUriUsingTree(tree, DocumentsContract.getTreeDocumentId(tree)),
                new String[] { Document.COLUMN_DISPLAY_NAME }, null, null, null)) {
            if (c != null && c.moveToFirst() && !c.isNull(0)) return c.getString(0);
        } catch (Exception e) { /* (named generically) */ }
        return "your ROM folder";
    }

    private boolean granted(Uri tree, boolean write) {
        for (UriPermission p : ctx.getContentResolver().getPersistedUriPermissions())
            if (p.getUri().equals(tree) && p.isReadPermission() && (!write || p.isWritePermission())) return true;
        return false;
    }

    // ---- looking ----

    /** What one look found: the ROMs it copied in, and each file it refused, with why. */
    static final class Look {
        String where;                                   // the folder's name; null for files picked
        String lost;                                    // the folder could not be opened: why
        int files, gba, battleNetwork;                  // files picked; .gba looked at; Battle Network ROMs refused
        boolean saves;                                  // the folder's saves' copy copied to the data folder's found.cwsave
        final boolean[] kept = new boolean[ROMS.length];   // copied in now
        final boolean[] had = new boolean[ROMS.length];    // offered, a copy kept already
        final List<String> refused = new ArrayList<>();    // "name: why", Battle Network ROMs first
        final List<String> zipped = new ArrayList<>();

        void refuse(String name, String why, boolean bn) {
            if (bn) refused.add(battleNetwork++, name + ": " + why);
            else refused.add(name + ": " + why);
        }
        int bits() { return (kept[BN6] ? 1 : 0) | (kept[BN5] ? 2 : 0); }
    }

    /**
     * A look at the folder (tree, called where): its .gba files and those of
     * the folders directly in it. A look no player asked for (a start's) is
     * quick: only an 8 MB .gba not settled before (its document, size and
     * date) is opened. A pick's also fetches the saves' copy in the folder.
     */
    Look lookFolder(Uri tree, String where, boolean pick) {
        Look l = new Look();
        l.where = where;
        Set<String> seen = new HashSet<>(prefs.getStringSet("seen", new HashSet<>()));
        int before = seen.size();
        try {
            if (!pick && !granted(tree, false)) throw new SecurityException("its access is no longer held");
            walk(tree, DocumentsContract.getTreeDocumentId(tree), 1, !pick, seen, l);
            if (pick) l.saves = fetchSaves(tree);
        } catch (Exception e) {
            l.lost = e.getClass().getSimpleName() + ": " + e.getMessage();
        }
        if (seen.size() != before) {
            if (seen.size() > 400) seen.clear();
            prefs.edit().putStringSet("seen", seen).apply();
        }
        Log.i(TAG, "looked in " + where + ": " + l.gba + " .gba" + (l.bits() != 0 ? ", kept " + l.bits() : "")
            + (l.refused.isEmpty() ? "" : ", refused " + l.refused) + (l.saves ? ", saves found" : "")
            + (l.lost != null ? ", could not open it (" + l.lost + ")" : ""));
        return l;
    }

    /** The files picked: whatever their names, each is looked at (the player chose it). */
    Look lookFiles(List<Uri> files) {
        Look l = new Look();
        l.files = files.size();
        for (Uri u : files) {
            String name = "a file";
            long size = -1;
            try (Cursor c = ctx.getContentResolver().query(u, new String[] { OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE }, null, null, null)) {
                if (c != null && c.moveToFirst()) {
                    if (!c.isNull(0)) name = c.getString(0);
                    if (!c.isNull(1)) size = c.getLong(1);
                }
            } catch (Exception e) { /* (named generically) */ }
            if (zipped(name, l)) continue;
            ++l.gba;
            offer(u, name, size, l);
        }
        Log.i(TAG, "looked at " + l.files + " files picked: kept " + l.bits() + (l.refused.isEmpty() ? "" : ", refused " + l.refused));
        return l;
    }

    private void walk(Uri tree, String doc, int depth, boolean quick, Set<String> seen, Look l) throws FileNotFoundException {
        List<String> dirs = new ArrayList<>();
        try (Cursor c = ctx.getContentResolver().query(DocumentsContract.buildChildDocumentsUriUsingTree(tree, doc), COLUMNS, null, null, null)) {
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
            if (kept(ctx, i)) { l.had[i] = true; return true; }
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
        try (InputStream in = ctx.getContentResolver().openInputStream(uri)) {
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
        File dir = romDir(ctx), part = new File(dir, ROMS[i].file + ".part");
        if (!dir.isDirectory() && !dir.mkdirs()) return "the app could not make its ROM folder";
        try (InputStream in = ctx.getContentResolver().openInputStream(uri); OutputStream out = new FileOutputStream(part)) {
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
        if (!part.renameTo(rom(ctx, i))) {
            part.delete();
            return "the app could not keep it";
        }
        return null;
    }

    // ---- what a look says ----

    /** The launcher's note on a look: what came in, else why nothing did. */
    String words(Look l) {
        if (l.lost != null)
            return "The folder " + (l.where != null ? l.where : folderName()) + " can't be opened any more: it was moved or removed, or its access was taken back. Choose it again.";
        StringBuilder s = new StringBuilder();
        if (l.kept[BN6] || l.kept[BN5]) {
            s.append("Took ").append(l.kept[BN6] ? ROMS[BN6].tag : "").append(l.kept[BN6] && l.kept[BN5] ? " and " : "")
                .append(l.kept[BN5] ? ROMS[BN5].tag : "").append(l.where != null ? " from " + l.where : "").append('.');
        } else if (kept(ctx, BN6)) {
            if (kept(ctx, BN5)) s.append("Both ROMs are in already.");
            else s.append("No ").append(ROMS[BN5].tag).append(l.where != null ? " in " + l.where : " there").append('.');
        } else if (l.where != null) {
            s.append(l.gba == 0 ? "No .gba file in " + l.where + " or the folders in it." : "No " + ROMS[BN6].tag + " in " + l.where + ".");
        } else s.append(l.files == 1 ? "That is not " : "None of those is ").append(ROMS[BN6].tag).append('.');
        int shown = 0;
        for (String r : l.refused) {
            if (shown == 3) break;
            s.append(' ').append(r).append('.');
            ++shown;
        }
        if (l.refused.size() > shown) s.append(" And ").append(l.refused.size() - shown).append(" more.");
        for (int k = 0; k < l.zipped.size() && k < 2; ++k) s.append(' ').append(l.zipped.get(k)).append(" is zipped: unzip it first.");
        return s.toString();
    }

    // ---- the saves' copy in the folder (src/launcher/mirror.h) ----

    /** The child document of the folder named `name`, null for none. */
    private Uri child(Uri tree, String name) {
        String doc = DocumentsContract.getTreeDocumentId(tree);
        try (Cursor c = ctx.getContentResolver().query(DocumentsContract.buildChildDocumentsUriUsingTree(tree, doc),
                new String[] { Document.COLUMN_DOCUMENT_ID, Document.COLUMN_DISPLAY_NAME }, null, null, null)) {
            while (c != null && c.moveToNext())
                if (name.equals(c.getString(1))) return DocumentsContract.buildDocumentUriUsingTree(tree, c.getString(0));
        } catch (Exception e) { /* (none to be read) */ }
        return null;
    }

    /** The folder's saves' copy, a reinstall's, copied to the data folder's
     *  found.cwsave (a new one, where a write was cut short before its
     *  rename, as well as none). */
    boolean fetchSaves(Uri tree) {
        return fetchSaves(tree, new File(dataDir(ctx), "found.cwsave"));
    }

    boolean fetchSaves(Uri tree, File to) {
        Uri saves = child(tree, SAVES);
        if (saves == null) saves = child(tree, SAVES + ".new");
        if (saves == null) return false;
        if (copySaves(saves, to)) return true;
        /* The present file remains untouched and gets the common refusal
         * screen; auto-export cannot erase an unreadable synced-in file. */
        try (OutputStream out = new FileOutputStream(to)) {
            out.write("The incoming saves file could not be read.".getBytes(StandardCharsets.US_ASCII));
            return true;
        } catch (Exception e) { return false; }
    }

    /** Only stages bytes: validation and the comparison stay in C on
     *  every system. A failed read leaves the old staged file alone. */
    boolean copySaves(Uri saves, File to) {
        File part = new File(to.getPath() + ".part");
        dataDir(ctx).mkdirs();
        try (InputStream in = ctx.getContentResolver().openInputStream(saves); OutputStream out = new FileOutputStream(part)) {
            if (in == null) return false;
            byte[] buf = new byte[1 << 16];
            long n = 0;
            for (int k; (k = in.read(buf)) > 0 && n <= SAVES_MAX; n += k) out.write(buf, 0, k);
            if (n == 0 || n > SAVES_MAX) { part.delete(); return false; }
        } catch (Exception e) {
            part.delete();
            return false;
        }
        return part.renameTo(to);
    }

    String fileName(Uri uri) {
        try (Cursor c = ctx.getContentResolver().query(uri, new String[] { OpenableColumns.DISPLAY_NAME }, null, null, null)) {
            if (c != null && c.moveToFirst() && !c.isNull(0)) return c.getString(0);
        } catch (Exception e) { /* (named generically) */ }
        return SAVES;
    }

    /** The saves' file `from` written into the folder kept as SAVES: a new
     *  one written whole, then put in the old one's place; where the folder's
     *  provider renames nothing, written over the old one. False where no
     *  folder is kept, its access is read-only, or it fails. */
    boolean putSaves(File from) {
        Uri tree = savesFolder();
        if (tree == null || !granted(tree, true) || !from.isFile()) return false;
        try {
            Uri parent = DocumentsContract.buildDocumentUriUsingTree(tree, DocumentsContract.getTreeDocumentId(tree));
            Uri old = child(tree, SAVES), stale = child(tree, SAVES + ".new");
            if (stale != null) DocumentsContract.deleteDocument(ctx.getContentResolver(), stale);
            Uri fresh = DocumentsContract.createDocument(ctx.getContentResolver(), parent, "application/octet-stream", SAVES + ".new");
            if (fresh == null || !write(from, fresh)) return false;
            if (!renames(fresh)) {
                DocumentsContract.deleteDocument(ctx.getContentResolver(), fresh);
                Uri into = old != null ? old : DocumentsContract.createDocument(ctx.getContentResolver(), parent, "application/octet-stream", SAVES);
                return into != null && write(from, into);
            }
            if (old != null) DocumentsContract.deleteDocument(ctx.getContentResolver(), old);
            return DocumentsContract.renameDocument(ctx.getContentResolver(), fresh, SAVES) != null;
        } catch (Exception e) {
            Log.w(TAG, "saves: the copy in " + savesFolderName() + " could not be written: " + e.getMessage());
            return false;
        }
    }

    private boolean renames(Uri doc) {
        try (Cursor c = ctx.getContentResolver().query(doc, new String[] { Document.COLUMN_FLAGS }, null, null, null)) {
            return c != null && c.moveToFirst() && (c.getInt(0) & Document.FLAG_SUPPORTS_RENAME) != 0;
        } catch (Exception e) {
            return false;
        }
    }

    boolean write(File from, Uri to) {
        try (InputStream in = new FileInputStream(from); OutputStream out = ctx.getContentResolver().openOutputStream(to, "wt")) {
            if (out == null) return false;
            byte[] buf = new byte[1 << 16];
            for (int k; (k = in.read(buf)) > 0; ) out.write(buf, 0, k);
            return true;
        } catch (Exception e) {
            return false;
        }
    }
}
