#!/usr/bin/env python3
"""ROM-free regressions of RomLook against failing document-provider doubles.

Runs the production Java adapter on a JVM. The doubles model the small
Android surface it uses, including null cursors, thrown provider errors,
and read failures. They do not claim device/provider integration coverage.
Run with a JDK, or: python3 tests/test_android_saves.py --docker
"""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
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
    public abstract void mutation();
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
    public static boolean deleteDocument(ContentResolver resolver, Uri uri) { resolver.mutation(); return true; }
    public static Uri createDocument(ContentResolver resolver, Uri parent, String type, String name) { resolver.mutation(); return Uri.parse(parent + "/" + name); }
    public static Uri renameDocument(ContentResolver resolver, Uri uri, String name) { resolver.mutation(); return Uri.parse(uri + "/" + name); }
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
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class ProviderTest {
    private static final Uri TREE = Uri.parse("content://provider/tree/transfer");
    private static final byte[] INCOMING = { 1, 2, 3, 4, 5 };

    private static class Rows implements Cursor {
        private final boolean present, broken;
        private boolean read;
        Rows(boolean present, boolean broken) { this.present = present; this.broken = broken; }
        public boolean moveToNext() {
            if (broken) throw new SecurityException("provider cursor failed");
            boolean found = present && !read;
            read = true;
            return found;
        }
        public boolean moveToFirst() { read = false; return moveToNext(); }
        public String getString(int index) { return index == 0 ? "incoming" : RomLook.SAVES; }
        public long getLong(int index) { return 0; }
        public int getInt(int index) { return 64; }
        public boolean isNull(int index) { return false; }
    }

    private static class Provider extends ContentResolver {
        int queries, mutations, failAt;
        String failure;
        boolean present = true, unreadable;
        void reset(String failure, int failAt) { this.failure = failure; this.failAt = failAt; queries = mutations = 0; }
        public Cursor query(Uri uri, String[] columns, String selection, String[] args, String order) {
            ++queries;
            if (queries == failAt) {
                if ("null".equals(failure)) return null;
                if ("throw".equals(failure)) throw new SecurityException("provider unavailable");
                if ("cursor".equals(failure)) return new Rows(present, true);
            }
            return new Rows(present, false);
        }
        public InputStream openInputStream(Uri uri) throws FileNotFoundException {
            if (unreadable) throw new FileNotFoundException("incoming file unavailable");
            return new ByteArrayInputStream(INCOMING);
        }
        public OutputStream openOutputStream(Uri uri, String mode) { ++mutations; return new ByteArrayOutputStream(); }
        public List<UriPermission> getPersistedUriPermissions() { return Collections.singletonList(new UriPermission(TREE)); }
        public void mutation() { ++mutations; }
    }

    public static void main(String[] args) throws Exception {
        File files = new File(args[0]);
        Files.createDirectory(files.toPath());
        Provider provider = new Provider();
        Context context = new Context(files, provider);
        context.getSharedPreferences("saves", 0).edit().putString("folder", TREE.toString()).apply();
        RomLook adapter = new RomLook(context);
        File staged = new File(RomLook.dataDir(context), "found.cwsave");
        staged.getParentFile().mkdirs();
        assert adapter.fetchSaves(TREE, staged);
        assert Arrays.equals(INCOMING, Files.readAllBytes(staged.toPath()));
        provider.present = false;
        provider.reset(null, 0);
        assert !adapter.fetchSaves(TREE, staged) && provider.queries == 2;
        assert Arrays.equals(INCOMING, Files.readAllBytes(staged.toPath()));
        provider.present = true;
        for (String failure : new String[] { "null", "throw", "cursor" }) {
            provider.reset(failure, 1);
            assert adapter.fetchSaves(TREE, staged) && provider.queries == 1;
            assert new String(Files.readAllBytes(staged.toPath()), java.nio.charset.StandardCharsets.US_ASCII).contains("could not be read");
            assert provider.mutations == 0;
            // The next lookup recovers, but the first scan produced a refusal,
            // rather than falsely claiming the incoming file was absent.
            assert adapter.fetchSaves(TREE, staged);
            assert Arrays.equals(INCOMING, Files.readAllBytes(staged.toPath()));
        }
        provider.reset(null, 0);
        provider.unreadable = true;
        assert adapter.fetchSaves(TREE, staged) && provider.mutations == 0;
        provider.unreadable = false;
        File from = new File(files, "export.cwsave");
        Files.write(from.toPath(), new byte[] { 9, 9, 9 });
        for (String failure : new String[] { "null", "throw", "cursor" }) {
            for (int at : new int[] { 1, 2 }) {
                provider.reset(failure, at);
                assert !adapter.putSaves(from) && provider.queries == at;
                assert provider.mutations == 0;
            }
        }
        System.out.println("android saves: missing, null/error listing, cursor/read failure, and blocked replacement passed");
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--docker', action='store_true', help='use the existing cyberworld-android build image')
    args = parser.parse_args()
    if not args.docker and not shutil.which('javac'):
        parser.error('a JDK is required; use --docker with the Android build image')
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
        adapter = ROOT / 'android/app/src/main/java/io/github/saschb2b/cyberworldendless/RomLook.java'
        if args.docker:
            prefix = ['docker', 'run', '--rm', '--user', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src:ro', '-v', f'{work}:/test', '-w', '/test', 'cyberworld-android']
            sources = [str(Path('/test') / name) for name in STUBS] + ['/test/ProviderTest.java', '/src/' + str(adapter.relative_to(ROOT))]
            classes = '/test/classes'
        else:
            prefix = []
            sources = [str(directory / name) for name in STUBS] + [str(harness), str(adapter)]
            classes = str(directory / 'classes')
        subprocess.run(prefix + ['javac', '-Xlint:all', '-Werror', '-d', classes] + sources, check=True)
        files = '/test/provider' if args.docker else str(directory / 'provider')
        subprocess.run(prefix + ['java', '-ea', '-cp', classes, 'io.github.saschb2b.cyberworldendless.ProviderTest', files], check=True)


if __name__ == '__main__':
    main()
