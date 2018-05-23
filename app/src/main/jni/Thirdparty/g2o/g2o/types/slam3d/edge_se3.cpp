#include "edge_se3.h"
#include "isometry3d_gradients.h"
#include <iostream>

#ifdef G2O_HAVE_OPENGL
#include "g2o/stuff/opengl_wrapper.h"
#include "g2o/stuff/opengl_primitives.h"
#endif

namespace g2o {
  using namespace std;
  using namespace Eigen;

  EdgeSE3::EdgeSE3() : BaseBinaryEdge<6, Isometry3D, VertexSE3, VertexSE3>() {
    information().setIdentity();
  }

  bool EdgeSE3::read(std::istream& is) {
    Vector7d meas;
    for (int i=0; i<7; i++) 
      is >> meas[i];
    // normalize the quaternion to recover numerical precision lost by storing as human readable text
    Vector4D::MapType(meas.data()+3).normalize();
    setMeasurement(internal::fromVectorQT(meas));

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
      //  we overwrite the information matrix with the Identity
      information().setIdentity();
    } 
    return true;
  }

  bool EdgeSE3::write(std::ostream& os) const {
    Vector7d meas=internal::toVectorQT(_measurement);
    for (int i=0; i<7; i++) os  << meas[i] << " ";
    for (int i=0; i<information().rows(); i++)
      for (int j=i; j<information().cols(); j++) {
        os <<  information()(i,j) << " ";
      }
    return os.good();
  }

  void EdgeSE3::computeError() {
    VertexSE3 *from = static_cast<VertexSE3*>(_vertices[0]);
    VertexSE3 *to   = static_cast<VertexSE3*>(_vertices[1]);
    Isometry3D delta=_inverseMeasurement * from->estimate().inverse() * to->estimate();
    _error=internal::toVectorMQT(delta);
  }

  bool EdgeSE3::setMeasurementFromState(){
    VertexSE3 *from = static_cast<VertexSE3*>(_vertices[0]);
    VertexSE3 *to   = static_cast<VertexSE3*>(_vertices[1]);
    Isometry3D delta = from->estimate().inverse() * to->estimate();
    setMeasurement(delta);
    return true;
  }
  
  void EdgeSE3::linearizeOplus(){
    
    // BaseBinaryEdge<6, Isometry3D, VertexSE3, VertexSE3>::linearizeOplus();
    // return;

    VertexSE3 *from = static_cast<VertexSE3*>(_vertices[0]);
    VertexSE3 *to   = static_cast<VertexSE3*>(_vertices[1]);
    Isometry3D E;
    const Isometry3D& Xi=from->estimate();
    const Isometry3D& Xj=to->estimate();
    const Isometry3D& Z=_measurement;
    internal::computeEdgeSE3Gradient(E, _jacobianOplusXi , _jacobianOplusXj, Z, Xi, Xj);
  }

  void EdgeSE3::initialEstimate(const OptimizableGraph::VertexSet& from_, OptimizableGraph::Vertex* /*to_*/) {
    VertexSE3 *from = static_cast<VertexSE3*>(_vertices[0]);
    VertexSE3 *to   = static_cast<VertexSE3*>(_vertices[1]);

    if (from_.count(from) > 0) {
      to->setEstimate(from->estimate() * _measurement);
    } else {
      from->setEstimate(to->estimate() * _measurement.inverse());
    } 
  }

  EdgeSE3WriteGnuplotAction::EdgeSE3WriteGnuplotAction(): WriteGnuplotAction(typeid(EdgeSE3).name()){}

  HyperGraphElementAction* EdgeSE3WriteGnuplotAction::operator()(HyperGraph::HyperGraphElement* element, HyperGraphElementAction::Parameters* params_){
    if (typeid(*element).name()!=_typeName)
      return 0;
    WriteGnuplotAction::Parameters* params=static_cast<WriteGnuplotAction::Parameters*>(params_);
    if (!params->os){
      std::cerr << __PRETTY_FUNCTION__ << ": warning, on valid os specified" << std::endl;
      return 0;
    }

    EdgeSE3* e =  static_cast<EdgeSE3*>(element);
    VertexSE3* fromEdge = static_cast<VertexSE3*>(e->vertices()[0]);
    VertexSE3* toEdge   = static_cast<VertexSE3*>(e->vertices()[1]);
    Vector6d fromV, toV;
    fromV=internal::toVectorMQT(fromEdge->estimate());
    toV=internal::toVectorMQT(toEdge->estimate());
    for (int i=0; i<6; i++){
      *(params->os) << fromV[i] << " ";
    }
    for (int i=0; i<6; i++){
      *(params->os) << toV[i] << " ";
    }
    *(params->os) << std::endl;
    return this;
  }

