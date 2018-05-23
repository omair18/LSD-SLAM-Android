#include "VideoImageSource.h"
#include "logger.h"


using namespace lsd_slam;

extern "C" jclass findClass(const char* name);
VideoImageSource::VideoImageSource(JavaVM* jvm)
    : jvm_(jvm), loopDone_(false)
{
    imageBuffer_ = new NotifyBuffer<TimestampedMat*>(8);
}

VideoImageSource::~VideoImageSource()
{
    loopDone_.assignValue(true);
    delete imageBuffer_;
    imageBuffer_ = NULL;
}

void VideoImageSource::run()
{
    boost::thread thread(boost::ref(*this));
}

void VideoImageSource::operator()()
{
    LOGD("VideoImageSource thread start\n");
    loop();
    LOGD("VideoImageSource thread exit.\n");
}


void VideoImageSource::stop()
{
    loopDone_.assignValue(true);
}

JNICALL void VideoImageSource::loop()
{
    JNIEnv* env;
    int status = jvm_->AttachCurrentThread(&env, NULL);
    if (status != JNI_OK) {
        LOGE("AttachCurrentThread failed, code=%d\n", status);
        return;
    }
    // Get object
    jclass main = findClass("com/tc/tar/MainActivity");
	jmethodID getVideoSource = env->GetStaticMethodID(main, "getVideoSource", "()Lcom/tc/tar/VideoSource;");
	jobject videoSourceObj = env->CallStaticObjectMethod(main, getVideoSource);
    jclass videoSourceCls = env->GetObjectClass(videoSourceObj);
    jmethodID getFrame = env->GetMethodID(videoSourceCls, "getFrame", "()[B");

	cv::Mat image = cv::Mat(height_, width_, CV_8U);
	unsigned char grayData[width_ * height_] = {0};
	while (!loopDone_.getValue()) {
    	// Call object's method
        jbyteArray frameData = (jbyteArray) env->CallObjectMethod(videoSourceObj, getFrame);
        if (frameData == NULL) {
            env->DeleteLocalRef(frameData);
            continue;
        }
        env->GetByteArrayRegion(frameData, 0, width_ * height_, (jbyte*)grayData);
	    cv::Mat imageDist = cv::Mat(height_, width_, CV_8U, (unsigned char*)grayData);
	    assert(imageDist.type() == CV_8U);
        undistorter_->undistort(imageDist, image);
        assert(image.type() == CV_8U);
        
        TimestampedMat* bufferItem = new TimestampedMat();
        bufferItem->timestamp = Timestamp(Timestamp::now().toSec());
        bufferItem->data = image;
        if (imageBuffer_ != NULL) {
            while(!imageBuffer_->pushBack(bufferItem)) continue;     // TODO: use better way
        }
        
        //env->ReleaseByteArrayElements(frameData, pNV21FrameData, 0);
        env->DeleteLocalRef(frameData);
	}
	jvm_->DetachCurrentThread();
}

