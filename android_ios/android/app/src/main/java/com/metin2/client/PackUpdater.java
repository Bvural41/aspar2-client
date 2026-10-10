package com.metin2.client;

import android.os.StatFs;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.util.zip.CRC32;

/**
 * Oyun acilmadan once sunucudaki mobile_crclist'e bakar, degisen/eksik pack dosyalarini
 * (root.index, root.data, ...) mobile_pack klasorunden indirip uygulamanin harici dizinine acar.
 * PC patcher ile ayni dosya duzeni kullanilir:
 *
 *   {UPDATE_BASE_URL}mobile_crclist          satir: "crc32(hex) boyut x y dosyaadi"   (boyut = acilmis boyut)
 *   {UPDATE_BASE_URL}mobile_pack/<dosya>.lz  icerik: [DWORD gercek boyut][ham LZO1X akisi]
 *
 * mobile_pack icinde .lz yoksa ayni isimli ham dosya da denenir.
 */
public class PackUpdater {
    private static final String TAG = "PackUpdater";

    /** crclist'in ve mobile_pack klasorunun bulundugu surum klasoru (sonunda / olmali). */
    public static final String UPDATE_BASE_URL = "https://metin2plus.com/pe3qgb78x/patcher_01/0.0.0.1/";
    public static final String CRCLIST_NAME = "mobile_crclist";
    public static final String PACK_DIR_NAME = "mobile_pack/";

    private static final int CONNECT_TIMEOUT_MS = 15000;
    private static final int READ_TIMEOUT_MS = 20000;
    private static final int MAX_RETRY_PER_FILE = 3;

    public interface Listener {
        /** Genel durum satiri (ornek: "Sunucuya baglaniliyor..."). */
        void onStatus(String text);

        /**
         * Indirme ilerlemesi.
         * @param fileName aktif dosya
         * @param fileIndex 1 tabanli sira, fileCount toplam indirilecek dosya
         * @param allDone/allTotal toplam ilerleme (acilmis boyut cinsinden bayt)
         * @param netSpeedBps anlik ag hizi (bayt/sn)
         * @param etaSec tahmini kalan sure (sn), bilinmiyorsa -1
         */
        void onProgress(String fileName, int fileIndex, int fileCount,
                        long allDone, long allTotal, long netSpeedBps, long etaSec);

        /** Her sey hazir, oyuna girilebilir. */
        void onFinished(String version);

        /** Hata. canContinue: yerelde calisabilir pack var, "Oyuna Gir" gosterilebilir. */
        void onError(String message, boolean canContinue);
    }

    private static class Entry {
        String name;
        long size;    // Acilmis (uncompressed) boyut
        long crc;     // Acilmis CRC32
        long lzSize;  // Indirilecek (.lz) boyut
    }

    private final File mPackDir;
    private final Listener mListener;
    private volatile boolean mCancel = false;
    private Thread mThread;

    public PackUpdater(File packDir, Listener listener) {
        mPackDir = packDir;
        mListener = listener;
    }

