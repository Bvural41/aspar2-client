package com.metin2.client;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.net.Uri;
import android.view.Gravity;
import android.webkit.CookieManager;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity {
    private static final String TAG = "Metin2Mobile";
    public static MainActivity sInstance;

    private GLSurfaceView mGLView;
    private EditText mHiddenEditText;
    private boolean mIgnoreTextChange = false;
    private int mTargetWidth = 1067;
    private int mTargetHeight = 600;
    private int mJoystickPointerId = MotionEvent.INVALID_POINTER_ID;
    private int mActionPointerId = MotionEvent.INVALID_POINTER_ID;
    private float mActionStartX = 0.0f;
    private float mActionStartY = 0.0f;
    private float mActionLastX = 0.0f;
    private float mActionLastY = 0.0f;
    private boolean mActionDidDrag = false;
    private int mSecondaryPointerId = MotionEvent.INVALID_POINTER_ID;
    private float mSecondaryLastX = 0.0f;
    private float mSecondaryLastY = 0.0f;

    // Pack guncelleme (oyun acilmadan once)
    private UpdateScreen mUpdateScreen;
    private PackUpdater mUpdater;
    private volatile boolean mUpdateDone = false;
    private FrameLayout mRootLayout;
    private FrameLayout mWebLayout;
    private WebView mWebView;
    private volatile boolean mIsWebShowing = false;

    private static float sJoystickX = 0.0f;
    private static float sJoystickY = -1.0f;
    private static float sJoystickW = 194.0f;
    private static float sJoystickH = 194.0f;

    public static void setJoystickZone(float x, float y, float width, float height) {
        sJoystickX = x;
        sJoystickY = y;
        sJoystickW = width;
        sJoystickH = height;
    }

    private boolean isJoystickZone(float x, float y) {
        try {
            if (!NativeLib.isGamePhase()) {
                return false;
            }
        } catch (Throwable ignored) {
            return false;
        }
        float jx = sJoystickX;
        float jy = (sJoystickY >= 0.0f) ? sJoystickY : (mTargetHeight - sJoystickH);
        float jw = sJoystickW;
        float jh = sJoystickH;
        return (x >= jx && x <= jx + jw && y >= jy && y <= jy + jh);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        sInstance = this;

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.P) {
            WindowManager.LayoutParams lp = getWindow().getAttributes();
            lp.layoutInDisplayCutoutMode =
                WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
            getWindow().setAttributes(lp);
        }

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            try {
                android.view.Display display = getWindowManager().getDefaultDisplay();
                android.view.Display.Mode[] modes = display.getSupportedModes();
                android.view.Display.Mode targetMode = null;
                for (android.view.Display.Mode mode : modes) {
                    if (Math.abs(mode.getRefreshRate() - 60.0f) < 1.0f) {
                        targetMode = mode;
                        break;
                    }
                }
                if (targetMode != null) {
                    WindowManager.LayoutParams lp = getWindow().getAttributes();
                    lp.preferredDisplayModeId = targetMode.getModeId();
                    getWindow().setAttributes(lp);
                    Log.i(TAG, "Requested 60Hz display mode: " + targetMode);
                }
            } catch (Exception e) {
                Log.e(TAG, "Failed to set 60Hz display mode: " + e.getMessage());
            }
        }

        // Calculate aspect-ratio aware target resolution based on 720p base height
        int screenW = 1280;
        int screenH = 720;
        try {
            android.view.Display display = getWindowManager().getDefaultDisplay();
            android.graphics.Point realSize = new android.graphics.Point();
            display.getRealSize(realSize);
            screenW = Math.max(realSize.x, realSize.y);
            screenH = Math.min(realSize.x, realSize.y);
        } catch (Exception e) {
            android.util.DisplayMetrics dm = getResources().getDisplayMetrics();
            screenW = Math.max(dm.widthPixels, dm.heightPixels);
            screenH = Math.min(dm.widthPixels, dm.heightPixels);
        }

        mTargetHeight = 600;
        if (screenH > 0) {
            float aspect = (float) screenW / (float) screenH;
            mTargetWidth = Math.round(mTargetHeight * aspect);
            if (mTargetWidth % 2 != 0) {
                mTargetWidth++; // ensure even number
            }
        } else {
            mTargetWidth = 1067;
        }
        Log.i(TAG, "Screen physical: " + screenW + "x" + screenH + " -> Target render resolution: " + mTargetWidth + "x" + mTargetHeight);

        mGLView = new GLSurfaceView(this);
        mGLView.setPreserveEGLContextOnPause(true);
        mGLView.setEGLContextClientVersion(3);
        mGLView.getHolder().setFixedSize(mTargetWidth, mTargetHeight);
        mGLView.setRenderer(new Renderer());
        mGLView.setFocusable(true);
        mGLView.setFocusableInTouchMode(true);
        mGLView.requestFocus();
        mGLView.setKeepScreenOn(true);

        mGLView.setOnTouchListener((v, event) -> {
            final int action = event.getActionMasked();
            final float viewW = v.getWidth();
            final float viewH = v.getHeight();
            final float scaleX = (viewW > 0) ? ((float) mTargetWidth / viewW) : 1.0f;
            final float scaleY = (viewH > 0) ? ((float) mTargetHeight / viewH) : 1.0f;

            switch (action) {
                case MotionEvent.ACTION_DOWN: {
                    int pointerId = event.getPointerId(0);
                    final float x = event.getX(0) * scaleX;
                    final float y = event.getY(0) * scaleY;

                    if (isJoystickZone(x, y)) {
                        mJoystickPointerId = pointerId;
                        mGLView.queueEvent(() -> NativeLib.joystickTouch(MotionEvent.ACTION_DOWN, x, y));
                    } else {
                        mActionPointerId = pointerId;
                        mActionStartX = x;
                        mActionStartY = y;
                        mActionLastX = x;
                        mActionLastY = y;
                        mActionDidDrag = false;
                        mGLView.queueEvent(() -> NativeLib.actionTouch(MotionEvent.ACTION_DOWN, x, y, 0.0f, 0.0f, false));
                    }
                    break;
                }
                case MotionEvent.ACTION_POINTER_DOWN: {
                    int pointerIndex = event.getActionIndex();
                    int pointerId = event.getPointerId(pointerIndex);
                    final float x = event.getX(pointerIndex) * scaleX;
                    final float y = event.getY(pointerIndex) * scaleY;

                    if (isJoystickZone(x, y) && mJoystickPointerId == MotionEvent.INVALID_POINTER_ID) {
                        mJoystickPointerId = pointerId;
                        mGLView.queueEvent(() -> NativeLib.joystickTouch(MotionEvent.ACTION_DOWN, x, y));
                    } else if (mActionPointerId == MotionEvent.INVALID_POINTER_ID) {
                        mActionPointerId = pointerId;
                        mActionStartX = x;
                        mActionStartY = y;
                        mActionLastX = x;
                        mActionLastY = y;
                        mActionDidDrag = false;
                         mGLView.queueEvent(() -> NativeLib.actionTouch(MotionEvent.ACTION_DOWN, x, y, 0.0f, 0.0f, false));
                    } else if (mSecondaryPointerId == MotionEvent.INVALID_POINTER_ID && !isJoystickZone(x, y)) {
                        mSecondaryPointerId = pointerId;
                        mSecondaryLastX = x;
                        mSecondaryLastY = y;
                        mGLView.queueEvent(() -> NativeLib.secondaryTouch(MotionEvent.ACTION_DOWN, x, y, 0.0f, 0.0f));
                    }
                    break;
                }
                case MotionEvent.ACTION_MOVE: {
                    // 1. Joystick movement
                    if (mJoystickPointerId != MotionEvent.INVALID_POINTER_ID) {
                        int jIdx = event.findPointerIndex(mJoystickPointerId);
                        if (jIdx >= 0) {
                            final float jx = event.getX(jIdx) * scaleX;
                            final float jy = event.getY(jIdx) * scaleY;
                            mGLView.queueEvent(() -> NativeLib.joystickTouch(MotionEvent.ACTION_MOVE, jx, jy));
                        }
                    }

                    // 2. Action / HUD / Camera movement
                    if (mActionPointerId != MotionEvent.INVALID_POINTER_ID) {
                        int aIdx = event.findPointerIndex(mActionPointerId);
                        if (aIdx >= 0) {
                            final float ax = event.getX(aIdx) * scaleX;
                            final float ay = event.getY(aIdx) * scaleY;
                            final float dx = ax - mActionLastX;
                            final float dy = ay - mActionLastY;
                            mActionLastX = ax;
                            mActionLastY = ay;

                            float distSq = (ax - mActionStartX) * (ax - mActionStartX) + (ay - mActionStartY) * (ay - mActionStartY);
                            if (distSq > 100.0f) { // > 10 pixels total movement
                                mActionDidDrag = true;
                            }
                            final boolean isDrag = mActionDidDrag;
                            mGLView.queueEvent(() -> NativeLib.actionTouch(MotionEvent.ACTION_MOVE, ax, ay, dx, dy, isDrag));
                        }
                    }

                    // 3. Secondary finger (e.g. camera drag while attack button + joystick are held)
                    if (mSecondaryPointerId != MotionEvent.INVALID_POINTER_ID) {
                        int sIdx = event.findPointerIndex(mSecondaryPointerId);
                        if (sIdx >= 0) {
                            final float sx = event.getX(sIdx) * scaleX;
                            final float sy = event.getY(sIdx) * scaleY;
                            final float sdx = sx - mSecondaryLastX;
                            final float sdy = sy - mSecondaryLastY;
                            mSecondaryLastX = sx;
                            mSecondaryLastY = sy;
                            mGLView.queueEvent(() -> NativeLib.secondaryTouch(MotionEvent.ACTION_MOVE, sx, sy, sdx, sdy));
                        }
                    }
                    break;
                }
                case MotionEvent.ACTION_POINTER_UP: {
                    int pointerIndex = event.getActionIndex();
                    int pointerId = event.getPointerId(pointerIndex);
                    final float x = event.getX(pointerIndex) * scaleX;
                    final float y = event.getY(pointerIndex) * scaleY;

                    if (pointerId == mJoystickPointerId) {
                        mJoystickPointerId = MotionEvent.INVALID_POINTER_ID;
                        mGLView.queueEvent(() -> NativeLib.joystickTouch(MotionEvent.ACTION_UP, x, y));
                    } else if (pointerId == mActionPointerId) {
                        final boolean isDrag = mActionDidDrag;
                        mActionPointerId = MotionEvent.INVALID_POINTER_ID;
                        mGLView.queueEvent(() -> NativeLib.actionTouch(MotionEvent.ACTION_UP, x, y, 0.0f, 0.0f, isDrag));
                    } else if (pointerId == mSecondaryPointerId) {
                        mSecondaryPointerId = MotionEvent.INVALID_POINTER_ID;
                        mGLView.queueEvent(() -> NativeLib.secondaryTouch(MotionEvent.ACTION_UP, x, y, 0.0f, 0.0f));
                    }
                    break;
                }
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL: {
                    int pointerIndex = (event.getPointerCount() > 0) ? event.getActionIndex() : 0;
                    final float x = (pointerIndex >= 0 && pointerIndex < event.getPointerCount() ? event.getX(pointerIndex) : 0) * scaleX;
                    final float y = (pointerIndex >= 0 && pointerIndex < event.getPointerCount() ? event.getY(pointerIndex) : 0) * scaleY;

                    if (mJoystickPointerId != MotionEvent.INVALID_POINTER_ID) {
                        mJoystickPointerId = MotionEvent.INVALID_POINTER_ID;
                        mGLView.queueEvent(() -> NativeLib.joystickTouch(MotionEvent.ACTION_UP, x, y));
                    }
                    if (mActionPointerId != MotionEvent.INVALID_POINTER_ID) {
                        final boolean isDrag = mActionDidDrag;
                        mActionPointerId = MotionEvent.INVALID_POINTER_ID;
                        mGLView.queueEvent(() -> NativeLib.actionTouch(MotionEvent.ACTION_UP, x, y, 0.0f, 0.0f, isDrag));
                    }
                    if (mSecondaryPointerId != MotionEvent.INVALID_POINTER_ID) {
                        mSecondaryPointerId = MotionEvent.INVALID_POINTER_ID;
                        mGLView.queueEvent(() -> NativeLib.secondaryTouch(MotionEvent.ACTION_UP, x, y, 0.0f, 0.0f));
                    }
                    break;
                }
            }
            return true;
        });

        mGLView.setOnGenericMotionListener((v, event) -> {
            if ((event.getSource() & android.view.InputDevice.SOURCE_CLASS_POINTER) != 0) {
                if (event.getAction() == MotionEvent.ACTION_HOVER_MOVE) {
                    final float viewW = v.getWidth();
                    final float viewH = v.getHeight();
                    final float scaleX = (viewW > 0) ? ((float) mTargetWidth / viewW) : 1.0f;
                    final float scaleY = (viewH > 0) ? ((float) mTargetHeight / viewH) : 1.0f;
                    final float x = event.getX() * scaleX;
                    final float y = event.getY() * scaleY;
                    mGLView.queueEvent(() -> NativeLib.touchEvent(MotionEvent.ACTION_MOVE, x, y));
                    return true;
                }
            }
            return false;
        });

        // Setup hidden EditText for soft keyboard support
        FrameLayout rootLayout = new FrameLayout(this);
        rootLayout.addView(mGLView, new FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        mHiddenEditText = new EditText(this);
        mHiddenEditText.setFocusable(true);
        mHiddenEditText.setFocusableInTouchMode(true);
        mHiddenEditText.setImeOptions(EditorInfo.IME_FLAG_NO_EXTRACT_UI | EditorInfo.IME_ACTION_DONE);
        mHiddenEditText.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS);
        mHiddenEditText.setAlpha(0.001f); // nearly invisible, but interactable
        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(10, 10);
        rootLayout.addView(mHiddenEditText, lp);

        mHiddenEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
            @Override
            public void onTextChanged(CharSequence s, int start, int count, int after) {}
            @Override
            public void afterTextChanged(Editable s) {
                if (mIgnoreTextChange) return;
                final String text = s.toString();
                Log.i(TAG, "Keyboard text changed: " + text);
                if (mGLView != null) {
                    mGLView.queueEvent(() -> NativeLib.onKeyboardText(text));
                }
            }
        });

        mHiddenEditText.setOnEditorActionListener((v, actionId, event) -> {
            if (actionId == EditorInfo.IME_ACTION_DONE || actionId == EditorInfo.IME_ACTION_SEND
                || (event != null && event.getKeyCode() == KeyEvent.KEYCODE_ENTER && event.getAction() == KeyEvent.ACTION_DOWN)) {
                Log.i(TAG, "Keyboard enter pressed");
                if (mGLView != null) {
                    mGLView.queueEvent(() -> NativeLib.onKeyboardEnter());
                }
                hideKeyboard();
                return true;
            }
            return false;
        });

        setContentView(rootLayout);

        hideSystemUI();

        requestStoragePermission();

        mRootLayout = rootLayout;
        startPackUpdate();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mUpdater != null) {
            mUpdater.cancel();
        }
        if (mWebView != null) {
            try {
                mWebView.destroy();
            } catch (Exception ignored) {}
            mWebView = null;
        }
        try {
            NativeLib.saveConfig();
        } catch (Throwable t) {
            Log.e(TAG, "saveConfig error in onDestroy: " + t.getMessage());
        }
        if (sInstance == this) {
            sInstance = null;
        }
    }

    public void exitApp() {
        Log.i(TAG, "exitApp requested");
        try {
            NativeLib.saveConfig();
        } catch (Throwable t) {
            Log.e(TAG, "saveConfig error in exitApp: " + t.getMessage());
        }
        runOnUiThread(() -> {
            try {
                if (mGLView != null) {
                    mGLView.onPause();
                }
                finishAffinity();
                new android.os.Handler().postDelayed(() -> {
                    System.exit(0);
                }, 100);
            } catch (Exception e) {
                Log.e(TAG, "exitApp error", e);
                System.exit(0);
            }
        });
    }

    public void showKeyboard(final String initialText) {
        Log.i(TAG, "showKeyboard requested, initialText=" + initialText);
        runOnUiThread(() -> {
            if (mHiddenEditText != null) {
                mIgnoreTextChange = true;
                mHiddenEditText.setText(initialText != null ? initialText : "");
                mHiddenEditText.setSelection(mHiddenEditText.getText().length());
                mIgnoreTextChange = false;

                mHiddenEditText.setFocusable(true);
                mHiddenEditText.setFocusableInTouchMode(true);
                mHiddenEditText.requestFocus();

                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                if (imm != null) {
                    boolean shown = imm.showSoftInput(mHiddenEditText, InputMethodManager.SHOW_FORCED);
                    Log.i(TAG, "showSoftInput result: " + shown);
                    if (!shown) {
                        imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, InputMethodManager.HIDE_IMPLICIT_ONLY);
                        Log.i(TAG, "toggleSoftInput invoked as fallback");
                    }
                }
            }
        });
    }

    public void hideKeyboard() {
        Log.i(TAG, "hideKeyboard requested");
        runOnUiThread(() -> {
            if (mHiddenEditText != null) {
                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                if (imm != null) {
                    imm.hideSoftInputFromWindow(mHiddenEditText.getWindowToken(), 0);
                }
                if (mGLView != null) {
                    mGLView.requestFocus();
                }
                hideSystemUI();
            }
        });
    }

    public void showWebPage(final String url) {
        Log.i(TAG, "showWebPage requested, url=" + url);
        runOnUiThread(() -> {
            if (mRootLayout == null) {
                Log.e(TAG, "mRootLayout is null, cannot show web page!");
                return;
            }

            if (mWebLayout != null) {
                try {
                    if (mWebView != null) {
                        mWebView.stopLoading();
                        mWebView.destroy();
                        mWebView = null;
                    }
                    mRootLayout.removeView(mWebLayout);
                } catch (Exception ignored) {}
                mWebLayout = null;
            }

            mIsWebShowing = true;

            mWebLayout = new FrameLayout(this);
            mWebLayout.setBackgroundColor(Color.argb(250, 10, 10, 15));

            mWebView = new WebView(this);
            mWebView.setBackgroundColor(Color.argb(255, 18, 18, 24));

            WebSettings ws = mWebView.getSettings();
            ws.setJavaScriptEnabled(true);
            ws.setDomStorageEnabled(true);
            ws.setDatabaseEnabled(true);
            ws.setAllowFileAccess(true);
            ws.setSupportMultipleWindows(false);
            ws.setJavaScriptCanOpenWindowsAutomatically(true);
            ws.setUseWideViewPort(true);
            ws.setLoadWithOverviewMode(true);
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
                ws.setMixedContentMode(WebSettings.MIXED_CONTENT_ALWAYS_ALLOW);
                CookieManager.getInstance().setAcceptThirdPartyCookies(mWebView, true);
            }
            CookieManager.getInstance().setAcceptCookie(true);

            final ProgressBar progressBar = new ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal);
            progressBar.setIndeterminate(true);
            FrameLayout.LayoutParams pblp = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                dpToPx(4),
                Gravity.TOP
            );
            progressBar.setLayoutParams(pblp);

            mWebView.setWebViewClient(new WebViewClient() {
                @Override
                public boolean shouldOverrideUrlLoading(WebView view, String reqUrl) {
                    if (reqUrl == null) return false;
                    if (reqUrl.startsWith("http://") || reqUrl.startsWith("https://")) {
                        return false;
                    }
                    try {
                        Intent intent = Intent.parseUri(reqUrl, Intent.URI_INTENT_SCHEME);
                        if (intent != null) {
                            startActivity(intent);
                            return true;
                        }
                    } catch (Exception e) {
                        Log.e(TAG, "WebView intent error: " + e.getMessage());
                    }
                    return true;
                }

                @Override
                public void onPageFinished(WebView view, String finishUrl) {
                    super.onPageFinished(view, finishUrl);
                    if (progressBar != null) {
                        progressBar.setVisibility(View.GONE);
                    }
                }
            });

            mWebView.setWebChromeClient(new WebChromeClient());

            mWebLayout.addView(mWebView, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            ));

            mWebLayout.addView(progressBar);

            TextView closeBtn = new TextView(this);
            closeBtn.setText("✕ KAPAT");
            closeBtn.setTextColor(Color.WHITE);
            closeBtn.setTextSize(13.0f);
            closeBtn.setTypeface(null, Typeface.BOLD);
            closeBtn.setGravity(Gravity.CENTER);
            closeBtn.setPadding(dpToPx(16), dpToPx(8), dpToPx(16), dpToPx(8));

            GradientDrawable btnBg = new GradientDrawable();
            btnBg.setShape(GradientDrawable.RECTANGLE);
            btnBg.setColor(Color.parseColor("#C62828"));
            btnBg.setCornerRadius(dpToPx(18));
            btnBg.setStroke(dpToPx(1), Color.parseColor("#EF5350"));
            closeBtn.setBackground(btnBg);

            FrameLayout.LayoutParams btnParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                dpToPx(36),
                Gravity.TOP | Gravity.END
            );
            btnParams.topMargin = dpToPx(12);
            btnParams.rightMargin = dpToPx(16);
            btnParams.setMarginEnd(dpToPx(16));
            closeBtn.setLayoutParams(btnParams);

            closeBtn.setOnClickListener(v -> hideWebPage());
            mWebLayout.addView(closeBtn);

            mRootLayout.addView(mWebLayout, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            ));

            mWebView.loadUrl(url);
            mWebView.requestFocus();
        });
    }

    public void hideWebPage() {
        Log.i(TAG, "hideWebPage requested");
        runOnUiThread(() -> {
            mIsWebShowing = false;
            if (mWebLayout != null && mRootLayout != null) {
                if (mWebView != null) {
                    mWebView.stopLoading();
                    mWebView.loadUrl("about:blank");
                    mWebLayout.removeView(mWebView);
                    mWebView.destroy();
                    mWebView = null;
                }
                mRootLayout.removeView(mWebLayout);
                mWebLayout = null;
            }
            hideSystemUI();
            if (mGLView != null) {
                mGLView.requestFocus();
            }
        });
    }

    public boolean isWebShowing() {
        return mIsWebShowing;
    }

    private int dpToPx(int dp) {
        return Math.round(dp * getResources().getDisplayMetrics().density);
    }

    @Override
    public void onBackPressed() {
        if (mIsWebShowing && mWebView != null) {
            if (mWebView.canGoBack()) {
                mWebView.goBack();
                return;
            } else {
                hideWebPage();
                return;
            }
        }
        super.onBackPressed();
    }

    private void requestStoragePermission() {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
            if (!android.os.Environment.isExternalStorageManager()) {
                try {
                    android.content.Intent intent = new android.content.Intent(
                        android.provider.Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION,
                        android.net.Uri.parse("package:" + getPackageName())
                    );
                    startActivity(intent);
                } catch (Exception e) {
                    try {
                        android.content.Intent intent = new android.content.Intent(
                            android.provider.Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION
                        );
                        startActivity(intent);
                    } catch (Exception ignored) {}
                }
            }
        } else if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            if (checkSelfPermission(android.Manifest.permission.READ_EXTERNAL_STORAGE) != android.content.pm.PackageManager.PERMISSION_GRANTED) {
                requestPermissions(new String[]{
                    android.Manifest.permission.READ_EXTERNAL_STORAGE,
                    android.Manifest.permission.WRITE_EXTERNAL_STORAGE
                }, 1001);
            }
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            hideSystemUI();
        }
    }

    private void hideSystemUI() {
        try {
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
                getWindow().setDecorFitsSystemWindows(false);
                if (getWindow() != null && getWindow().getDecorView() != null) {
                    final android.view.WindowInsetsController controller = getWindow().getInsetsController();
                    if (controller != null) {
                        controller.hide(android.view.WindowInsets.Type.statusBars() | android.view.WindowInsets.Type.navigationBars());
                        controller.setSystemBarsBehavior(android.view.WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
                    }
                }
            }
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.P) {
                WindowManager.LayoutParams lp = getWindow().getAttributes();
                lp.layoutInDisplayCutoutMode =
                    WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
                getWindow().setAttributes(lp);
            }
            if (getWindow() != null && getWindow().getDecorView() != null) {
                getWindow().getDecorView().setSystemUiVisibility(
                    android.view.View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                    | android.view.View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | android.view.View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | android.view.View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | android.view.View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | android.view.View.SYSTEM_UI_FLAG_FULLSCREEN
                );
            }
        } catch (Exception e) {
            Log.w(TAG, "hideSystemUI error: " + e.getMessage());
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        hideSystemUI();
        if (mWebView != null) {
            try {
                mWebView.onResume();
            } catch (Exception ignored) {}
        }
        try {
            NativeLib.resumeAudio();
        } catch (Throwable t) {
            Log.e(TAG, "resumeAudio error: " + t.getMessage());
        }
        if (mGLView != null) {
            mGLView.onResume();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mWebView != null) {
            try {
                mWebView.onPause();
            } catch (Exception ignored) {}
        }
        try {
            NativeLib.saveConfig();
        } catch (Throwable t) {
            Log.e(TAG, "saveConfig error in onPause: " + t.getMessage());
        }
        try {
            NativeLib.pauseAudio();
        } catch (Throwable t) {
            Log.e(TAG, "pauseAudio error: " + t.getMessage());
        }
        if (mGLView != null) {
            mGLView.onPause();
        }
    }

    private void copyAssets() {
        File filesDir = getFilesDir();
        File extDir = getExternalFilesDir(null);
        File extPack = (extDir != null) ? new File(extDir, "pack/root.index") : null;
        boolean hasExternalPack = (extPack != null && extPack.exists());

        long apkTime = new File(getPackageCodePath()).lastModified();
        File timeFile = new File(filesDir, ".apk_timestamp");
        if (timeFile.exists() && timeFile.lastModified() >= apkTime) {
            Log.i(TAG, "Assets are up to date.");
            return;
        }

        Log.i(TAG, "Extracting/Updating assets to internal storage... (hasExternalPack=" + hasExternalPack + ")");
        if (!hasExternalPack) {
            copyAssetFolder("pack", new File(filesDir, "pack"));
        } else {
            Log.i(TAG, "External pack found! Skipping internal pack extraction.");
        }
        copyAssetFolder("lib", new File(filesDir, "lib"));
        copyAssetFolder("mark", new File(filesDir, "mark"));
        copyAssetFolder("BGM", new File(filesDir, "BGM"));
        try {
            if (!timeFile.exists()) timeFile.createNewFile();
            timeFile.setLastModified(apkTime);
        } catch (Exception ignored) {}
        Log.i(TAG, "Asset extraction completed.");
    }

    private void copySingleAsset(String srcName, File dstFile) {
        try {
            if (!dstFile.getParentFile().exists()) dstFile.getParentFile().mkdirs();
            try (InputStream in = getAssets().open(srcName);
                 OutputStream out = new FileOutputStream(dstFile)) {
                byte[] buffer = new byte[65536];
                int read;
                while ((read = in.read(buffer)) != -1) {
                    out.write(buffer, 0, read);
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Error copying " + srcName, e);
        }
    }

    private void copyAssetFolder(String srcName, File dstDir) {
        try {
            String[] fileList = getAssets().list(srcName);
            if (fileList == null || fileList.length == 0) return;
            if (!dstDir.exists()) dstDir.mkdirs();
            for (String filename : fileList) {
                String fullSrcPath = srcName.isEmpty() ? filename : (srcName + "/" + filename);
                String[] subList = getAssets().list(fullSrcPath);
                if (subList != null && subList.length > 0) {
                    copyAssetFolder(fullSrcPath, new File(dstDir, filename));
                } else {
                    File dstFile = new File(dstDir, filename);
                    Log.i(TAG, "Extracting asset: " + fullSrcPath + " -> " + dstFile.getAbsolutePath());
                    try (InputStream in = getAssets().open(fullSrcPath);
                         OutputStream out = new FileOutputStream(dstFile)) {
                        byte[] buffer = new byte[65536];
                        int read;
                        while ((read = in.read(buffer)) != -1) {
                            out.write(buffer, 0, read);
                        }
                    }
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Error extracting assets: " + srcName, e);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        return super.onTouchEvent(event);
    }

    /** Elle konmus (gelistirici) pack klasorleri varsa native bunlari once kullanir; o zaman indirme atlanir. */
    private boolean hasDevPackOverride() {
        String[] roots = {
            "/sdcard/aspar2/pack/root.index",
            "/storage/emulated/0/aspar2/pack/root.index",
            "/sdcard/metin2/pack/root.index",
            "/storage/emulated/0/metin2/pack/root.index"
        };
        for (String p : roots) {
            if (new File(p).canRead()) return true;
        }
        return false;
    }

    private void startPackUpdate() {
        File extDir = getExternalFilesDir(null);
        if (extDir == null || hasDevPackOverride()) {
            Log.i(TAG, "Pack update skipped (extDir=" + extDir + ", devOverride=" + hasDevPackOverride() + ")");
            mUpdateDone = true;
            return;
        }
        mUpdateScreen = new UpdateScreen(this);
        mRootLayout.addView(mUpdateScreen, new FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        runPackUpdater(new File(extDir, "pack"));
    }

    private void runPackUpdater(final File packDir) {
        mUpdater = new PackUpdater(packDir, new PackUpdater.Listener() {
            @Override
            public void onStatus(final String text) {
                runOnUiThread(() -> { if (mUpdateScreen != null) mUpdateScreen.setStatus(text); });
            }

            @Override
            public void onProgress(final String fileName, final int fileIndex, final int fileCount,
                                   final long allDone, final long allTotal,
                                   final long netSpeedBps, final long etaSec) {
                runOnUiThread(() -> {
                    if (mUpdateScreen != null) {
                        mUpdateScreen.setProgress(fileName, fileIndex, fileCount, allDone, allTotal, netSpeedBps, etaSec);
                    }
                });
            }

            @Override
            public void onFinished(String version) {
                Log.i(TAG, "Pack update finished, version=" + version);
                runOnUiThread(() -> { if (mUpdateScreen != null) mUpdateScreen.setDone(); });
                mUpdateDone = true; // GL thread bir sonraki karede oyunu baslatir, ekran init bitince kalkar
            }

            @Override
            public void onError(final String message, final boolean canContinue) {
                runOnUiThread(() -> {
                    if (mUpdateScreen == null) return;
                    mUpdateScreen.showError(message, canContinue,
                        v -> {
                            mUpdateScreen.setStatus("Tekrar deneniyor...");
                            runPackUpdater(packDir);
                        },
                        v -> {
                            mUpdateScreen.setDone();
                            mUpdateDone = true;
                        });
                });
            }
        });
        mUpdater.start();
    }

    private void hideUpdateScreen() {
        runOnUiThread(() -> {
            if (mUpdateScreen != null && mRootLayout != null) {
                mRootLayout.removeView(mUpdateScreen);
                mUpdateScreen = null;
            }
        });
    }

    private class Renderer implements GLSurfaceView.Renderer {
        private boolean mInitialized = false;
        private int mSurfaceW = 0;
        private int mSurfaceH = 0;
        private long mLastFrameTimeNs = 0;
        private static final long TARGET_FRAME_DURATION_NS = 1_000_000_000L / 60L; // 60 FPS (~16.666 ms)

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
                try {
                    mGLView.getHolder().getSurface().setFrameRate(60.0f, android.view.Surface.FRAME_RATE_COMPATIBILITY_DEFAULT);
                } catch (Exception ignored) {}
            }

            mSurfaceW = width;
            mSurfaceH = height;
            tryInit();
        }

        // Pack guncellemesi bitmeden native motor baslatilmaz.
        private void tryInit() {
            if (mInitialized || !mUpdateDone || mSurfaceW <= 0) return;
            copyAssets();
            File extDir = getExternalFilesDir(null);
            String extPath = (extDir != null) ? extDir.getAbsolutePath() : "";
            NativeLib.init(MainActivity.this.getAssets(), getFilesDir().getAbsolutePath(), extPath, mSurfaceW, mSurfaceH);
            mInitialized = true;
            hideUpdateScreen();
        }

        public void onDrawFrame(GL10 gl) {
            if (!mInitialized) {
                tryInit();
                if (!mInitialized) return;
            }

            NativeLib.render();
        }
    }
}
