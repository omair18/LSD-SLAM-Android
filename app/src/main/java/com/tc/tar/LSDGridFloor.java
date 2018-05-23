package com.tc.tar;

import android.opengl.GLES20;

import org.rajawali3d.Object3D;
import org.rajawali3d.materials.Material;
import org.rajawali3d.math.Matrix4;
import org.rajawali3d.math.Quaternion;
import org.rajawali3d.math.vector.Vector3;
import org.rajawali3d.primitives.Line3D;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Stack;

/**
 * Created by aarontang on 2017/4/6.
 */

public class LSDGridFloor {

    public LSDGridFloor() {
    }

    public Collection<Object3D> createGridFloor() {
        ArrayList<Object3D> result = new ArrayList<>();

        Stack<Vector3> points = new Stack<>();
        double dGridInterval = 0.1;

        // Create big grid
        double dMin = -100.0 * dGridInterval;
        double dMax = 100.0 * dGridInterval;
        double height = -4;
        int color = 0x4c4c4c;
        for(int x = -10; x <= 10; x += 1)
        {
            if(x == 0)
                color = 0xffffff;
            else
                color = 0x4c4c4c;
            points.add(new Vector3((double) x * 10 * dGridInterval, dMin, height));
            points.add(new Vector3((double) x * 10 * dGridInterval, dMax, height));
        }

        for(int y = -10; y <= 10; y += 1)
        {
            if(y == 0)
                color = 0xffffff;
            else
                color = 0x4c4c4c;
            points.add(new Vector3(dMin, (double) y * 10 * dGridInterval, height));
            points.add(new Vector3(dMax, (double) y * 10 * dGridInterval, height));
        }
        result.add(createLine(points, color, 1));

        // Create small grid
        points.clear();
        dMin = -10.0 * dGridInterval;
        dMax = 10.0 * dGridInterval;
        color = 0x808080;
        for(int x = -10; x <= 10; x++)
        {
            if(x == 0)
                color = 0xffffff;
            else
                color = 0x808080;
            points.add(new Vector3((double) x * dGridInterval, dMin, height));
            points.add(new Vector3((double) x * dGridInterval, dMax, height));
        }

        for(int y = -10; y <= 10; y++)
        {
            if(y == 0)
                color = 0xffffff;
            else
                color = 0x808080;
            points.add(new Vector3(dMin, (double) y * dGridInterval, height));
            points.add(new Vector3(dMax, (double) y * dGridInterval, height));
        }
        result.add(createLine(points, color, 1));

        // Create axis
        points.clear();
        points.add(new Vector3(0, 0, height));
        points.add(new Vector3(1, 0, height));
        result.add(createLine(points, 0xff0000, 2));

        points.clear();
        points.add(new Vector3(0, 0, height));
        points.add(new Vector3(0, 1, height));
        result.add(createLine(points, 0x00ff00, 2));

        points.clear();
        points.add(new Vector3(0, 0, height));
        points.add(new Vector3(0, 0, height + 1));
        result.add(createLine(points, 0x0000ff, 2));

        return result;
    }

    private Line3D createLine(Stack<Vector3> points, int color, int thickness) {
        Line3D line = new Line3D(points, thickness, color);
        line.setMaterial(new Material());
        line.setDrawingMode(GLES20.GL_LINES);

        float[] pose = {0, 0, 1, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 1};
        Matrix4 poseMatrix = new Matrix4(pose);
        line.setPosition(poseMatrix.getTranslation());
        line.setOrientation(new Quaternion().fromMatrix(poseMatrix));

        return line;
    }

}
