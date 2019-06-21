#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <iomanip>
#include <cmath>
#include <sstream>

#include <Eigen/Core>

namespace gui3d {

typedef void* hObject;       // 句柄, 指向所有的图形对象
using std::vector;
using std::string;
using Channel = std::string;

typedef Eigen::Matrix4f                                            Pose;
typedef Eigen::Vector3f                                            LandMark3d;
typedef Eigen::Vector3f                                            Position3d;
typedef Eigen::Vector2i                                            KeyPoint;
typedef Eigen::Matrix<float, 7, 1>                                 ColourLandMark3d;
typedef Eigen::Vector4i                                            Color;
typedef int                                                        Name; // FrameID

typedef std::vector<Name>                                          NameV;
typedef std::vector<Pose, Eigen::aligned_allocator<Pose> >         PoseV;
typedef std::vector<LandMark3d >                                   LandMark3dV;
typedef std::vector<Position3d >                                   Position3dV;
typedef std::vector<ColourLandMark3d >                             PointCloud;
typedef std::vector<KeyPoint, Eigen::aligned_allocator<KeyPoint> > KeyPointV;
typedef std::vector<Color >                                        Color4V;
typedef std::vector<PointCloud >                                   PointCloudV;

typedef Eigen::Vector3d                                            State;
typedef std::vector<State >                                        StateV;

// data type cast
template<class T, int rows, int cols>
std::vector<Eigen::Matrix<float, rows, cols>>
cast(const std::vector<Eigen::Matrix<T, rows, cols>> &d) {
  std::vector<Eigen::Matrix<float, rows, cols>> res;
  res.reserve(d.size());
  for (auto &i : d) {
    res.push_back(i.cast<float>());
  }
  return res;
}

} // namespace gui3d


