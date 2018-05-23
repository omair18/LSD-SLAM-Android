#include "util/logger.h"
#include <GLES/gl.h>

void printTrans(const Sophus::Sim3f::Transformation& trans) {
    std::ostringstream out;
    out << trans;
    LOGD("%s:\n%s", __FUNCTION__, out.str().c_str());
}

void printMatrix4f(const Sophus::Matrix4f& m) {
    std::ostringstream out;
    out << m;
    LOGD("%s:\n%s", __FUNCTION__, out.str().c_str());
}

void printMatrix4x4(GLfloat* m) {
    LOGD("%s:\n", __FUNCTION__);
    for (int i=0; i<4; ++i) {
        LOGD("%f %f %f %f\n", m[i*4], m[i*4+1], m[i*4+2], m[i*4+3]);
    }
}

void dumpCurrentMatrix() {
    GLfloat matrix[16];
    glGetFloatv (GL_MODELVIEW_MATRIX, matrix);
    printMatrix4x4(matrix);
}

void printObjectClassName(JNIEnv* env, jobject obj) {
    jclass cls = env->GetObjectClass(obj);
    jmethodID getClassMid = env->GetMethodID(cls, "getClass", "()Ljava/lang/Class;");
    jobject clsObj = env->CallObjectMethod(obj, getClassMid);
    
    cls = env->GetObjectClass(clsObj);
    jmethodID getNameMid = env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
    jstring strObj = (jstring)env->CallObjectMethod(clsObj, getNameMid);
    const char* str = env->GetStringUTFChars(strObj, NULL);
    LOGD("\nCalling class is: %s\n", str);
    env->ReleaseStringUTFChars(strObj, str);
}