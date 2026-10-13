#!/usr/bin/env python3
"""ROM-free regressions of RomLook's saves against document-provider doubles.

Runs the production Java adapter on a JVM. The doubles model the small
Android surface it uses, including null cursors, thrown provider errors,
revoked grants and read and write failures. They do not claim
device/provider integration coverage. With --docker, the app's own Java
(GameActivity with SDL's activity) is also compiled against Android's
android.jar, and its sources are checked for the JNI contract
src/launcher/pick_android.c calls.
Run with a JDK, or: python3 tests/test_android_saves.py --docker
"""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
JAVA = ROOT / 'android/app/src/main/java/io/github/saschb2b/cyberworldendless'
STUBS = {
    'android/net/Uri.java': '''package android.net;
public final class Uri {
    private final String text;
    private Uri(String text) { this.text = text; }
    public static Uri parse(String text) { return new Uri(text); }
    public String toString() { return text; }
    public boolean equals(Object other) { return other instanceof Uri && text.equals(other.toString()); }
    public int hashCode() { return text.hashCode(); }
}''',
    'android/content/Intent.java': '''package android.content;
public class Intent {
    public static final int FLAG_GRANT_READ_URI_PERMISSION = 1, FLAG_GRANT_WRITE_URI_PERMISSION = 2;
}''',
    'android/content/UriPermission.java': '''package android.content;
import android.net.Uri;
public class UriPermission {
    private final Uri uri;
    public UriPermission(Uri uri) { this.uri = uri; }
    public Uri getUri() { return uri; }
    public boolean isReadPermission() { return true; }
    public boolean isWritePermission() { return true; }
}''',
    'android/content/SharedPreferences.java': '''package android.content;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
public class SharedPreferences {
    private final Map<String, Object> values = new HashMap<>();
    public String getString(String key, String fallback) { return (String)values.getOrDefault(key, fallback); }
    @SuppressWarnings("unchecked")
    public Set<String> getStringSet(String key, Set<String> fallback) { return (Set<String>)values.getOrDefault(key, fallback); }
    public boolean contains(String key) { return values.containsKey(key); }
    public Editor edit() { return new Editor(); }
    public class Editor {
        public Editor putString(String key, String value) { values.put(key, value); return this; }
        public Editor putStringSet(String key, Set<String> value) { values.put(key, value); return this; }
        public void apply() { }
    }
}''',
    'android/content/Context.java': '''package android.content;
import java.io.File;
import java.util.HashMap;
import java.util.Map;
public class Context {
    public static final int MODE_PRIVATE = 0;
    private final File files;
    private final ContentResolver resolver;
    private final Map<String, SharedPreferences> prefs = new HashMap<>();
    public Context(File files, ContentResolver resolver) { this.files = files; this.resolver = resolver; }
    public File getFilesDir() { return files; }
    public ContentResolver getContentResolver() { return resolver; }
    public SharedPreferences getSharedPreferences(String name, int mode) { return prefs.computeIfAbsent(name, ignored -> new SharedPreferences()); }
}''',
    'android/content/ContentResolver.java': '''package android.content;
import android.database.Cursor;
import android.net.Uri;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileNotFoundException;
import java.util.List;
public abstract class ContentResolver {
    public abstract Cursor query(Uri uri, String[] columns, String selection, String[] args, String order);
    public abstract InputStream openInputStream(Uri uri) throws FileNotFoundException;
    public abstract OutputStream openOutputStream(Uri uri, String mode) throws FileNotFoundException;
    public abstract List<UriPermission> getPersistedUriPermissions();
    public void takePersistableUriPermission(Uri uri, int flags) { }
    public void releasePersistableUriPermission(Uri uri, int flags) { }
    public abstract void mutation(String what, Uri uri);
}''',
    'android/database/Cursor.java': '''package android.database;
public interface Cursor extends AutoCloseable {
    boolean moveToNext();
    boolean moveToFirst();
    String getString(int index);
    long getLong(int index);
    int getInt(int index);
    boolean isNull(int index);
    default void close() { }
}''',
    'android/provider/OpenableColumns.java': '''package android.provider;
public class OpenableColumns { public static final String DISPLAY_NAME = "name", SIZE = "size"; }''',
    'android/provider/DocumentsContract.java': '''package android.provider;
import android.content.ContentResolver;
import android.net.Uri;
public class DocumentsContract {
    public static String getTreeDocumentId(Uri tree) { return "tree"; }
    public static Uri buildDocumentUriUsingTree(Uri tree, String id) { return Uri.parse(tree + "/doc/" + id); }
    public static Uri buildChildDocumentsUriUsingTree(Uri tree, String id) { return Uri.parse(tree + "/children/" + id); }
    public static boolean deleteDocument(ContentResolver resolver, Uri uri) { resolver.mutation("delete", uri); return true; }
    public static Uri createDocument(ContentResolver resolver, Uri parent, String type, String name) { resolver.mutation("create", parent); return Uri.parse(parent + "/" + name); }
    public static Uri renameDocument(ContentResolver resolver, Uri uri, String name) { resolver.mutation("rename", uri); return Uri.parse(uri + "/" + name); }
    public static class Document {
        public static final String COLUMN_DOCUMENT_ID = "id", COLUMN_DISPLAY_NAME = "name", COLUMN_MIME_TYPE = "mime", COLUMN_SIZE = "size", COLUMN_LAST_MODIFIED = "date", COLUMN_FLAGS = "flags";
        public static final String MIME_TYPE_DIR = "directory";
        public static final int FLAG_SUPPORTS_RENAME = 64;
    }
}''',
    'android/util/Log.java': '''package android.util;
public class Log {
    public static int w(String tag, String message) { return 0; }
    public static int i(String tag, String message) { return 0; }
}''',
}

