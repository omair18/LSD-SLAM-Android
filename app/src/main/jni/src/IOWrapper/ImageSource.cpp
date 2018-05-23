#include "ImageSource.h"
#include "util/logger.h"

using namespace lsd_slam;

void ImageSource::setCalibration(std::string file)
{
    LOGD("calibration file: %s\n", file.c_str());
    undistorter_ = Undistorter::getUndistorterForFile(file.c_str());
	if(undistorter_ == NULL) {
		LOGE("need camera calibration file!\n");
		exit(0);
	}

	width_ = undistorter_->getOutputWidth();
	height_ = undistorter_->getOutputHeight();

	int inWidth = undistorter_->getInputWidth();
	int inHeight = undistorter_->getInputHeight();
	LOGD("width=%d, height=%d, inWidth=%d, inHeight=%d\n", width_, height_, inWidth, inHeight);

	fx_ = undistorter_->getK().at<double>(0, 0);
	fy_ = undistorter_->getK().at<double>(1, 1);
	cx_ = undistorter_->getK().at<double>(2, 0);
	cy_ = undistorter_->getK().at<double>(2, 1);
	
	K_ << fx_, 0.0, cx_, 0.0, fy_, cy_, 0.0, 0.0, 1.0;
	LOGD("fx=%f, fy=%f, cx=%f, cy=%f\n", fx_, fy_, cx_, cy_);
}

