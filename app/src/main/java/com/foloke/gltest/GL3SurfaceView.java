package com.foloke.gltest;

import android.content.Context;
import android.opengl.GLSurfaceView;

public class GL3SurfaceView extends GLSurfaceView {

    public GL3SurfaceView(Context context) {
        super(context);
    }

    public void init() {
        setRenderer(new TestRenderer());
    }
}
