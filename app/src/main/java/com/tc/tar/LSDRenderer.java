package com.tc.tar;

import android.content.Context;
import android.opengl.GLES20;
import android.view.MotionEvent;
import android.widget.Toast;

import com.tc.tar.rajawali.PointCloud;

import org.rajawali3d.Object3D;
import org.rajawali3d.cameras.ArcballCamera;
import org.rajawali3d.materials.Material;
import org.rajawali3d.math.Matrix4;
import org.rajawali3d.math.Quaternion;
import org.rajawali3d.math.vector.Vector3;
import org.rajawali3d.primitives.Line3D;
import org.rajawali3d.renderer.Renderer;
import org.rajawali3d.util.ArrayUtils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Stack;

/**
 * Created by aarontang on 2017/4/6.
 */

public class LSDRenderer extends Renderer {
    public static final String TAG = LSDRenderer.class.getSimpleName();
    private static final float CAMERA_NEAR = 0.01f;
    private static final float CAMERA_FAR = 200f;
    private static final int MAX_POINTS = 500000;

    private float intrinsics[];
    private int resolution[];
    private Object3D mCurrentCameraFrame;
    private ArrayList<Object3D> mCameraFrames = new ArrayList<>();
    private int mLastKeyFrameCount;
    private PointCloud mPointCloud;
    private boolean mHasPointCloudAdded;
    private RenderListener mRenderListener;

    public interface RenderListener {
        void onRender();
    }

    public LSDRenderer(Context context) {
        super(context);
    }

    @Override
    protected void initScene() {
        intrinsics = TARNativeInterface.nativeGetIntrinsics();
        resolution = TARNativeInterface.nativeGetResolution();

        ArcballCamera arcball = new ArcballCamera(mContext, ((MainActivity)mContext).getView());
        arcball.setPosition(0, -4, 0);
        getCurrentScene().replaceAndSwitchCamera(getCurrentCamera(), arcball);
        getCurrentCamera().setNearPlane(CAMERA_NEAR);
        getCurrentCamera().setFarPlane(CAMERA_FAR);
        getCurrentCamera().setFieldOfView(37.5);

        drawGrid();
    }

    @Override
    public void onOffsetsChanged(float xOffset, float yOffset, float xOffsetStep, float yOffsetStep, int xPixelOffset, int yPixelOffset) {

    }

    @Override
    public void onTouchEvent(MotionEvent event) {

    }

    @Override
    protected void onRender(long ellapsedRealtime, double deltaTime) {
        super.onRender(ellapsedRealtime, deltaTime);
        drawFrustum();
        drawKeyframes();
        if (mRenderListener != null) {
            mRenderListener.onRender();
        }
    }

    public void setRenderListener(RenderListener listener) {
        mRenderListener = listener;
    }

    private void drawGrid() {
        getCurrentScene().addChildren(new LSDGridFloor().createGridFloor());
    }

    private void drawFrustum() {
        float pose[] = TARNativeInterface.nativeGetCurrentPose();
        Matrix4 poseMatrix = new Matrix4();
        poseMatrix.setAll(pose);
        if (mCurrentCameraFrame == null) {
            mCurrentCameraFrame = createCameraFrame(0xff0000, 1);
            getCurrentScene().addChild(mCurrentCameraFrame);
        }
        mCurrentCameraFrame.setPosition(poseMatrix.getTranslation());
        mCurrentCameraFrame.setOrientation(new Quaternion().fromMatrix(poseMatrix));
    }

    private void drawKeyframes() {
        int currentKeyFrameCount = TARNativeInterface.nativeGetKeyFrameCount();
        if (mLastKeyFrameCount < currentKeyFrameCount) {
            LSDKeyFrame[] keyFrames = TARNativeInterface.nativeGetAllKeyFrames();
            if (keyFrames == null || keyFrames.length == 0) {
                return;
            }

            drawPoints(keyFrames);
            drawCamera(keyFrames);

            mLastKeyFrameCount = currentKeyFrameCount;
        }
    }

