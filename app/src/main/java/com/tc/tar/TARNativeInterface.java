package com.tc.tar;

/**
 * Created by aarontang on 2017/3/30.
 */

public class TARNativeInterface {
    public static final String TAG = TARNativeInterface.class.getSimpleName();

    public static native void nativeInit(String calibPath);
    public static native void nativeDestroy();
    public static native void nativeStart();
    public static native void nativeKey(int keycode);
    public static native float[] nativeGetIntrinsics();
    public static native int[] nativeGetResolution();
    public static native float[] nativeGetCurrentPose();
    public static native LSDKeyFrame[] nativeGetAllKeyFrames();
    public static native int nativeGetKeyFrameCount();
    public static native byte[] nativeGetCurrentImage(int format);  // only support format = 0 (ARGB)
}
