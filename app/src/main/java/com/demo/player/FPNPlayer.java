package com.demo.player;

import android.view.Surface;

public class FPNPlayer {
    private long mHandle = -1;
    public FPNPlayer() {
        mHandle = nativeCreate();
    }

    public void setSurface(Surface surface) {
        nativeSetSurface(mHandle, surface);
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
}
