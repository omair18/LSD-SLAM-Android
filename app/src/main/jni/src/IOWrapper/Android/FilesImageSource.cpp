#include "FilesImageSource.h"
#include "logger.h"
#include <boost/thread.hpp>
#include "opencv2/opencv.hpp"
#include "misc.h"


using namespace lsd_slam;

FilesImageSource::FilesImageSource(const std::string& sourceDir)
    : sourceDir_(sourceDir), loopDone_(false)
{
    if(getdir(sourceDir_, files_) >= 0) {
        LOGD("found %d image files in folder %s!\n", (int)files_.size(), sourceDir_.c_str());
    }
    else if(getFile(sourceDir_, files_) >= 0) {
        LOGD("found %d image files in file %s!\n", (int)files_.size(), sourceDir_.c_str());
    }
    else {
        LOGD("could not load file list! wrong path / file?\n");
    }
    imageBuffer_ = new NotifyBuffer<TimestampedMat*>(8);
}

FilesImageSource::~FilesImageSource()
{
    loopDone_.assignValue(true);
    delete imageBuffer_;
    imageBuffer_ = NULL;
}

void FilesImageSource::run()
{
	boost::thread thread(boost::ref(*this));
}

void FilesImageSource::operator()()
{
    LOGD("FilesImageSource thread start\n");
    loop();
    LOGD("FilesImageSource thread exit.\n");
}

void FilesImageSource::stop() {
    loopDone_.assignValue(true);
}

void FilesImageSource::loop() {
    // get HZ
    double hz = 30;
    cv::Mat image = cv::Mat(height_, width_, CV_8U);
    float fakeTimeStamp = 0;

    for(unsigned int i = 0; i < files_.size(); i++)
    {
        if(loopDone_.getValue())
            break;

        cv::Mat imageDist = cv::Mat(height_, width_, CV_8U);
        imageDist = cv::imread(files_[i], CV_LOAD_IMAGE_GRAYSCALE);
        if(imageDist.rows != undistorter_->getInputHeight()|| imageDist.cols != undistorter_->getInputWidth())
        {
            if(imageDist.rows * imageDist.cols == 0)
                LOGE("failed to load image %s! skipping.\n", files_[i].c_str());
            else
                LOGE("image %s has wrong dimensions - expecting %d x %d, found %d x %d. Skipping.\n",
                        files_[i].c_str(), width_, height_, imageDist.cols, imageDist.rows);
            continue;
        }
        assert(imageDist.type() == CV_8U);
        undistorter_->undistort(imageDist, image);
        assert(image.type() == CV_8U);
        
        TimestampedMat* bufferItem = new TimestampedMat();
        bufferItem->timestamp = Timestamp(Timestamp::now().toSec() + fakeTimeStamp);
        bufferItem->data = image;
        if (imageBuffer_ != NULL) {
            while(!imageBuffer_->pushBack(bufferItem)) continue;     // TODO: use better way
        }

        fakeTimeStamp += 0.03;
    }
}

