#ifndef FILES_IMAGE_SOURCE_H_
#define FILES_IMAGE_SOURCE_H_

#include "ImageSource.h"
#include <string>
#include <vector>
#include "util/ThreadMutexObject.h"


namespace lsd_slam
{
class FilesImageSource : public ImageSource
{
public:
    FilesImageSource(const std::string& sourceDir);
    ~FilesImageSource();

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
    const std::string sourceDir_;
    std::vector<std::string> files_;
    ThreadMutexObject<bool> loopDone_;
};
}
#endif // FILES_IMAGE_SOURCE_H_
