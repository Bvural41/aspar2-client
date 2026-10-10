package com.metin2.client;

import android.util.Log;

public class NativeLib {
    static {
        System.loadLibrary("metin2_mobile");
    }

    public static native void init(Object assetManager, String internalPath, String externalPath, int width, int height);
    public static native void render();
    public static native void touchEvent(int action, float x, float y);
    public static native void cameraRotate(float deltaX, float deltaY);
    public static native void joystickTouch(int action, float x, float y);
    public static native void actionTouch(int action, float x, float y, float deltaX, float deltaY, boolean isDrag);
    public static native void secondaryTouch(int action, float x, float y, float deltaX, float deltaY);
    public static native void onKeyboardText(String text);
    public static native void onKeyboardEnter();
    public static native void pauseAudio();
    public static native void resumeAudio();
    public static native boolean isGamePhase();
    public static native void saveConfig();
    /** PC patcher .lz formatini ([DWORD boyut][ham LZO1X]) acar. 0 = basarili. */
    public static native int lzDecompressFile(String srcPath, String dstPath);

    public static void showKeyboard(String initialText) {
        Log.i("Metin2Mobile", "NativeLib.showKeyboard called with: " + initialText);
        if (MainActivity.sInstance != null) {
            MainActivity.sInstance.showKeyboard(initialText);
        } else {
            Log.e("Metin2Mobile", "NativeLib.showKeyboard: MainActivity.sInstance is null!");
        }
    }

    public static void hideKeyboard() {
        Log.i("Metin2Mobile", "NativeLib.hideKeyboard called");
        if (MainActivity.sInstance != null) {
            MainActivity.sInstance.hideKeyboard();
        }
    }

    public static void showWebPage(String url) {
        Log.i("Metin2Mobile", "NativeLib.showWebPage called with: " + url);
        if (MainActivity.sInstance != null) {
            MainActivity.sInstance.showWebPage(url);
        } else {
            Log.e("Metin2Mobile", "NativeLib.showWebPage: MainActivity.sInstance is null!");
        }
    }

    public static void hideWebPage() {
        Log.i("Metin2Mobile", "NativeLib.hideWebPage called");
        if (MainActivity.sInstance != null) {
            MainActivity.sInstance.hideWebPage();
        }
    }

    public static boolean isWebShowing() {
        if (MainActivity.sInstance != null) {
            return MainActivity.sInstance.isWebShowing();
        }
        return false;
    }

    public static void updateJoystickZone(float x, float y, float width, float height) {
        MainActivity.setJoystickZone(x, y, width, height);
    }

    public static void exitApp() {
        Log.i("Metin2Mobile", "NativeLib.exitApp called");
        if (MainActivity.sInstance != null) {
            MainActivity.sInstance.exitApp();
        } else {
            System.exit(0);
        }
    }
}
