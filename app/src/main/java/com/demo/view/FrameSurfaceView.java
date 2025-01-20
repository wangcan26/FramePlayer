package com.demo.view;

import android.app.Activity;
import android.content.Context;
import android.graphics.PixelFormat;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.util.AttributeSet;
import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;

import com.demo.player.FPNPlayer;

import org.jetbrains.annotations.Nullable;

import java.io.File;
import java.lang.ref.WeakReference;

public class FrameSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("fpn_mono");
    }
    private String TAG = this.getClass().getSimpleName();
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
        mSurfaceHolder.setFormat(PixelFormat.TRANSLUCENT);
        //this.setZOrderOnTop(true);
        mSurfaceHolder.setKeepScreenOn(true);

        mPlayer = new FPNPlayer();
        //set content uri
        File[] externalStorageVolumes =
                ContextCompat.getExternalFilesDirs(activity.getApplicationContext(), null);
        if (externalStorageVolumes.length == 0) {
            Log.e(TAG, "root url not valid!");
            return;
        }

        File dir = externalStorageVolumes[0];
        assert (dir.exists() && dir.isDirectory());
        String uri = externalStorageVolumes[0].getAbsolutePath();
        Log.i(TAG, "asset dir url: " + uri);
        mPlayer.setContentUri(uri);

        //start
        mPlayer.start();
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
        mPlayer.setSurface(null);
    }
}
