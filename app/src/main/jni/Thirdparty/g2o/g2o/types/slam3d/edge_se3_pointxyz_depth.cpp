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

#include "edge_se3_pointxyz_depth.h"
#ifdef G2O_HAVE_OPENGL
#include "g2o/stuff/opengl_wrapper.h"
#include "g2o/stuff/opengl_primitives.h"
#endif

namespace g2o {
  using namespace g2o;

  // point to camera projection, monocular
  EdgeSE3PointXYZDepth::EdgeSE3PointXYZDepth() : BaseBinaryEdge<3, Vector3D, VertexSE3, VertexPointXYZ>() {
    resizeParameters(1);
    installParameter(params, 0);
    information().setIdentity();
    information()(2,2)=100;
    J.fill(0);
    J.block<3,3>(0,0) = -Matrix3D::Identity();
  }

  bool EdgeSE3PointXYZDepth::resolveCaches(){
    ParameterVector pv(1);
    pv[0]=params;
    resolveCache(cache, (OptimizableGraph::Vertex*)_vertices[0],"CACHE_CAMERA",pv);
    return cache != 0;
  }




  bool EdgeSE3PointXYZDepth::read(std::istream& is) {
    int pid;
    is >> pid;
    setParameterId(0,pid);

    // measured keypoint
    Vector3D meas;
    for (int i=0; i<3; i++) is >> meas[i];
    setMeasurement(meas);
    // don't need this if we don't use it in error calculation (???)
    // information matrix is the identity for features, could be changed to allow arbitrary covariances    
    if (is.bad()) {
      return false;
    }
    for ( int i=0; i<information().rows() && is.good(); i++)
      for (int j=i; j<information().cols() && is.good(); j++){
  is >> information()(i,j);
  if (i!=j)
    information()(j,i)=information()(i,j);
      }
    if (is.bad()) {
      //  we overwrite the information matrix
      information().setIdentity();
      information()(2,2)=10/_measurement(2); // scale the info by the inverse of the measured depth
    } 
    return true;
  }

  bool EdgeSE3PointXYZDepth::write(std::ostream& os) const {
    os << params->id() << " ";
    for (int i=0; i<3; i++) os  << measurement()[i] << " ";
    for (int i=0; i<information().rows(); i++)
      for (int j=i; j<information().cols(); j++) {
        os <<  information()(i,j) << " ";
      }
    return os.good();
  }


  void EdgeSE3PointXYZDepth::computeError() {
    // from cam to point (track)
    //VertexSE3 *cam = static_cast<VertexSE3*>(_vertices[0]);
    VertexPointXYZ *point = static_cast<VertexPointXYZ*>(_vertices[1]);

    Vector3D p = cache->w2i() * point->estimate();
    Vector3D perr;
    perr.head<2>() = p.head<2>()/p(2);
    perr(2) = p(2);

    // error, which is backwards from the normal observed - calculated
    // _measurement is the measured projection
    _error = perr - _measurement;
    //    std::cout << _error << std::endl << std::endl;
  }

  void EdgeSE3PointXYZDepth::linearizeOplus() {
    //VertexSE3 *cam = static_cast<VertexSE3 *>(_vertices[0]);
    VertexPointXYZ *vp = static_cast<VertexPointXYZ *>(_vertices[1]);

    const Vector3D& pt = vp->estimate();

    Vector3D Zcam = cache->w2l() * pt;

    //  J(0,3) = -0.0;
    J(0,4) = -2*Zcam(2);
    J(0,5) = 2*Zcam(1);

    J(1,3) = 2*Zcam(2);
    //  J(1,4) = -0.0;
    J(1,5) = -2*Zcam(0);

    J(2,3) = -2*Zcam(1);
    J(2,4) = 2*Zcam(0);
    //  J(2,5) = -0.0;

    J.block<3,3>(0,6) = cache->w2l().rotation();

    Eigen::Matrix<double,3,9,Eigen::ColMajor> Jprime = params->Kcam_inverseOffsetR()  * J;
    Vector3D Zprime = cache->w2i() * pt;

    Eigen::Matrix<double,3,9,Eigen::ColMajor> Jhom;
    Jhom.block<2,9>(0,0) = 1/(Zprime(2)*Zprime(2)) * (Jprime.block<2,9>(0,0)*Zprime(2) - Zprime.head<2>() * Jprime.block<1,9>(2,0));
    Jhom.block<1,9>(2,0) = Jprime.block<1,9>(2,0);

    _jacobianOplusXi = Jhom.block<3,6>(0,0);
    _jacobianOplusXj = Jhom.block<3,3>(0,6);
  }


