package com.demo.view;

import android.app.Activity;
import android.content.Context;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.util.AttributeSet;
import androidx.annotation.NonNull;

import org.jetbrains.annotations.Nullable;

import java.lang.ref.WeakReference;

public class FrameSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("fpn_mono");
    }

    private SurfaceHolder       mSurfaceHolder;
    private WeakReference<Activity>     mWeakActivity;

    public FrameSurfaceView(Context context) {
        this(context, null);
    }

    public FrameSurfaceView( Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void init(Activity activity) {
        mWeakActivity = new WeakReference<>(activity);
        mSurfaceHolder = this.getHolder();
        mSurfaceHolder.addCallback(this);
        mSurfaceHolder.setKeepScreenOn(true);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }
}
