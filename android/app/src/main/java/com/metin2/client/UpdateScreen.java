package com.metin2.client;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.ClipDrawable;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.LayerDrawable;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

/** Oyun acilmadan once gosterilen guncelleme / yukleme ekrani. Tum metodlar UI thread'den cagrilmali. */
public class UpdateScreen extends FrameLayout {
    private static final int GOLD = Color.parseColor("#F2C465");
    private static final int GOLD_DARK = Color.parseColor("#9A6B1E");
    private static final int TEXT = Color.parseColor("#EFE6D8");
    private static final int SUBTEXT = Color.parseColor("#AFA595");

    private final TextView mStatus;
    private final TextView mPercent;
    private final TextView mDetail;
    private final ProgressBar mBar;
    private final LinearLayout mButtons;
    private final Button mRetry;
    private final Button mContinue;

    private int dp(int v) {
        return Math.round(v * getResources().getDisplayMetrics().density);
    }

    public UpdateScreen(Context ctx) {
        super(ctx);
        setClickable(true); // alttaki oyun view'ina dokunma gitmesin

        // 1. Arka plan resmi (loading2.jpg)
        ImageView bgView = new ImageView(ctx);
        try {
            bgView.setImageResource(R.drawable.bg_loading);
        } catch (Throwable ignored) {}
        bgView.setScaleType(ImageView.ScaleType.CENTER_CROP);
        addView(bgView, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        // 2. Hafif genel karartma / atmosfer katmani
        View dim = new View(ctx);
        dim.setBackgroundColor(Color.parseColor("#33000000"));
        addView(dim, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        // 3. Sol Üst: Aspar2 Logosu
        ImageView logoView = new ImageView(ctx);
        try {
            logoView.setImageResource(R.drawable.logo);
        } catch (Throwable ignored) {}
        logoView.setScaleType(ImageView.ScaleType.FIT_START);
        FrameLayout.LayoutParams logoLp = new FrameLayout.LayoutParams(dp(180), dp(65), Gravity.TOP | Gravity.START);
        logoLp.leftMargin = dp(24);
        logoLp.topMargin = dp(16);
        addView(logoView, logoLp);

        // 4. Sağ Üst: Sürüm etiketi
        TextView versionView = new TextView(ctx);
        versionView.setText("v1.0.4 • Android");
        versionView.setTextColor(Color.parseColor("#B0C8B8A0"));
        versionView.setTextSize(12);
        versionView.setShadowLayer(6, 0, 1, Color.parseColor("#CC000000"));
        FrameLayout.LayoutParams verLp = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.TOP | Gravity.END);
        verLp.rightMargin = dp(24);
        verLp.topMargin = dp(20);
        addView(versionView, verLp);

        // 5. Alt Kısım: Klasik MMORPG Altın Çerçeveli Panel
        LinearLayout panel = new LinearLayout(ctx);
        panel.setOrientation(LinearLayout.VERTICAL);

        GradientDrawable panelBg = new GradientDrawable();
        panelBg.setColor(Color.parseColor("#E00F0C09")); // %88 koyu obsidian taş
        panelBg.setCornerRadius(dp(12));
        panelBg.setStroke(dp(2), Color.parseColor("#A0824E")); // Altın çerçeve
        panel.setBackground(panelBg);
        panel.setPadding(dp(20), dp(14), dp(20), dp(14));

        FrameLayout.LayoutParams panelLp = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT,
                Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL);
        panelLp.leftMargin = dp(32);
        panelLp.rightMargin = dp(32);
        panelLp.bottomMargin = dp(18);
        addView(panel, panelLp);

        // Panel Üst Satırı: Durum (sol) ve Yüzde (sağ)
        LinearLayout topRow = new LinearLayout(ctx);
        topRow.setOrientation(LinearLayout.HORIZONTAL);
        topRow.setGravity(Gravity.CENTER_VERTICAL);
        panel.addView(topRow, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        mStatus = new TextView(ctx);
        mStatus.setTextColor(TEXT);
        mStatus.setTextSize(14);
        mStatus.setTypeface(Typeface.DEFAULT_BOLD);
        mStatus.setText("Hazırlanıyor...");
        mStatus.setShadowLayer(6, 0, 1, Color.parseColor("#DD000000"));
        topRow.addView(mStatus, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1.0f));

        mPercent = new TextView(ctx);
        mPercent.setTextColor(GOLD);
        mPercent.setTextSize(16);
        mPercent.setTypeface(Typeface.DEFAULT_BOLD);
        mPercent.setText("%0");
        mPercent.setShadowLayer(8, 0, 2, Color.parseColor("#EE000000"));
        topRow.addView(mPercent, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        // İlerleme çubuğu (Altın dolgulu, koyu kanallı)
        GradientDrawable track = new GradientDrawable();
        track.setColor(Color.parseColor("#1F1912"));
        track.setCornerRadius(dp(7));
        track.setStroke(dp(1), Color.parseColor("#5A462A"));
        GradientDrawable fill = new GradientDrawable(GradientDrawable.Orientation.LEFT_RIGHT,
                new int[] { GOLD_DARK, GOLD });
        fill.setCornerRadius(dp(7));
        ClipDrawable clip = new ClipDrawable(fill, Gravity.LEFT, ClipDrawable.HORIZONTAL);
        LayerDrawable layers = new LayerDrawable(new android.graphics.drawable.Drawable[] { track, clip });
        layers.setId(0, android.R.id.background);
        layers.setId(1, android.R.id.progress);

        mBar = new ProgressBar(ctx, null, android.R.attr.progressBarStyleHorizontal);
        mBar.setIndeterminate(false);
        mBar.setMax(1000);
        mBar.setProgress(0);
        mBar.setProgressDrawable(layers);
        LinearLayout.LayoutParams barLp = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, dp(14));
        barLp.topMargin = dp(8);
        panel.addView(mBar, barLp);

        // Panel Alt Satırı: Hız ve Boyut Detayı
        mDetail = new TextView(ctx);
        mDetail.setTextColor(SUBTEXT);
        mDetail.setTextSize(12);
        mDetail.setShadowLayer(5, 0, 1, Color.parseColor("#CC000000"));
        mDetail.setText("");
        LinearLayout.LayoutParams dtLp = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        dtLp.topMargin = dp(6);
        panel.addView(mDetail, dtLp);

        // Hata / Yeniden Dene Butonları
        mButtons = new LinearLayout(ctx);
        mButtons.setOrientation(LinearLayout.HORIZONTAL);
        mButtons.setGravity(Gravity.CENTER);
        mButtons.setVisibility(View.GONE);
        LinearLayout.LayoutParams btnRowLp = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        btnRowLp.topMargin = dp(10);
        panel.addView(mButtons, btnRowLp);

        mRetry = makeButton(ctx, "Tekrar Dene");
        mContinue = makeButton(ctx, "Oyuna Gir");
        LinearLayout.LayoutParams b1 = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        b1.rightMargin = dp(12);
        mButtons.addView(mRetry, b1);
        mButtons.addView(mContinue, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
    }

    private Button makeButton(Context ctx, String text) {
        Button b = new Button(ctx);
        b.setText(text);
        b.setAllCaps(false);
        b.setTextColor(Color.parseColor("#1A1208"));
        b.setTextSize(14);
        b.setTypeface(Typeface.DEFAULT_BOLD);
        GradientDrawable d = new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM,
                new int[] { GOLD, GOLD_DARK });
        d.setCornerRadius(dp(8));
        b.setBackground(d);
        b.setPadding(dp(20), dp(6), dp(20), dp(6));
        return b;
    }