  bool EdgeSE3PointXYZDepth::setMeasurementFromState(){
    //VertexSE3 *cam = static_cast<VertexSE3*>(_vertices[0]);
    VertexPointXYZ *point = static_cast<VertexPointXYZ*>(_vertices[1]);

    // calculate the projection
    const Vector3D& pt = point->estimate();

    Vector3D p = cache->w2i() * pt;
    Vector3D perr;
    perr.head<2>() = p.head<2>()/p(2);
    perr(2) = p(2);
    _measurement = perr;
    return true;
  }

  EdgeSE3PointXYZDepth::InformationType EdgeSE3PointXYZDepth::getReprojectedInformation()
  {
    const Eigen::Matrix<double, 2, 2, Eigen::ColMajor>& invKcam = params->Kcam().topLeftCorner<2, 2>();
    InformationType m = information();
    m.topLeftCorner<2,2>() = invKcam.transpose() * m.topLeftCorner<2,2>() * invKcam;//not 100% sure about this
    return m;
  }
  Vector3D EdgeSE3PointXYZDepth::getReprojectedMeasurement()
  {
    const Eigen::Matrix<double, 3, 3, Eigen::ColMajor>& invKcam = params->invKcam();
    Vector3D p;
    p(2) = _measurement(2);
    p.head<2>() = _measurement.head<2>()*p(2);
    p=invKcam*p;
    return params->offset() * p;
  }

  void EdgeSE3PointXYZDepth::initialEstimate(const OptimizableGraph::VertexSet& from, OptimizableGraph::Vertex* /*to_*/)
  {
    (void) from;
    assert(from.size() == 1 && from.count(_vertices[0]) == 1 && "Can not initialize VertexDepthCam position by VertexTrackXYZ");

    VertexSE3 *cam = dynamic_cast<VertexSE3*>(_vertices[0]);
    VertexPointXYZ *point = dynamic_cast<VertexPointXYZ*>(_vertices[1]);
    const Eigen::Matrix<double, 3, 3, Eigen::ColMajor>& invKcam = params->invKcam();
    Vector3D p;
    p(2) = _measurement(2);
    p.head<2>() = _measurement.head<2>()*p(2);
    p=invKcam*p;
    point->setEstimate(cam->estimate() * (params->offset() * p));
  }
#ifdef G2O_HAVE_OPENGL

  bool EdgeSE3PointXYZDepthDrawAction::refreshPropertyPtrs(HyperGraphElementAction::Parameters* params_){
    if (!DrawAction::refreshPropertyPtrs(params_))
      return false;
    if (_previousParams){
      _showMeasurementAndError = _previousParams->makeProperty<BoolProperty>(_typeName + "::SHOW_MEASUREMENT_AND_ERROR", false);
      _showEllipsoid = _previousParams->makeProperty<BoolProperty>(_typeName + "::SHOW_STD_DEV", false);
      _showEdge = _previousParams->makeProperty<BoolProperty>(_typeName + "::SHOW_EDGE", false);
    } else {
      _showMeasurementAndError = 0;
      _showEllipsoid = 0;
      _showEdge = 0;
    }
    return true;
  }

  EdgeSE3PointXYZDepthDrawAction::EdgeSE3PointXYZDepthDrawAction(): DrawAction(typeid(EdgeSE3PointXYZDepth).name()){}

