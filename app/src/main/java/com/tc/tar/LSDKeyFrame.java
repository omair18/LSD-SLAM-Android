package com.tc.tar;

/**
 * Created by aarontang on 2017/4/10.
 */

public class LSDKeyFrame {
    float[] pose;
    int pointCount;
    float[] worldPoints;    // 3 float per point, world coordinate(NOTE!!)
    int[] colors;           // 1 int per point
}