#ifdef G2O_HAVE_OPENGL

  bool EdgeSE3DrawAction::refreshPropertyPtrs(HyperGraphElementAction::Parameters* params_){
    if (!DrawAction::refreshPropertyPtrs(params_))
      return false;
    if (_previousParams){
      _showMeasurementAndError = _previousParams->makeProperty<BoolProperty>(_typeName + "::SHOW_MEASUREMENT_AND_ERROR", false);
      _showEllipsoid = _previousParams->makeProperty<BoolProperty>(_typeName + "::SHOW_STD_DEV", false);
    } else {
      _showMeasurementAndError = 0;
      _showEllipsoid = 0;
    }
    return true;
  }

  EdgeSE3DrawAction::EdgeSE3DrawAction(): DrawAction(typeid(EdgeSE3).name()){}

  void  EdgeSE3DrawAction::drawMeasurementAndError(Eigen::Vector3f& fromPos,
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

  void EdgeSE3DrawAction::drawUncertainty(Isometry3D& measuredTo, EdgeSE3::InformationType& infoMat) {
      //Draw uncertainty ellipsoid for one std dev
      glColor3f(EDGE_UNCERTAINTY_ELLIPSOID_COLOR);
      glPushMatrix();
      glPushAttrib(GL_POLYGON_BIT);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//Draw wireframe
      glMultMatrixd(measuredTo.matrix().data());
      EdgeSE3::InformationType cov = infoMat.inverse();
      opengl::drawEllipsoid(3*sqrt(cov(0,0)), 3*sqrt(cov(1,1)), 3*sqrt(cov(2,2)));
      glPopAttrib();//Restore from wireframe
      glPopMatrix();
  }


  HyperGraphElementAction* EdgeSE3DrawAction::operator()(HyperGraph::HyperGraphElement* element, 
               HyperGraphElementAction::Parameters* params_){
    if (typeid(*element).name()!=_typeName)
      return 0;
    refreshPropertyPtrs(params_);
    if (! _previousParams)
      return this;
    
    if (_show && !_show->value())
      return this;
    
    auto* edge = static_cast<EdgeSE3*>(element);
    auto* from = static_cast<VertexSE3*>(edge->vertices()[0]);
    auto* to   = static_cast<VertexSE3*>(edge->vertices()[1]);
    Eigen::Vector3f fromPos = from->estimate().translation().cast<float>();
    Eigen::Vector3f estToPos = to->estimate().translation().cast<float>();
    Isometry3D measuredTo = (from->estimate() * edge->measurement());
    Eigen::Vector3f measToPos = measuredTo.translation().cast<float>();
    if (! from || ! to)
      return this;

    glPushAttrib(GL_ENABLE_BIT);
    glPushAttrib(GL_LINE_BIT);

    glDisable(GL_LIGHTING);
    glLineWidth(EDGE_LINE_WIDTH);
    glBegin(GL_LINES);
    glColor3f(POSE_EDGE_COLOR);
    glVertex3f(fromPos.x(),fromPos.y(),fromPos.z());
    glVertex3f(estToPos.x(),estToPos.y(),estToPos.z());
    glEnd();

    if(_showMeasurementAndError && _showMeasurementAndError->value()){
      drawMeasurementAndError(fromPos, estToPos, measToPos);
    }


    if(_showEllipsoid && _showEllipsoid->value()){
      drawUncertainty(measuredTo, edge->information());
    }

    glPopAttrib();//restore Line width
    glPopAttrib();//restore enable bit (lighting?)
    return this;
  }

#endif

}