  void  EdgeSE3PointXYZDepthDrawAction::drawMeasurementAndError(Eigen::Vector3f& fromPos,
                                                                Eigen::Vector3f& estToPos,
                                                                Eigen::Vector3f& measToPos)
  {
      glPushAttrib(GL_LINE_BIT);
      glLineStipple(1, 0xAAAA);
      glLineWidth(EDGE_LINE_WIDTH);
      glEnable(GL_LINE_STIPPLE);
      glBegin(GL_LINES);
      //Measured transformation in yellow 
      glColor3f(POSE_EDGE_MEASUREMENT_COLOR);
      glVertex3f(fromPos.x(),fromPos.y(),fromPos.z());
      glVertex3f(measToPos.x(),measToPos.y(),measToPos.z());
      //and difference to estimate in dotted red
      glColor3f(POSE_EDGE_ERROR_COLOR);
      glVertex3f(measToPos.x(),measToPos.y(),measToPos.z());
      glVertex3f(estToPos.x(),estToPos.y(),estToPos.z());
      glEnd();
      glPopAttrib();
  }

  void EdgeSE3PointXYZDepthDrawAction::drawUncertainty(Isometry3D& measuredTo, EdgeSE3PointXYZDepth::InformationType& infoMat) {
      //Draw uncertainty ellipsoid for one std dev
      glColor3f(EDGE_UNCERTAINTY_ELLIPSOID_COLOR);
      glPushMatrix();
      glPushAttrib(GL_POLYGON_BIT);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//Draw wireframe
      glMultMatrixd(measuredTo.matrix().data());
      opengl::drawEllipsoid(sqrt(1./infoMat(0,0)), sqrt(1./infoMat(1,1)), sqrt(1./infoMat(2,2)));
      glPopAttrib();//Restore from wireframe
      glPopMatrix();
  }


  HyperGraphElementAction* EdgeSE3PointXYZDepthDrawAction::operator()(HyperGraph::HyperGraphElement* element, 
               HyperGraphElementAction::Parameters* params_){
    if (typeid(*element).name()!=_typeName)
      return 0;
    refreshPropertyPtrs(params_);
    if (! _previousParams)
      return this;
    
    if (_show && !_show->value())
      return this;
    
    auto* edge = static_cast<EdgeSE3PointXYZDepth*>(element);
    auto* from = static_cast<VertexSE3*>(edge->vertices()[0]);
    auto* to   = static_cast<VertexPointXYZ*>(edge->vertices()[1]);
    Eigen::Vector3f fromPos = from->estimate().translation().cast<float>();
    Eigen::Vector3f estToPos = to->estimate().cast<float>();
    Eigen::Vector3f measToPos = (from->estimate() * edge->getReprojectedMeasurement()).cast<float>();
    Isometry3D measuredTo = (from->estimate() * Eigen::Translation3d(edge->getReprojectedMeasurement()));
    if (! from || ! to)
      return this;

    glPushAttrib(GL_ENABLE_BIT);
    glPushAttrib(GL_LINE_BIT);

    glDisable(GL_LIGHTING);
    if(_showEdge && _showEdge->value()){
      glLineWidth(EDGE_LINE_WIDTH);
      glBegin(GL_LINES);
      glColor3f(POSE_EDGE_COLOR);
      glVertex3f(fromPos.x(),fromPos.y(),fromPos.z());
      glVertex3f(estToPos.x(),estToPos.y(),estToPos.z());
      glEnd();
      glPopAttrib();//restore Line width
    }

    if(_showMeasurementAndError && _showMeasurementAndError->value()){
      drawMeasurementAndError(fromPos, estToPos, measToPos);
    }


    if(_showEllipsoid && _showEllipsoid->value()){
      auto infoMatLvalue = edge->getReprojectedInformation();
      drawUncertainty(measuredTo, infoMatLvalue);
    }

    glPopAttrib();//restore enable bit (lighting?)
    return this;
  }

#endif

}
