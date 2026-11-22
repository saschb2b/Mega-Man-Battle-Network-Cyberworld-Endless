package io.github.saschb2b.cyberworldendless;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.net.URI;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLParameters;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;

/** TCP/TLS bytes for the game's WebSocket codec. No cookies or saved traffic. */
public final class OnlineStream {
    private final SecureRandom random = new SecureRandom();
    private final ExecutorService writer = Executors.newSingleThreadExecutor(job -> {
        Thread thread = new Thread(job, "Square write");
        thread.setDaemon(true);
        return thread;
    });
    private volatile Socket socket;
    private volatile boolean closed;
    private volatile int state;

    private synchronized boolean use(Socket next) throws IOException {
        if (closed) { next.close(); return false; }
        socket = next;
        return true;
    }

    /** DNS/connect/TLS run separately so the native caller can cancel promptly. */
    public void open(String address) {
        Thread connector = new Thread(() -> connect(address), "Square connect");
        connector.setDaemon(true);
        connector.start();
    }

    private void connect(String address) {
        try {
            URI uri = new URI(address);
            boolean secure = "wss".equals(uri.getScheme());
            if (!secure && !"ws".equals(uri.getScheme())) throw new IOException();
            String host = uri.getHost();
            if (host == null || uri.getUserInfo() != null || !"/presence".equals(uri.getPath())) throw new IOException();
            int port = uri.getPort() < 0 ? (secure ? 443 : 80) : uri.getPort();
            Socket raw = new Socket();
            if (!use(raw)) return;
            InetSocketAddress target = new InetSocketAddress(host, port);
            if (closed) return;
            raw.connect(target, 5000);
            raw.setTcpNoDelay(true);
            raw.setSoTimeout(5000);
            Socket ready = secure ? tls(raw, host, port) : raw;
            ready.setSoTimeout(1);
            synchronized (this) { if (!closed) state = 1; }
        } catch (Exception failure) { close(); }
    }

    private Socket tls(Socket raw, String host, int port) throws Exception {
        SSLSocket secure = (SSLSocket)((SSLSocketFactory)SSLSocketFactory.getDefault()).createSocket(raw, host, port, true);
        if (!use(secure)) throw new IOException();
        SSLParameters parameters = secure.getSSLParameters();
        boolean verifyAfter = false;
        try {
            /* Android added this method at API24; API21-23 use its verifier. */
            SSLParameters.class.getMethod("setEndpointIdentificationAlgorithm", String.class).invoke(parameters, "HTTPS");
            secure.setSSLParameters(parameters);
        } catch (NoSuchMethodException oldAndroid) { verifyAfter = true; }
        secure.startHandshake();
        if (verifyAfter && !HttpsURLConnection.getDefaultHostnameVerifier().verify(host, secure.getSession())) throw new IOException();
        return secure;
    }

    public int status() { return state; }

    /** OutputStream has no write timeout: its sole writer is bounded here. */
    public boolean send(byte[] bytes) {
        if (state != 1 || bytes.length > 2048) return false;
        Future<Boolean> task = null;
        try {
            task = writer.submit(() -> { socket.getOutputStream().write(bytes); return true; });
            return task.get(2, TimeUnit.SECONDS);
        } catch (Exception failure) {
            if (task != null) task.cancel(true);
            close();
            return false;
        }
    }

    /** Empty means pending, null means closed; a one-ms read detects EOF too. */
    public byte[] receive(int maximum) {
        if (state != 1 || maximum <= 0 || maximum > 2048) return null;
        byte[] bytes = new byte[maximum];
        try {
            int count = socket.getInputStream().read(bytes);
            if (count < 0) { close(); return null; }
            return Arrays.copyOf(bytes, count);
        } catch (SocketTimeoutException pending) { return new byte[0]; }
        catch (IOException failure) { close(); return null; }
    }

    public byte[] random(int length) {
        if (length < 0 || length > 64) return null;
        byte[] bytes = new byte[length];
        random.nextBytes(bytes);
        return bytes;
    }

    public synchronized void close() {
        closed = true;
        state = -1;
        if (socket != null) try { socket.close(); } catch (IOException ignored) { }
        writer.shutdownNow();
    }
}
