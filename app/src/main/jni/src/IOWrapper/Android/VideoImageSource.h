#ifndef VIDEO_IMAGE_SOURCE_H_
#define VIDEO_IMAGE_SOURCE_H_

#include "ImageSource.h"
#include <string>
#include <vector>
#include <jni.h>
#include "util/ThreadMutexObject.h"


namespace lsd_slam
{
class VideoImageSource : public ImageSource
{
public:
    VideoImageSource(JavaVM* jvm);
    ~VideoImageSource();

    /**
	 * Starts the thread.
	 */
	void run();

	/**
	 * Thread main function.
	 */
	void operator()();

	void stop();

private:
    void loop();
    JavaVM* jvm_;
    ThreadMutexObject<bool> loopDone_;
};
}
#endif // VIDEO_IMAGE_SOURCE_H_
