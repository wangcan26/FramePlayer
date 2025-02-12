package com.demo;

import android.app.Activity;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.view.WindowManager;

import androidx.annotation.Nullable;

import com.demo.view.FrameSurfaceView;

public class TestFramePlayer extends Activity {
    private FrameSurfaceView mSurfaceView;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test_frame_player);
        //getWindow().setFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS, WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        //getWindow().setFormat(PixelFormat.TRANSLUCENT);
        mSurfaceView = (FrameSurfaceView) findViewById(R.id.frame_surface_view);
        mSurfaceView.init(this);
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mSurfaceView.onDestroy();
    }
}
