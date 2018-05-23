#ifndef IMAGE_SOURCE_H_
#define IMAGE_SOURCE_H_
#pragma once

#include "IOWrapper/NotifyBuffer.h"
#include "IOWrapper/TimestampedObject.h"
#include "util/Undistorter.h"
#include "sophus/sim3.hpp"


namespace lsd_slam
{
class ImageSource
{
public:
	virtual ~ImageSource() {};
	
	/**
	 * Starts the thread.
	 */
	virtual void run() {};


	virtual void setCalibration(std::string file);

	/**
	 * Gets the NotifyBuffer to which incoming images are stored.
	 */
	inline NotifyBuffer<TimestampedMat*>* getBuffer() {return imageBuffer_;};


	/**
	 * Gets the Camera Calibration. To avoid any dependencies, just as simple float / int's.
	 */
	inline float fx() {return fx_;}
	inline float fy() {return fy_;}
	inline float cx() {return cx_;}
	inline float cy() {return cy_;}
	inline int width() {return width_;}
	inline int height() {return height_;}
	inline Sophus::Matrix3f K() {return K_;}

protected:
	NotifyBuffer<TimestampedMat*>* imageBuffer_;
	Undistorter* undistorter_;
	float fx_, fy_, cx_, cy_;
	int width_, height_;
	Sophus::Matrix3f K_;
};
}
#endif // IMAGE_SOURCE_H_