    private void drawPoints(LSDKeyFrame[] keyFrames) {
        float[] vertices = null;
        int[] colors = null;
        int pointNum = 0;
        for (LSDKeyFrame keyFrame : keyFrames) {
            if (vertices == null) {
                vertices = keyFrame.worldPoints;
                colors = keyFrame.colors;
            } else {
                vertices = ArrayUtils.concatAllFloat(vertices, keyFrame.worldPoints);
                colors = ArrayUtils.concatAllInt(colors, keyFrame.colors);
            }
            pointNum += keyFrame.pointCount;
        }

        if (!mHasPointCloudAdded) {
            mPointCloud = new PointCloud(MAX_POINTS, 3); // 1+ phone maximum value
            getCurrentScene().addChild(mPointCloud);
            mHasPointCloudAdded = true;
        }

        if (pointNum >= MAX_POINTS) {
            ((MainActivity)mContext).runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(mContext, "Max point limit reached. (" + MAX_POINTS + ")!!", Toast.LENGTH_LONG).show();
                }
            });
            return;
        }

        ByteBuffer byteBuf = ByteBuffer.allocateDirect(vertices.length * 4); // 4 bytes per float
        byteBuf.order(ByteOrder.nativeOrder());
        FloatBuffer buffer = byteBuf.asFloatBuffer();
        buffer.put(vertices);
        buffer.position(0);
        mPointCloud.updateCloud(pointNum, buffer, colors);
    }

    private void drawCamera(LSDKeyFrame[] keyFrames) {
        float allPose[] = getAllPose(keyFrames);
        for (Object3D obj : mCameraFrames) {
            getCurrentScene().removeChild(obj);
        }
        mCameraFrames.clear();
        for (int i = 0; i < keyFrames.length; ++i) {
            float pose[] = Arrays.copyOfRange(allPose, i * 16, i * 16 + 16);
            Matrix4 poseMatrix = new Matrix4();
            poseMatrix.setAll(pose);
            Line3D line = createCameraFrame(0xff0000, 2);
            line.setPosition(poseMatrix.getTranslation());
            line.setOrientation(new Quaternion().fromMatrix(poseMatrix));
            mCameraFrames.add(line);
        }
        getCurrentScene().addChildren(mCameraFrames);
    }

    private float[] getAllPose(LSDKeyFrame[] keyframes) {
        float allPose[] = new float[keyframes.length * 16];
        int offset = 0;
        for (LSDKeyFrame keyFrame : keyframes) {
            System.arraycopy(keyFrame.pose, 0, allPose, offset, keyFrame.pose.length);
            offset += keyFrame.pose.length;
        }
        return allPose;
    }

    private Line3D createCameraFrame(int color, int thickness) {
        float cx = intrinsics[0];
        float cy = intrinsics[1];
        float fx = intrinsics[2];
        float fy = intrinsics[3];
        int width = resolution[0];
        int height = resolution[1];

        Stack<Vector3> points = new Stack<>();
        points.add(new Vector3(0, 0, 0));
        points.add(new Vector3(0.05 * (0 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05));
        points.add(new Vector3(0, 0, 0));
        points.add(new Vector3(0.05 * (0 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05));
        points.add(new Vector3(0, 0, 0));
        points.add(new Vector3(0.05 * (width - 1 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05));
        points.add(new Vector3(0, 0, 0));
        points.add(new Vector3(0.05 * (width - 1 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05));
        points.add(new Vector3(0.05 * (width - 1 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05));
        points.add(new Vector3(0.05 * (width - 1 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05));
        points.add(new Vector3(0.05 * (width - 1 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05));
        points.add(new Vector3(0.05 * (0 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05));
        points.add(new Vector3(0.05 * (0 - cx) / fx, 0.05 * (height - 1 - cy) / fy, 0.05));
        points.add(new Vector3(0.05 * (0 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05));
        points.add(new Vector3(0.05 * (0 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05));
        points.add(new Vector3(0.05 * (width - 1 - cx) / fx, 0.05 * (0 - cy) / fy, 0.05));

        Line3D frame = new Line3D(points, thickness, color);
        frame.setMaterial(new Material());
        frame.setDrawingMode(GLES20.GL_LINES);
        return frame;
    }
}
