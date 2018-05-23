package com.tc.tar;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;

/**
 * Created by aarontang on 2017/3/16.
 */

public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback, Camera.PreviewCallback {

    public static final String TAG = CameraPreview.class.getSimpleName();
    public static final int CAMERA_WIDTH = 720;
    public static final int CAMERA_HEIGHT = 480;
    private static final int MAGIC_TEX_ID = 10;

    public interface ARListener {
        void onFrame(byte[] bytes);
    }

    private ARListener mListener;
    private SurfaceHolder mHolder;
    private Camera mCamera;
    private SurfaceTexture mSurfaceTexture;
    private boolean mVisible;

    public CameraPreview(Context context, Camera camera, boolean visible) {
        super(context);
        mCamera = camera;
        Camera.Parameters params = camera.getParameters();
        params.setPreviewSize(CAMERA_WIDTH, CAMERA_HEIGHT);
        params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
        mCamera.addCallbackBuffer(new byte[CAMERA_WIDTH * CAMERA_HEIGHT * 3]);
        mCamera.setParameters(params);
        mSurfaceTexture = new SurfaceTexture(MAGIC_TEX_ID);

        // Install a SurfaceHolder.Callback so we get notified when the
        // underlying surface is created and destroyed.
        mHolder = getHolder();
        mHolder.addCallback(this);
        // deprecated setting, but required on Android versions prior to 3.0
        mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        mVisible = visible;
    }

    public void setARListener(ARListener listener) {
        mListener = listener;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // The Surface has been created, now tell the camera where to draw the preview.
        try {
            mCamera.setPreviewCallbackWithBuffer(this);
            if (mVisible) {
                mCamera.setPreviewDisplay(holder);
            } else {
                mCamera.setPreviewTexture(mSurfaceTexture);
            }
            mCamera.startPreview();
        } catch (IOException e) {
            Log.d(TAG, "Error setting camera preview: " + e.getMessage());
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        // empty. Take care of releasing the Camera preview in your activity.
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        // If your preview can change or rotate, take care of those events here.
        // Make sure to stop the preview before resizing or reformatting it.

        if (mHolder.getSurface() == null) {
            // preview surface does not exist
            return;
        }

        // stop preview before making changes
        try {
            mCamera.stopPreview();
        } catch (Exception e) {
            // ignore: tried to stop a non-existent preview
        }

        // set preview size and make any resize, rotate or
        // reformatting changes here

        // start preview with new settings
        try {
            mCamera.setPreviewCallbackWithBuffer(this);
            if (mVisible) {
                mCamera.setPreviewDisplay(mHolder);
            } else {
                mCamera.setPreviewTexture(mSurfaceTexture);
            }
            mCamera.startPreview();
        } catch (Exception e) {
            Log.d(TAG, "Error starting camera preview: " + e.getMessage());
        }
    }

    @Override
    public void onPreviewFrame(byte[] bytes, Camera camera) {
        camera.addCallbackBuffer(bytes);
        if (mListener != null) {
            mListener.onFrame(bytes);
        }
    }

}