HARNESS = '''package io.github.saschb2b.cyberworldendless;
import android.content.ContentResolver;
import android.content.Context;
import android.content.UriPermission;
import android.database.Cursor;
import android.net.Uri;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ProviderTest {
    private static final Uri TREE = Uri.parse("content://provider/tree/transfer");
    private static final byte[] INCOMING = { 1, 2, 3, 4, 5 };

    /* a folder's listing (its children's names), or one document's row */
    private static class Rows implements Cursor {
        private final List<String> names;
        private final boolean broken, doc;
        private int at = -1;
        Rows(List<String> names, boolean broken, boolean doc) { this.names = names; this.broken = broken; this.doc = doc; }
        public boolean moveToNext() {
            if (broken) throw new SecurityException("provider cursor failed");
            return ++at < names.size();
        }
        public boolean moveToFirst() { at = -1; return moveToNext(); }
        public String getString(int index) {
            String name = names.get(at);
            if (doc) return name;
            return index == 0 ? "id-" + name : index == 1 ? name : "application/octet-stream";
        }
        public long getLong(int index) { return 0; }
        public int getInt(int index) { return 64; }
        public boolean isNull(int index) { return false; }
    }

    private static class Provider extends ContentResolver {
        int queries, failAt, opens;
        String failure, readFailure;
        boolean granted = true, writeFails;
        byte[] incoming = INCOMING;
        List<String> children = new ArrayList<>(Collections.singletonList(RomLook.SAVES));
        final List<String> mutations = new ArrayList<>();
        final List<Uri> opened = new ArrayList<>();
        final Map<Uri, ByteArrayOutputStream> written = new HashMap<>();
        void reset(String failure, int failAt) {
            this.failure = failure; this.failAt = failAt; queries = opens = 0;
            mutations.clear(); opened.clear(); written.clear();
        }
        public Cursor query(Uri uri, String[] columns, String selection, String[] args, String order) {
            ++queries;
            boolean listing = uri.toString().contains("/children/");
            List<String> rows = listing ? children : Collections.singletonList(RomLook.SAVES);
            if (queries == failAt) {
                if ("null".equals(failure)) return null;
                if ("throw".equals(failure)) throw new SecurityException("provider unavailable");
                if ("cursor".equals(failure)) return new Rows(rows, true, !listing);
            }
            return new Rows(rows, false, !listing);
        }
        public InputStream openInputStream(Uri uri) throws FileNotFoundException {
            ++opens;
            opened.add(uri);
            if ("open".equals(readFailure)) throw new FileNotFoundException("incoming file unavailable");
            if ("null".equals(readFailure)) return null;
            if ("read".equals(readFailure)) return new InputStream() {
                private int left = 2;
                public int read() throws IOException {
                    if (left-- > 0) return 7;
                    throw new IOException("provider read failed");
                }
            };
            return new ByteArrayInputStream(incoming);
        }
        public OutputStream openOutputStream(Uri uri, String mode) throws FileNotFoundException {
            mutation("write", uri);
            if (writeFails) throw new FileNotFoundException("document unavailable");
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            written.put(uri, out);
            return out;
        }
        public List<UriPermission> getPersistedUriPermissions() {
            return granted ? Collections.singletonList(new UriPermission(TREE)) : Collections.emptyList();
        }
        public void mutation(String what, Uri uri) { mutations.add(what + " " + uri); }
    }

    private static byte[] read(File f) throws IOException { return Files.readAllBytes(f.toPath()); }

    /* savesGet's answer, and nothing at `to` (nor a .part) unless it is 1 */
    private static int get(RomLook adapter, File to) throws IOException {
        Files.write(to.toPath(), "an older copy".getBytes(java.nio.charset.StandardCharsets.US_ASCII));
        int got = adapter.getSaves(to);
        assert !new File(to.getPath() + ".part").exists() : "a .part was left";
        assert got == 1 || !to.exists() : "something was left at to for " + got;
        return got;
    }

    public static void main(String[] args) throws Exception {
        File files = new File(args[0]);
        Files.createDirectory(files.toPath());
        Provider provider = new Provider();
        Context context = new Context(files, provider);
        context.getSharedPreferences("saves", 0).edit().putString("folder", TREE.toString()).apply();
        RomLook adapter = new RomLook(context);
        File to = new File(RomLook.dataDir(context), "saves-found.cwsave");
        to.getParentFile().mkdirs();
        int checks = 0;

        // readable: copied whole, from one listing, nothing written to the folder
        provider.reset(null, 0);
        assert get(adapter, to) == 1 && Arrays.equals(INCOMING, read(to));
        assert provider.queries == 1 && provider.mutations.isEmpty();
        ++checks;
        // a write cut short before its rename: SAVES.new read where SAVES is not
        provider.children = new ArrayList<>(Arrays.asList("notes.txt", RomLook.SAVES + ".new"));
        provider.reset(null, 0);
        assert get(adapter, to) == 1 && provider.opened.get(0).toString().endsWith("id-" + RomLook.SAVES + ".new");
        ++checks;
        // ... and SAVES first where both are there, whatever their order
        provider.children = new ArrayList<>(Arrays.asList(RomLook.SAVES + ".new", RomLook.SAVES));
        provider.reset(null, 0);
        assert get(adapter, to) == 1 && provider.opened.get(0).toString().endsWith("id-" + RomLook.SAVES);
        ++checks;

        // absent: 0, nothing read or written
        provider.children = new ArrayList<>(Collections.singletonList("notes.txt"));
        provider.reset(null, 0);
        assert get(adapter, to) == 0 && provider.queries == 1 && provider.opens == 0 && provider.mutations.isEmpty();
        ++checks;
        provider.children = new ArrayList<>(Collections.singletonList(RomLook.SAVES));

        // a folder that cannot be listed: 0, as a write there fails too
        for (String failure : new String[] { "null", "throw", "cursor" }) {
            provider.reset(failure, 1);
            assert get(adapter, to) == 0 : failure;
            assert provider.queries == 1 && provider.opens == 0 && provider.mutations.isEmpty() : failure;
            ++checks;
        }
        // ... a grant taken back: 0 before any listing
        provider.granted = false;
        provider.reset(null, 0);
        assert get(adapter, to) == 0 && provider.queries == 0 && provider.opens == 0;
        ++checks;
        provider.granted = true;
        // ... no saves folder kept: 0
        Provider none = new Provider();
        assert get(new RomLook(new Context(new File(files, "none"), none)), to) == 0 && none.queries == 0;
        ++checks;

        // there but unreadable: -1, and no marker file left in its place
        for (String failure : new String[] { "open", "null", "read" }) {
            provider.readFailure = failure;
            provider.reset(null, 0);
            assert get(adapter, to) == -1 : failure;
            assert provider.opens == 1 && provider.mutations.isEmpty() : failure;
            ++checks;
        }
        provider.readFailure = null;
        // ... empty, or past SAVES_MAX's 16 MB
        provider.incoming = new byte[0];
        provider.reset(null, 0);
        assert get(adapter, to) == -1;
        provider.incoming = new byte[(int)RomLook.SAVES_MAX + 1];
        provider.reset(null, 0);
        assert get(adapter, to) == -1;
        provider.incoming = INCOMING;
        checks += 2;
        // ... and the next read, with the provider well again, copies it
        provider.reset(null, 0);
        assert get(adapter, to) == 1 && Arrays.equals(INCOMING, read(to));
        ++checks;

        // the ROM folder's look (a pick's and a start's) leaves the saves to savesGet
        context.getSharedPreferences("roms", 0).edit().putString("folder", TREE.toString()).apply();
        Files.delete(to.toPath());
        for (boolean pick : new boolean[] { true, false }) {
            provider.reset(null, 0);
            RomLook.Look l = adapter.lookFolder(TREE, "ROMs", pick);
            assert l.lost == null && provider.opens == 0 && provider.mutations.isEmpty() : "look " + pick;
            assert !new File(RomLook.dataDir(context), "found.cwsave").exists() : "look " + pick;
            String[] left = RomLook.dataDir(context).list();
            assert left != null && left.length == 0 : "look " + pick + " left " + Arrays.toString(left);
            ++checks;
        }

        // an export: written whole, or its created document deleted again
        File from = new File(files, "export.cwsave");
        Files.write(from.toPath(), new byte[] { 9, 9, 9 });
        Uri doc = Uri.parse("content://provider/document/picked");
        provider.reset(null, 0);
        assert adapter.export(from, doc) && Arrays.equals(new byte[] { 9, 9, 9 }, provider.written.get(doc).toByteArray());
        assert !provider.mutations.contains("delete " + doc);
        ++checks;
        provider.writeFails = true;
        provider.reset(null, 0);
        assert !adapter.export(from, doc) && provider.mutations.contains("delete " + doc);
        provider.writeFails = false;
        provider.reset(null, 0);
        assert !adapter.export(null, doc) && provider.mutations.contains("delete " + doc);
        provider.reset(null, 0);
        assert !adapter.export(new File(files, "gone.cwsave"), doc) && provider.mutations.contains("delete " + doc);
        checks += 3;

        // the saves written into the folder: whole, then renamed over the old one
        provider.reset(null, 0);
        assert adapter.putSaves(from);
        assert provider.mutations.get(provider.mutations.size() - 1).startsWith("rename ");
        ++checks;
        // ... never where the folder cannot be listed
        for (String failure : new String[] { "null", "throw", "cursor" }) {
            for (int at : new int[] { 1, 2 }) {
                provider.reset(failure, at);
                assert !adapter.putSaves(from) && provider.queries == at;
                assert provider.mutations.isEmpty();
                ++checks;
            }
        }
        System.out.println("android saves: " + checks + " checks passed (savesGet 1/0/-1, no marker, no found.cwsave, export cleanup, putSaves)");
    }
}
'''


