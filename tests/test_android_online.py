#!/usr/bin/env python3
"""Run Android's production TCP/TLS adapter on the JVM, with local sockets.

Checks bytes, EOF, cancel, untrusted certificates and wrong hostnames. This
does not replace an Android device test. Run with a JDK, or --docker to use
the repository's cyberworld-android build image.
"""
import argparse
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'android/app/src/main/java/io/github/saschb2b/cyberworldendless/OnlineStream.java'
HARNESS = r'''
import io.github.saschb2b.cyberworldendless.OnlineStream;
import java.io.*;
import java.net.*;
import java.security.*;
import java.util.Arrays;
import javax.net.ssl.*;

public final class OnlineStreamTest {
    static void check(boolean passed, String what) {
        if (!passed) throw new AssertionError(what);
    }
    static int ready(OnlineStream stream) throws Exception {
        long start = System.nanoTime();
        while (stream.status() == 0 && System.nanoTime() - start < 8_000_000_000L) Thread.sleep(5);
        return stream.status();
    }
    static byte[] received(OnlineStream stream) throws Exception {
        long start = System.nanoTime();
        byte[] got;
        do { got = stream.receive(128); }
        while (got != null && got.length == 0 && System.nanoTime() - start < 2_000_000_000L);
        return got;
    }
    static Thread echo(ServerSocket listener) {
        Thread thread = new Thread(() -> {
            try (Socket peer = listener.accept()) {
                peer.setSoTimeout(3000);
                byte[] data = peer.getInputStream().readNBytes(4);
                peer.getOutputStream().write(data);
            } catch (IOException rejected) { }
        });
        thread.setDaemon(true);
        thread.start();
        return thread;
    }
    static void plain() throws Exception {
        try (ServerSocket listener = new ServerSocket(0, 1, InetAddress.getLoopbackAddress())) {
            Thread server = echo(listener);
            OnlineStream stream = new OnlineStream();
            stream.open("ws://localhost:" + listener.getLocalPort() + "/presence");
            check(ready(stream) == 1, "plain connection");
            check(stream.receive(128).length == 0, "pending read");
            byte[] bytes = {0, 1, (byte)0x80, (byte)0xff};
            check(stream.send(bytes), "binary send");
            check(Arrays.equals(bytes, received(stream)), "binary receive");
            check(received(stream) == null, "EOF detected");
            stream.close();
            server.join(1000);
        }
        OnlineStream cancelled = new OnlineStream();
        cancelled.open("wss://localhost:1/presence");
        cancelled.close();
        check(cancelled.status() == -1, "cancel closes pending connect");
    }
    static SSLServerSocket listener(String keystore) throws Exception {
        KeyStore keys = KeyStore.getInstance("PKCS12");
        try (InputStream input = new FileInputStream(keystore)) { keys.load(input, "testpass".toCharArray()); }
        KeyManagerFactory managers = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        managers.init(keys, "testpass".toCharArray());
        SSLContext tls = SSLContext.getInstance("TLS");
        tls.init(managers.getKeyManagers(), null, null);
        return (SSLServerSocket)tls.getServerSocketFactory().createServerSocket(0, 1, InetAddress.getLoopbackAddress());
    }
    static void tls(String keystore, String host, boolean accepted) throws Exception {
        try (SSLServerSocket listener = listener(keystore)) {
            Thread server = echo(listener);
            OnlineStream stream = new OnlineStream();
            stream.open("wss://" + host + ":" + listener.getLocalPort() + "/presence");
            check((ready(stream) == 1) == accepted, "TLS trust/hostname: " + host);
            if (accepted) {
                byte[] bytes = {1, 2, 3, 4};
                check(stream.send(bytes) && Arrays.equals(bytes, received(stream)), "TLS binary exchange");
            }
            stream.close();
            server.join(1000);
        }
    }
    public static void main(String[] args) throws Exception {
        if (args.length == 1) { plain(); tls(args[0], "localhost", false); }
        else { tls(args[0], "localhost", true); tls(args[0], "127.0.0.1", false); }
        System.out.println("Android stream: sockets, cancellation and TLS boundaries passed");
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--docker', action='store_true')
    options = parser.parse_args()
    (ROOT / '.build').mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='android-online-', dir=ROOT / '.build') as directory:
        folder = Path(directory)
        (folder / 'OnlineStreamTest.java').write_text(HARNESS)
        def path(item):
            return '/src/' + str(Path(item).relative_to(ROOT)) if options.docker else str(item)
        def run(*arguments):
            command = list(arguments)
            if options.docker:
                command = ['docker', '--context', 'default', 'run', '--rm', '-u', '1000:1000',
                           '-v', f'{ROOT}:/src', '-w', '/src', 'cyberworld-android'] + command
            subprocess.run(command, check=True, timeout=60)
        key = path(folder / 'server.p12')
        run('keytool', '-genkeypair', '-alias', 'local', '-keyalg', 'RSA', '-keysize', '2048',
            '-storetype', 'PKCS12', '-keystore', key, '-storepass', 'testpass', '-keypass', 'testpass',
            '-dname', 'CN=localhost', '-ext', 'SAN=dns:localhost', '-validity', '1', '-noprompt')
        run('javac', '-Xlint:all', '-Werror', '-d', path(folder), path(SOURCE), path(folder / 'OnlineStreamTest.java'))
        run('java', '-cp', path(folder), 'OnlineStreamTest', key)
        run('java', f'-Djavax.net.ssl.trustStore={key}', '-Djavax.net.ssl.trustStorePassword=testpass',
            '-cp', path(folder), 'OnlineStreamTest', key, 'trusted')


if __name__ == '__main__':
    main()
