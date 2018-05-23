// g2o - General Graph Optimization
// Copyright (C) 2011 R. Kuemmerle, G. Grisetti, W. Burgard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef G2O_EDGE_PROJECT_DEPTH_H_
#define G2O_EDGE_PROJECT_DEPTH_H_

#include "g2o/core/base_binary_edge.h"

#include "vertex_se3.h"
#include "vertex_pointxyz.h"
#include "parameter_camera.h"
#include "g2o_types_slam3d_api.h"

namespace g2o {

  /*! \class EdgeProjectDepth
   * \brief g2o edge from a track to a depth camera node using a depth measurement (true distance, not disparity)
   */
  class G2O_TYPES_SLAM3D_API EdgeSE3PointXYZDepth : public BaseBinaryEdge<3, Vector3D, VertexSE3, VertexPointXYZ> {
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    /**
     * Note, that you need to reference a parameter set before you add this Edge to an optimizer. 
     * See: https://github.com/RainerKuemmerle/g2o/issues/34
     * For this Edge class you need the parameter set to contain the camera projection matrix.
     */
    EdgeSE3PointXYZDepth();
    virtual bool read(std::istream& is);
    virtual bool write(std::ostream& os) const;

    /** 
     * return the error estimate as a 3-vector, where the first two dimensions
     * represent the reprojection error in pixels, while the third dimension
     * is the difference in depth.
     */
    void computeError();
    // jacobian
    virtual void linearizeOplus();
    

    /** 
     * Measurement is a 3-vector, where the first two dimensions
     * represent the pixel coordinates, and the third dimension
     * is the depth.
     */
    virtual void setMeasurement(const Vector3D& m){
      _measurement = m;
    }

    virtual bool setMeasurementData(const double* d){
      Eigen::Map<const Vector3D> v(d);
      _measurement = v;
      return true;
    }

    virtual bool getMeasurementData(double* d) const{
      Eigen::Map<Vector3D> v(d);
      v=_measurement;
      return true;
    }
    
    virtual int measurementDimension() const {return 3;}

    virtual bool setMeasurementFromState() ;

    virtual double initialEstimatePossible(const OptimizableGraph::VertexSet& from, 
             OptimizableGraph::Vertex* to) { 
      (void) to; 
      return (from.count(_vertices[0]) == 1 ? 1.0 : -1.0);
    }

    virtual void initialEstimate(const OptimizableGraph::VertexSet& from, OptimizableGraph::Vertex* to);
    Vector3D getReprojectedMeasurement();
    InformationType getReprojectedInformation();
    

  private:
    Eigen::Matrix<double,3,9,Eigen::ColMajor> J; // jacobian before projection

    virtual bool resolveCaches();
    ///params contains 
    /// - the projection (and inverse) Matrix containing the camera calibration
    /// - the offset from the camera's "base_link" to the optical center.
    ///   i.e., paramse->offset() is the relative position of the optical center
    ///   with respect to an arbitrary external point.
    ParameterCamera* params;
    CacheCamera* cache;
  };
#ifdef G2O_HAVE_OPENGL
  /**
   * \brief Visualize a 3D pose-pose constraint
   */
  class G2O_TYPES_SLAM3D_API EdgeSE3PointXYZDepthDrawAction: public DrawAction{
  public:
    EdgeSE3PointXYZDepthDrawAction();
    virtual HyperGraphElementAction* operator()(HyperGraph::HyperGraphElement* element, 
            HyperGraphElementAction::Parameters* params_);
    virtual bool refreshPropertyPtrs(HyperGraphElementAction::Parameters* params_);
    void drawUncertainty(Isometry3D& measuredTo, EdgeSE3PointXYZDepth::InformationType& infoMat);
    void  drawMeasurementAndError(Eigen::Vector3f& fromPos,
                                  Eigen::Vector3f& estToPos,
                                  Eigen::Vector3f& measToPos);
    BoolProperty* _showMeasurementAndError, *_showEllipsoid, *_showEdge;
  };
#endif

}
#endif
