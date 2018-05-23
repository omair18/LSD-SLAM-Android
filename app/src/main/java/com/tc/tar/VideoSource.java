package com.tc.tar;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.os.Environment;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Arrays;

/**
 * Created by aarontang on 2017/4/17.
 */

public class VideoSource implements Camera.PreviewCallback {

    public static final String TAG = VideoSource.class.getSimpleName();
    private static final int MAGIC_TEX_ID = 10;
    private int mWidth;
    private int mHeight;

    private Camera mCamera;
    private SurfaceTexture mSurfaceTexture;
    private byte[] mCurrentFrame;
    private Object mFrameLock = new Object();

    public VideoSource(Context context, int width, int height) {
        mCamera = getCameraInstance();
        mWidth = width;
        mHeight = height;
        Camera.Parameters params = mCamera.getParameters();
        params.setPreviewSize(mWidth, mHeight);
        params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
        params.setPreviewFormat(ImageFormat.NV21);
        mCamera.setParameters(params);
        int format = params.getPreviewFormat();
        mCamera.addCallbackBuffer(new byte[mWidth * mHeight * ImageFormat.getBitsPerPixel(format) / 8]);
        mSurfaceTexture = new SurfaceTexture(MAGIC_TEX_ID);
    }

    public void start() {
        try {
            mCamera.setPreviewCallbackWithBuffer(this);
            mCamera.setPreviewTexture(mSurfaceTexture);
            mCamera.startPreview();
        } catch (IOException e) {
            Log.d(TAG, "Error setting camera preview: " + e.getMessage());
        }
    }

    public void stop() {
        try {
            mCamera.stopPreview();
            mCamera.release();
        } catch (Exception e) {
            // ignore: tried to stop a non-existent preview
        }
    }

    public byte[] getFrame() {
        if (mCurrentFrame == null)
            return null;

        byte[] copyData;
        synchronized (mFrameLock) {
            copyData = Arrays.copyOf(mCurrentFrame, mCurrentFrame.length);
        }
        return copyData;
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        camera.addCallbackBuffer(data);
        synchronized (mFrameLock) {
            mCurrentFrame = data;
        }
    }

    /** A safe way to get an instance of the Camera object. */
    public static Camera getCameraInstance(){
        Camera c = null;
        try {
            c = Camera.open(); // attempt to get a Camera instance
        }
        catch (Exception e){
            // Camera is not available (in use or does not exist)
        }
        return c; // returns null if camera is unavailable
    }
}
