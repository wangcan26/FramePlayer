package com.demo.view;

import android.app.Activity;
import android.content.Context;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.util.AttributeSet;
import androidx.annotation.NonNull;

import com.demo.player.FPNPlayer;

import org.jetbrains.annotations.Nullable;

import java.lang.ref.WeakReference;

public class FrameSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("fpn_mono");
    }

    private SurfaceHolder       mSurfaceHolder;
    private FPNPlayer           mPlayer;

    public FrameSurfaceView(Context context) {
        this(context, null);
    }

    public FrameSurfaceView( Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void init(Activity activity) {
        mSurfaceHolder = this.getHolder();
        mSurfaceHolder.addCallback(this);
        mSurfaceHolder.setKeepScreenOn(true);

        mPlayer = new FPNPlayer();
    }

    public void onDestroy() {
        if (mPlayer != null) {
            mPlayer.release();
        }
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        mPlayer.setSurface(mSurfaceHolder.getSurface());
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }
}
