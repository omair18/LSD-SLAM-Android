# LSD-SLAM: Large-Scale Direct Monocular SLAM ANDROID

LSD-SLAM is a novel approach to real-time monocular SLAM. It is fully direct (i.e. does not use keypoints / features) and creates large-scale, 
semi-dense maps in real-time on a laptop. For more information see
[http://vision.in.tum.de/lsdslam](http://vision.in.tum.de/lsdslam)
where you can also find the corresponding publications and Youtube videos, as well as some 
example-input datasets, and the generated output as rosbag or .ply point cloud.

This fork contains Android Version of LSD SLAM, including the JNI files used by https://github.com/striversist/LSDDemo. 

Android application has been forked from https://github.com/striversist/LSDDemo. 

All dependencies & libraries have been included in JNI folder. I have also placed the NDK compiled version of g2o library (for armeabi & armebi-v7a archs). 

I have used ndk-14b for the compilation. 

#Recommended: 
Run ndk-build inside JNI folder before running this app to get fresh compiled libraries.. 

#Note:
All credit goes to authors of lsdslam. My contribution is only related to compilation for Android platform. 

![ScreenShot](https://github.com/omair18/LSD-SLAM/blob/master/snap.png)

