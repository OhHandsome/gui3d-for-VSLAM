#pragma once

#include <gui3d/utils/dependency.hpp>
#include <gui3d/base/type_def.h>

using Eigen::Vector3f;
using Eigen::Matrix3f;
using Eigen::Matrix4f;

namespace gui3d {

CImage castImage(const cv::Mat& im);
CImage castImage_clone(const cv::Mat& im);

// math
// Input Pose is opengl matrix
Vector3f pickTranslation(const Pose& Twc); // memory stored by rows or cols
Matrix3f pickRotation(const Pose& Twc);
Matrix4f pickPose(const Pose& Twc);

Pose toPose44(const State& pose31);
CPose3D castPose(const Pose& Tnb);
CPose3D castPose(const State& state);
CPose3D castCamera(const Pose& Twc); // Adopt wxWidget's model

TPoint3D castPoint(const LandMark3d& point);
void castLines(const LandMark3dV& vvPoints, std::vector<TSegment3D>& vLines);
TColorf castColor(const Color& color);

template<class T>
T normilizeAngle(T angle) {
  if (angle > M_PI)
    angle -= 2 * T(M_PI);
  else if (angle <= -M_PI)
    angle += 2 * T(M_PI);
  return angle;
}

template<typename _Scalar>
using Mat44 = Eigen::Matrix<_Scalar, 4, 4>;
template<typename _Scalar>
using Mat33 = Eigen::Matrix<_Scalar, 3, 3>;
template<typename _Scalar>
using Vec3 = Eigen::Matrix<_Scalar, 3, 1>;

template<typename _Scalar>
inline Vec3<_Scalar> toSE2(const Mat44<_Scalar>& T) {
  Mat33<_Scalar> R = T.template block<3, 3>(0, 0);
  Eigen::AngleAxis<_Scalar> rotVec(R);
  Vec3<_Scalar> pose31(T(0, 3), T(1, 3), 0);
  pose31(2) = rotVec.axis()(2) * rotVec.angle();
  pose31(2) = normilizeAngle(pose31(2));

  return pose31;
}


} // namespace gui3d

