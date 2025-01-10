package com.demo;

import android.app.Activity;
import android.os.Bundle;

import androidx.annotation.Nullable;

import com.demo.view.FrameSurfaceView;

public class TestFramePlayer extends Activity {
    private FrameSurfaceView mSurfaceView;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test_frame_player);

        // Example of a call to a native method
        mSurfaceView = (FrameSurfaceView) findViewById(R.id.frame_surface_view);
        mSurfaceView.init(this);
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
