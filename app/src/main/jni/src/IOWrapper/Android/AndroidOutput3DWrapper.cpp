/*
 * AndroidOutput3DWrapper.cpp
 *
 *  Created on: 17 Oct 2014
 *      Author: thomas
 */

#include "AndroidOutput3DWrapper.h"

#include "util/SophusUtil.h"
#include "util/settings.h"
#include "DataStructures/Frame.h"
#include "GlobalMapping/KeyFrameGraph.h"
#include "sophus/sim3.hpp"
#include "GlobalMapping/g2oTypeSim3Sophus.h"
#include "util/logger.h"

namespace lsd_slam
{

AndroidOutput3DWrapper::AndroidOutput3DWrapper(int width, int height)
 : width(width),
   height(height),
   publishLvl(0),
   depthImgBuffer(0)
{

}

AndroidOutput3DWrapper::~AndroidOutput3DWrapper()
{
    if(depthImgBuffer.getValue())
        delete [] depthImgBuffer.getValue();

    boost::mutex::scoped_lock lock(keyframes.getMutex());
    for(std::map<int, Keyframe *>::iterator i = keyframes.getReference().begin(); i != keyframes.getReference().end(); ++i)
    {
        delete i->second;
    }
    keyframes.getReference().clear();

    lock.unlock();
}

void AndroidOutput3DWrapper::updateImage(unsigned char * data)
{
    if (depthImgBuffer.getValue() == NULL) {
        LOGD("create image buffer");
        depthImgBuffer.assignValue(new unsigned char[Resolution::getInstance().numPixels() * 3]);
    }
    
    boost::mutex::scoped_lock lock(depthImgBuffer.getMutex());
    memcpy(depthImgBuffer.getReference(), data, Resolution::getInstance().numPixels() * 3);
    lock.unlock();
}

void AndroidOutput3DWrapper::publishKeyframe(Frame* f)
{
    LOGD("publishKeyframe: frame id=%d\n", f->id());
    Keyframe * fMsg = new Keyframe;

    boost::shared_lock<boost::shared_mutex> lock = f->getActiveLock();

    fMsg->id = f->id();
    fMsg->time = f->timestamp();
    fMsg->isKeyframe = true;

    int w = f->width(publishLvl);
    int h = f->height(publishLvl);

    fMsg->camToWorld = f->getScaledCamToWorld().cast<float>();

    fMsg->fx = f->fx(publishLvl);
    fMsg->fy = f->fy(publishLvl);
    fMsg->cx = f->cx(publishLvl);
    fMsg->cy = f->cy(publishLvl);

    fMsg->width = w;
    fMsg->height = h;

    fMsg->pointData = new unsigned char[w * h * sizeof(InputPointDense)];

    InputPointDense * pc = (InputPointDense*)fMsg->pointData;

    const float* idepth = f->idepth(publishLvl);
    const float* idepthVar = f->idepthVar(publishLvl);
    const float* color = f->image(publishLvl);

    for(int idx = 0;idx < w * h; idx++)
    {
        pc[idx].idepth = idepth[idx];
        pc[idx].idepth_var = idepthVar[idx];
        pc[idx].color[0] = color[idx];
        pc[idx].color[1] = color[idx];
        pc[idx].color[2] = color[idx];
        pc[idx].color[3] = color[idx];
    }
    
    lock.unlock();

    // addKeyframe
    boost::mutex::scoped_lock lock2(keyframes.getMutex());
    if(keyframes.getReference().find(fMsg->id) != keyframes.getReference().end())   // Exists
    {
        LOGD("updatePoints id=%d\n", fMsg->id);
        keyframes.getReference()[fMsg->id]->updatePoints(fMsg);
        delete fMsg;
    }
    else
    {
        fMsg->initId = keyframes.getReference().size();
        keyframes.getReference()[fMsg->id] = fMsg;
        LOGD("addKeyframe: initId=%d, id=%d\n", fMsg->initId, fMsg->id);
    }
    lock2.unlock();
}

void AndroidOutput3DWrapper::publishTrackedFrame(Frame* kf)
{
    // TODO:
}

void AndroidOutput3DWrapper::publishKeyframeGraph(KeyFrameGraph* graph)
{
    LOGD("publishKeyframeGraph\n");
    graph->keyframesAllMutex.lock_shared();

    int num = graph->keyframesAll.size();

    unsigned char * buffer = new unsigned char[num * sizeof(GraphFramePose)];

    GraphFramePose* framePoseData = (GraphFramePose*)buffer;

    for(unsigned int i = 0; i < graph->keyframesAll.size(); i++)
    {
        framePoseData[i].id = graph->keyframesAll[i]->id();
        memcpy(framePoseData[i].camToWorld, graph->keyframesAll[i]->getScaledCamToWorld().cast<float>().data(), sizeof(float) * 7);
    }

    graph->keyframesAllMutex.unlock_shared();

    // updateKeyframePoses
    boost::mutex::scoped_lock lock(keyframes.getMutex());
    for(int i = 0; i < num; i++)
    {
        if(keyframes.getReference().find(framePoseData[i].id) != keyframes.getReference().end())
        {
            memcpy(keyframes.getReference()[framePoseData[i].id]->camToWorld.data(), &framePoseData[i].camToWorld[0], sizeof(float) * 7);
        }
    }
    lock.unlock();

    delete [] buffer;
}

void AndroidOutput3DWrapper::publishTrajectory(std::vector<Eigen::Matrix<float, 3, 1>> trajectory, std::string identifier)
{
    // TODO:
}

void AndroidOutput3DWrapper::publishTrajectoryIncrement(Eigen::Matrix<float, 3, 1> pt, std::string identifier)
{
    // TODO:
}

void AndroidOutput3DWrapper::publishDebugInfo(Eigen::Matrix<float, 20, 1> data)
{
    // TODO:
}

}
