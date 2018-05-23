/*
 * PangolinOutput3DWrapper.h
 *
 *  Created on: 17 Oct 2014
 *      Author: thomas
 */

#ifndef ANDROIDOUTPUT3DWRAPPER_H_
#define ANDROIDOUTPUT3DWRAPPER_H_

#include "IOWrapper/Output3DWrapper.h"
#include "Keyframe.h"
#include "util/ThreadMutexObject.h"
#include "util/Resolution.h"


namespace lsd_slam
{

class Frame;
class KeyFrameGraph;

struct GraphConstraint
{
    int from;
    int to;
    float err;
};

class AndroidOutput3DWrapper : public Output3DWrapper
{
    public:
        AndroidOutput3DWrapper(int width, int height);
        virtual ~AndroidOutput3DWrapper();

        virtual void publishKeyframeGraph(KeyFrameGraph* graph);

        // publishes a keyframe. if that frame already existis, it is overwritten, otherwise it is added.
        virtual void publishKeyframe(Frame* f);

        virtual void updateImage(unsigned char * data);

        // published a tracked frame that did not become a keyframe (i.e. has no depth data)
        virtual void publishTrackedFrame(Frame* f);

        // publishes graph and all constraints, as well as updated KF poses.
        virtual void publishTrajectory(std::vector<Eigen::Matrix<float, 3, 1>> trajectory, std::string identifier);

        virtual void publishTrajectoryIncrement(Eigen::Matrix<float, 3, 1> pt, std::string identifier);

        virtual void publishDebugInfo(Eigen::Matrix<float, 20, 1> data);
        ThreadMutexObject<std::map<int, Keyframe *> >& getKeyframes() { return keyframes; }
        int getKeyframesCount() { return keyframes.getReference().size(); }
        ThreadMutexObject<unsigned char * >& getImageBuffer() { return depthImgBuffer; }
        int getImageBufferSize() const { return Resolution::getInstance().numPixels() * 3; }

        int publishLvl;

    private:
        int width, height;
        ThreadMutexObject<std::map<int, Keyframe *> > keyframes;
        ThreadMutexObject<unsigned char * > depthImgBuffer;
};
}

#endif /* ANDROIDOUTPUT3DWRAPPER_H_ */
