package com.demo;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        // 找到按钮并设置点击事件
        Button btnTestPlayer = findViewById(R.id.btnTestPlayer);
        btnTestPlayer.setOnClickListener(v -> {
            Intent intent = new Intent(MainActivity.this, TestFramePlayer.class);
            startActivity(intent);
        });
    }
} 