    public void start() {
        mCancel = false;
        mThread = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    runUpdate();
                } catch (Throwable t) {
                    Log.e(TAG, "update failed", t);
                    mListener.onError("Hata: " + t.getMessage(), hasLocalPacks());
                }
            }
        }, "PackUpdater");
        mThread.start();
    }

    public void cancel() {
        mCancel = true;
    }

    public boolean hasLocalPacks() {
        return new File(mPackDir, "root.index").exists() && new File(mPackDir, "root.data").exists();
    }

    // ------------------------------------------------------------------------------------

    private void runUpdate() throws Exception {
        if (!mPackDir.exists()) mPackDir.mkdirs();

        mListener.onStatus("Sunucuya bağlanılıyor...");
        String listText;
        try {
            listText = new String(httpGetBytes(UPDATE_BASE_URL + CRCLIST_NAME + "?t=" + System.currentTimeMillis()), "UTF-8");
        } catch (Exception e) {
            Log.e(TAG, "crclist fetch failed", e);
            mListener.onError("Sunucuya bağlanılamadı.\nİnternet bağlantını kontrol et.", hasLocalPacks());
            return;
        }

        List<Entry> all = parseCrcList(listText);
        if (all.isEmpty()) {
            mListener.onError("Güncelleme listesi boş veya okunamadı.", hasLocalPacks());
            return;
        }

        // Hangi dosyalar guncellenmeli?
        Properties state = loadState();
        List<Entry> todo = new ArrayList<Entry>();
        long totalNeeded = 0;
        long maxSize = 0;
        for (int i = 0; i < all.size(); i++) {
            if (mCancel) return;
            Entry e = all.get(i);
            mListener.onStatus("Dosyalar kontrol ediliyor... (" + (i + 1) + "/" + all.size() + ")");
            if (isUpToDate(e, state)) continue;
            todo.add(e);
            totalNeeded += e.size;
            if (e.size > maxSize) maxSize = e.size;
        }
        saveState(state);

        if (todo.isEmpty()) {
            mListener.onFinished("");
            return;
        }

        // Disk alani: acilmis dosyalar + en buyuk .lz gecici kopyasi
        try {
            StatFs fs = new StatFs(mPackDir.getAbsolutePath());
            long free = fs.getAvailableBytes();
            long need = totalNeeded + maxSize + 50L * 1024 * 1024;
            if (free < need) {
                mListener.onError("Yetersiz depolama alanı.\nGerekli: " + fmtMB(need) + "  Boş: " + fmtMB(free), hasLocalPacks());
                return;
            }
        } catch (Throwable ignored) {}

        // İndirilecek .lz boyutunu belirle (küçük güncellemelerde tam, toplu indirmede yaklaşık .lz boyutu)
        long totalLzNeeded = 0;
        if (todo.size() <= 6) {
            for (Entry e : todo) {
                if (mCancel) return;
                long lz = queryRemoteLzSize(e.name);
                e.lzSize = (lz > 0) ? lz : (long) (e.size * 0.667);
                totalLzNeeded += e.lzSize;
            }
        } else {
            for (Entry e : todo) {
                e.lzSize = (long) (e.size * 0.667);
                totalLzNeeded += e.lzSize;
            }
        }

        Progress prog = new Progress();
        prog.allTotal = totalLzNeeded;
        for (int i = 0; i < todo.size(); i++) {
            if (mCancel) return;
            Entry e = todo.get(i);
            boolean ok = false;
            String lastErr = "";
            for (int attempt = 1; attempt <= MAX_RETRY_PER_FILE && !ok && !mCancel; attempt++) {
                try {
                    downloadOne(e, i + 1, todo.size(), prog);
                    ok = true;
                } catch (Exception ex) {
                    lastErr = ex.getMessage();
                    Log.e(TAG, "download " + e.name + " attempt " + attempt + " failed: " + ex);
                    mListener.onStatus("Tekrar deneniyor (" + attempt + "/" + MAX_RETRY_PER_FILE + "): " + e.name);
                    try { Thread.sleep(1500); } catch (InterruptedException ie) { return; }
                }
            }
            if (mCancel) return;
            if (!ok) {
                mListener.onError("İndirme başarısız: " + e.name + "\n" + lastErr, hasLocalPacks());
                return;
            }
            state.setProperty(e.name, Long.toHexString(e.crc));
            saveState(state);
        }

        mListener.onFinished("");
    }

    /** Ilerleme / hiz hesabi icin ortak durum. */
    private static class Progress {
        long allTotal = 0;
        long baseDone = 0;          // tamamlanmis dosyalarin indirilen bayt toplami
        long lastNs = System.nanoTime();
        long netBytesWindow = 0;
        long netSpeed = 0;
    }

    /** "crc32(hex) boyut x y dosyaadi" satirlarini okur (PC crclist ile ayni format). */
    private List<Entry> parseCrcList(String text) throws Exception {
        List<Entry> list = new ArrayList<Entry>();
        String[] lines = text.split("\r?\n");
        for (String line : lines) {
            line = line.trim();
            if (line.length() == 0 || line.startsWith("#") || line.startsWith(";")) continue;
            String[] t = line.split("\\s+");
            if (t.length < 3) continue;
            Entry e = new Entry();
            try {
                e.crc = Long.parseLong(t[0], 16) & 0xFFFFFFFFL;
                e.size = Long.parseLong(t[1]);
            } catch (NumberFormatException nfe) {
                continue;
            }
            StringBuilder nm = new StringBuilder();
            int from = (t.length >= 5) ? 4 : 2;
            for (int i = from; i < t.length; i++) {
                if (nm.length() > 0) nm.append(' ');
                nm.append(t[i]);
            }
            String name = nm.toString().replace('\\', '/');
            int slash = name.lastIndexOf('/');
            if (slash >= 0) name = name.substring(slash + 1);
            if (name.length() == 0 || name.contains("..")) continue;
            e.name = name;
            list.add(e);
        }
        return list;
    }

    private boolean isUpToDate(Entry e, Properties state) throws Exception {
        File f = new File(mPackDir, e.name);
        if (!f.exists() || f.length() != e.size) return false;
        String saved = state.getProperty(e.name);
        if (saved != null && saved.equalsIgnoreCase(Long.toHexString(e.crc))) return true;
        // Durum kaydi yok (dosya elle konmus olabilir): CRC hesaplayip dogrula
        mListener.onStatus("Doğrulanıyor: " + e.name);
        long crc = crcOfFile(f);
        if (crc == e.crc) {
            state.setProperty(e.name, Long.toHexString(e.crc));
            return true;
        }
        return false;
    }

    private void downloadOne(Entry e, int index, int count, Progress prog) throws Exception {
        File lzPart = new File(mPackDir, e.name + ".lz.part");
        File rawPart = new File(mPackDir, e.name + ".part");
        File dest = new File(mPackDir, e.name);

        String packUrl = UPDATE_BASE_URL + PACK_DIR_NAME;
        boolean isLz = true;

        // Once .lz, yoksa (404) ham dosya
        File target = lzPart;
        int rc = fetchToFile(packUrl + e.name + ".lz", lzPart, e, index, count, prog);
        if (rc == 404) {
            isLz = false;
            target = rawPart;
            rc = fetchToFile(packUrl + e.name, rawPart, e, index, count, prog);
            if (rc == 404) throw new Exception("Sunucuda yok: " + e.name + ".lz");
        }
        if (mCancel) return;

        File finalPart = rawPart;
        if (isLz) {
            mListener.onStatus("Açılıyor: " + e.name);
            if (rawPart.exists()) rawPart.delete();
            int r = NativeLib.lzDecompressFile(lzPart.getAbsolutePath(), rawPart.getAbsolutePath());
            if (r != 0) {
                lzPart.delete();
                rawPart.delete();
                throw new Exception("LZ açma hatası (" + r + ")");
            }
            lzPart.delete();
        }

        if (finalPart.length() != e.size) {
            finalPart.delete();
            throw new Exception("Boyut uyuşmuyor (" + finalPart.length() + "/" + e.size + ")");
        }

        mListener.onStatus("Doğrulanıyor: " + e.name);
        long crc = crcOfFile(finalPart);
        if (crc != e.crc) {
            finalPart.delete();
            throw new Exception("CRC uyuşmuyor");
        }

        if (dest.exists()) dest.delete();
        if (!finalPart.renameTo(dest)) throw new Exception("Dosya taşınamadı");
        prog.baseDone += e.lzSize;
    }

    /**
     * url'yi part dosyasina indirir (varsa kaldigi yerden devam eder).
     * @return HTTP kodu 404 ise 404, basariliysa 200. Diger hatalarda exception.
     */
    private int fetchToFile(String url, File part, Entry e, int index, int count, Progress prog) throws Exception {
        long have = part.exists() ? part.length() : 0;

        HttpURLConnection c = open(url, have);
        int code = c.getResponseCode();
        if (code == 404) {
            c.disconnect();
            return 404;
        }
        long contentLen = c.getContentLengthLong();
        long totalLen;
        if (code == 416) {
            // Parca zaten tam olabilir; bastan emin olmak icin sil, tekrar dene
            c.disconnect();
            part.delete();
            throw new Exception("HTTP 416");
        } else if (code == 206) {
            totalLen = (contentLen >= 0) ? have + contentLen : -1;
        } else if (code == 200) {
            have = 0; // sunucu Range'i desteklemedi: bastan
            totalLen = contentLen;
        } else {
            c.disconnect();
            throw new Exception("HTTP " + code);
        }

        if (totalLen > 0) {
            if (e.lzSize > 0 && e.lzSize != totalLen) {
                prog.allTotal += (totalLen - e.lzSize);
            }
            e.lzSize = totalLen;
        }

        RandomAccessFile raf = new RandomAccessFile(part, "rw");
        try {
            raf.setLength(have);
            raf.seek(have);
            InputStream in = new BufferedInputStream(c.getInputStream(), 65536);
            byte[] buf = new byte[65536];
            long fileDone = have;
            long lastReport = 0;
            int n;
            while ((n = in.read(buf)) != -1) {
                if (mCancel) return 200;
                raf.write(buf, 0, n);
                fileDone += n;
                prog.netBytesWindow += n;

                long now = System.nanoTime();
                if (now - lastReport >= 100000000L) {
                    lastReport = now;
                    report(e, index, count, prog, fileDone, totalLen, now);
                }
            }
            in.close();
            if (e.lzSize != fileDone) {
                prog.allTotal += (fileDone - e.lzSize);
                e.lzSize = fileDone;
            }
            report(e, index, count, prog, fileDone, totalLen, System.nanoTime());
            if (totalLen >= 0 && fileDone != totalLen) {
                throw new Exception("Dosya eksik indi (" + fileDone + "/" + totalLen + ")");
            }
        } finally {
            try { raf.close(); } catch (Exception ignored) {}
            c.disconnect();
        }
        return 200;
    }

    private void report(Entry e, int index, int count, Progress prog, long fileDone, long totalLen, long nowNs) {
        long curDone = prog.baseDone + fileDone;
        if (curDone > prog.allTotal) curDone = prog.allTotal;

        long dt = nowNs - prog.lastNs;
        if (dt >= 1000000000L) {
            prog.netSpeed = prog.netBytesWindow * 1000000000L / dt;
            prog.netBytesWindow = 0;
            prog.lastNs = nowNs;
        }
        long eta = (prog.netSpeed > 0) ? (prog.allTotal - curDone) / prog.netSpeed : -1;
        mListener.onProgress(e.name, index, count, curDone, prog.allTotal, prog.netSpeed, eta);
    }

    // ------------------------------------------------------------------------------------

    private HttpURLConnection open(String urlStr, long rangeFrom) throws Exception {
        String cur = urlStr;
        for (int i = 0; i < 6; i++) {
            HttpURLConnection c = (HttpURLConnection) new URL(cur).openConnection();
            c.setConnectTimeout(CONNECT_TIMEOUT_MS);
            c.setReadTimeout(READ_TIMEOUT_MS);
            c.setInstanceFollowRedirects(false);
            c.setRequestProperty("Accept-Encoding", "identity");
            c.setRequestProperty("User-Agent", "Aspar2Android");
            if (rangeFrom > 0) c.setRequestProperty("Range", "bytes=" + rangeFrom + "-");
            int code = c.getResponseCode();
            if (code == 301 || code == 302 || code == 303 || code == 307 || code == 308) {
                String loc = c.getHeaderField("Location");
                c.disconnect();
                if (loc == null) throw new Exception("Yönlendirme hatası");
                cur = new URL(new URL(cur), loc).toString();
                continue;
            }
            return c;
        }
        throw new Exception("Çok fazla yönlendirme");
    }

    private byte[] httpGetBytes(String url) throws Exception {
        HttpURLConnection c = open(url, 0);
        try {
            int code = c.getResponseCode();
            if (code != 200) throw new Exception("HTTP " + code);
            InputStream in = c.getInputStream();
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            byte[] buf = new byte[8192];
            int n;
            while ((n = in.read(buf)) != -1) out.write(buf, 0, n);
            in.close();
            return out.toByteArray();
        } finally {
            c.disconnect();
        }
    }

    private long crcOfFile(File f) throws Exception {
        CRC32 crc = new CRC32();
        InputStream in = new BufferedInputStream(new FileInputStream(f), 1 << 16);
        try {
            byte[] buf = new byte[1 << 16];
            int n;
            while ((n = in.read(buf)) != -1) {
                if (mCancel) break;
                crc.update(buf, 0, n);
            }
        } finally {
            in.close();
        }
        return crc.getValue();
    }

    private File stateFile() {
        return new File(mPackDir, ".update_state");
    }

    private Properties loadState() {
        Properties p = new Properties();
        File f = stateFile();
        if (f.exists()) {
            try {
                FileInputStream in = new FileInputStream(f);
                p.load(in);
                in.close();
            } catch (Exception ignored) {}
        }
        return p;
    }

    private void saveState(Properties p) {
        try {
            FileOutputStream out = new FileOutputStream(stateFile());
            p.store(out, null);
            out.close();
        } catch (Exception ignored) {}
    }

    private long queryRemoteLzSize(String name) {
        try {
            HttpURLConnection c = (HttpURLConnection) new URL(UPDATE_BASE_URL + PACK_DIR_NAME + name + ".lz").openConnection();
            c.setConnectTimeout(5000);
            c.setReadTimeout(5000);
            c.setRequestMethod("HEAD");
            c.setRequestProperty("User-Agent", "Aspar2Android");
            int code = c.getResponseCode();
            long len = c.getContentLengthLong();
            c.disconnect();
            if (code == 200 && len > 0) return len;
        } catch (Exception ignored) {}
        return -1;
    }

    public static String fmtMB(long bytes) {
        if (bytes >= 1024L * 1024 * 1024) {
            return String.format(java.util.Locale.US, "%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0));
        }
        return String.format(java.util.Locale.US, "%.1f MB", bytes / (1024.0 * 1024.0));
    }
}
