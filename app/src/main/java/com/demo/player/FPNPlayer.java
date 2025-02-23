package com.demo.player;
import android.util.Log;
import android.view.Surface;

/**
 * FPNPlayer uses hardware gpu api (e.g opengl) to render
 * raw image data onto the surface.
 * It Holds a handle of native FPNPlayer.
 */
public class FPNPlayer {
    private long mHandle = -1;
    public FPNPlayer() {
        mHandle = nativeCreate();
        Log.i("FPNPlayer", "JavaFPNPlayer create ");
    }

    public void setContentUri(String uri) {
        if (mHandle != -1) {
            nativeSetContentUri(mHandle, uri);
        }
        Log.i("FPNPlayer", "JavaFPNPlayer set content: " + uri);
    }

    public void start() {
        if (mHandle != -1) {
            nativeStart(mHandle);
        }
        Log.i("FPNPlayer", "JavaFPNPlayer start");
    }

    public void setSurface(Surface surface) {
        if (mHandle != -1) {
            nativeSetSurface(mHandle, surface);
        }
        Log.i("FPNPlayer", "JavaFPNPlayer setSurface");
    }

    public long handle() {
        return mHandle;
    }

    public void release() {
        Log.i("FPNPlayer", "JavaFPNPlayer release");
        if (mHandle != -1) {
            nativeRelease(mHandle);
            mHandle = -1;
        }
    }

    static native long nativeCreate();
    static native void nativeRelease(long handle);
    static native void nativeSetSurface(long handle, Surface surface);
    static native void nativeSetContentUri(long handle, String uri);
    static native void nativeStart(long handle);
}