def check_sources():
    """The JNI contract pick_android.c calls, read off the sources."""
    game = (JAVA / 'GameActivity.java').read_text()
    look = (JAVA / 'RomLook.java').read_text()
    for signature in ('public int savesGet(String to)', 'public boolean savesPut(String from)',
                      'public boolean savesPick(int kind, String from)', 'public String savesResult()',
                      'public String savesFolder()', 'public String savesScope()'):
        assert signature in game, f'GameActivity lacks {signature}'
    assert 'savesDevice' not in game, 'savesDevice is back: the device model must not reach the saves'
    for path in JAVA.glob('*.java'):
        assert 'Build.MODEL' not in path.read_text(), f'{path.name} reads Build.MODEL'
    assert 'found.cwsave' not in look and 'fetchSaves' not in look, 'the ROM look copies the saves again'
    assert 'onSaveInstanceState' in game, 'an export in progress is not kept over a recreated activity'
    for text in re.findall(r'savesDone\(-2, "([^"]*)"', game) + re.findall(r': "(The [^"]*)"\);', game):
        assert '-' not in text and '—' not in text, f'a dash in the error shown: {text}'
    print('android saves: the JNI contract in the sources checked', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--docker', action='store_true', help='use the existing cyberworld-android build image')
    parser.add_argument('--name', help='a name for the containers --docker starts (NAME-app, NAME-javac, NAME-java)')
    args = parser.parse_args()
    if not args.docker and not shutil.which('javac'):
        parser.error('a JDK is required; use --docker with the Android build image')
    check_sources()
    build = ROOT / '.build'
    if args.docker:
        build.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='cw-android-provider-', dir=build if args.docker else None) as work:
        directory = Path(work)
        for name, source in STUBS.items():
            path = directory / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(source)
        harness = directory / 'ProviderTest.java'
        harness.write_text(HARNESS)
        adapter = JAVA / 'RomLook.java'
        if args.docker:
            def prefix(step):
                named = ['--name', f'{args.name}-{step}'] if args.name else []
                return ['docker', 'run', '--rm', *named, '--user', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src:ro',
                        '-v', f'{work}:/test', '-w', '/test', 'cyberworld-android']
            sources = [str(Path('/test') / name) for name in STUBS] + ['/test/ProviderTest.java', '/src/' + str(adapter.relative_to(ROOT))]
            classes = '/test/classes'
            # the app's own Java compiled as Gradle does, Java 11 on Android's API beside
            # SDL's activity (SDL's own warnings are SDL's: the app's are errors)
            app = ' '.join('/src/' + str(p.relative_to(ROOT)) for p in sorted(JAVA.glob('*.java')))
            subprocess.run(prefix('app') + ['sh', '-c', 'set -e; jar=$(ls /opt/android-sdk/platforms/*/android.jar | tail -n 1); '
                           'javac --release 11 -nowarn -cp "$jar" -d /test/sdl $(find /opt/sdl2-src/android-project/app/src/main/java -name "*.java"); '
                           f'javac --release 11 -Xlint:all,-options -Werror -cp "$jar:/test/sdl" -d /test/app {app}'], check=True)
            print('android saves: the app\'s Java compiled against android.jar', flush=True)
            run = prefix
        else:
            run = lambda step: []
            sources = [str(directory / name) for name in STUBS] + [str(harness), str(adapter)]
            classes = str(directory / 'classes')
        subprocess.run(run('javac') + ['javac', '-Xlint:all', '-Werror', '-d', classes] + sources, check=True)
        files = '/test/provider' if args.docker else str(directory / 'provider')
        subprocess.run(run('java') + ['java', '-ea', '-cp', classes, 'io.github.saschb2b.cyberworldendless.ProviderTest', files], check=True)


if __name__ == '__main__':
    main()
