#ifndef LOGGER_H_
#define LOGGER_H_

#include <android/log.h>
#include "sophus/sim3.hpp"
#include <GLES2/gl2.h>
#include <jni.h>


#define STRINGIFY(x) #x
#if 0
#define LOG_TAG    __FILE__ ":" STRINGIFY(LSDNative)
#ifdef LOGD
#undef LOGD
#endif
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#ifdef LOGE
#undef LOGE
#endif
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

#define LOG_TAG     STRINGIFY(LSDNative)
#ifdef LOGD
#undef LOGD
#endif
#define LOGD(x...) do { \
    char buf[512]; \
    sprintf(buf, x); \
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s | %i: %s", __FUNCTION__, __LINE__, buf); \
} while (0)

#ifdef LOGE
#undef LOGE
#endif
#define LOGE(x...) do { \
    char buf[512]; \
    sprintf(buf, x); \
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s | %i: %s", __FUNCTION__, __LINE__, buf); \
} while (0)

void printTrans(const Sophus::Sim3f::Transformation& trans);
void printMatrix4f(const Sophus::Matrix4f& m);
void printMatrix4x4(GLfloat* m);
void dumpCurrentMatrix();
void printObjectClassName(JNIEnv* env, jobject obj);

#endif