    public void setStatus(String text) {
        mStatus.setTextColor(TEXT);
        mStatus.setText(text);
        mButtons.setVisibility(View.GONE);
    }

    public void setProgress(String fileName, int fileIndex, int fileCount,
                            long allDone, long allTotal, long netSpeedBps, long etaSec) {
        mStatus.setTextColor(TEXT);
        mStatus.setText("İndiriliyor: " + fileName + "  (" + fileIndex + "/" + fileCount + ")");
        double frac = (allTotal > 0) ? Math.min(1.0, (double) allDone / (double) allTotal) : 0.0;
        mBar.setProgress((int) Math.round(frac * 1000));
        mPercent.setText("%" + (int) Math.floor(frac * 100));

        StringBuilder sb = new StringBuilder();
        sb.append(PackUpdater.fmtMB(allDone)).append(" / ").append(PackUpdater.fmtMB(allTotal));
        if (netSpeedBps > 0) {
            sb.append("   •   Hız: ").append(String.format(java.util.Locale.US, "%.1f MB/s", netSpeedBps / (1024.0 * 1024.0)));
        }
        if (etaSec >= 0) {
            sb.append("   •   Kalan: ").append(String.format(java.util.Locale.US, "%02d:%02d", etaSec / 60, etaSec % 60));
        }
        mDetail.setText(sb.toString());
        mButtons.setVisibility(View.GONE);
    }

    public void setDone() {
        mBar.setProgress(1000);
        mPercent.setText("%100");
        mStatus.setText("Oyun başlatılıyor...");
        mDetail.setText("");
        mButtons.setVisibility(View.GONE);
    }

    public void showError(String message, boolean canContinue, OnClickListener onRetry, OnClickListener onContinue) {
        mStatus.setTextColor(Color.parseColor("#FF7A6A"));
        mStatus.setText(message);
        mRetry.setOnClickListener(onRetry);
        mContinue.setOnClickListener(onContinue);
        mContinue.setVisibility(canContinue ? View.VISIBLE : View.GONE);
        mButtons.setVisibility(View.VISIBLE);
    }
}
