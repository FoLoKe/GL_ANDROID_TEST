package com.foloke.gltest;

import androidx.appcompat.app.AppCompatActivity;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    private GL3SurfaceView glSurfaceView;
    private AssetManager asm;

    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        asm = getResources().getAssets();
        glSurfaceView = new GL3SurfaceView(this);

        // Проверяем поддерживается ли OpenGL ES 2.0.
        final ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        final ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
        final boolean supportsEs3 = configurationInfo.reqGlEsVersion >= 0x30000;
        if (supportsEs3) {
// Request an OpenGL ES 2.0 compatible context.

// Assign our renderer.

            glSurfaceView.setEGLContextClientVersion(3);
            glSurfaceView.init();
            load(asm);
            setContentView(glSurfaceView);
            //rendererSet = true;
        } else {
            Toast.makeText(this, "This device does not support OpenGL ES 2.0.", Toast.LENGTH_LONG).show(); return;
        }


    }

    //View v = new GL2JNI
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native String test();

    public static native void step();
    public static native void init(int w, int h);
    public static native void load(AssetManager asm);
}