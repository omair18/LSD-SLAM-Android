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

#ifndef G2O_EDGE_SE3_OFFSET_H_
#define G2O_EDGE_SE3_OFFSET_H_

#include "g2o/core/base_binary_edge.h"

#include "vertex_se3.h"
#include "edge_se3.h"
#include "g2o_types_slam3d_api.h"

namespace g2o {
  class ParameterSE3Offset;
  class CacheSE3Offset;

  /**
   * \brief Offset edge.
   * The measurement of this Edge seems to measure only a part of the transformation
   * Between its vertices. I.e., vertex0 * offsetTo * measurement * offsetFrom = vertex1
   *
   * To use the offsets you need to create two ParameterSE3Offsets, e.g.
   * auto* p1 = new ParameterSE3Offset();
   * p1->setId(id1);//id1 is arbitrary
   * Isometry3D measurement2vertex = ...;//Transforms points from measurement frame to vertex frame
   * pc->setOffset(measurement2vertex);
   * optimizer->addParameter(p1);
   * optimizer->addParameter(p2);//created analogous to p1
   * auto* edge = new EdgeSE3Offset();
   * edge->setParameterId(0, id1);
   * edge->setParameterId(1, id2);
   */
  // first two args are the measurement type, second two the connection classes
  class G2O_TYPES_SLAM3D_API EdgeSE3Offset : public EdgeSE3 {
    public:
      EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
      EdgeSE3Offset();
      virtual bool read(std::istream& is);
      virtual bool write(std::ostream& os) const;

      void computeError();


      void linearizeOplus();

      virtual bool setMeasurementFromState() ;

      virtual double initialEstimatePossible(const OptimizableGraph::VertexSet& /*from*/, 
          OptimizableGraph::Vertex* /*to*/) { 
        return 1.;
      }

      virtual void initialEstimate(const OptimizableGraph::VertexSet& from, OptimizableGraph::Vertex* to);

      Isometry3D getMeasurementPlusOffsets();
    protected:
      virtual bool resolveCaches();
      ParameterSE3Offset *_offsetFrom, *_offsetTo;
      CacheSE3Offset  *_cacheFrom, *_cacheTo;
  };

#ifdef G2O_HAVE_OPENGL
  /**
   * \brief Visualize a 3D pose-pose constraint
   */
  class G2O_TYPES_SLAM3D_API EdgeSE3OffsetDrawAction: public EdgeSE3DrawAction {
  public:
    EdgeSE3OffsetDrawAction();
    virtual HyperGraphElementAction* operator()(HyperGraph::HyperGraphElement* element, 
            HyperGraphElementAction::Parameters* params_);
  };
#endif

} // end namespace
#endif
