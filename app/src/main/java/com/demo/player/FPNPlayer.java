package com.demo.player;
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
    }

    public void setContentUri(String uri) {
        nativeSetContentUri(mHandle, uri);
    }

    public void start() {
        nativeStart(mHandle);
    }

    public void setSurface(Surface surface) {
        nativeSetSurface(mHandle, surface);
    }

    public long handle() {
        return mHandle;
    }

    public void release() {
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
