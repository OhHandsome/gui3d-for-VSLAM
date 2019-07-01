#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <memory>
#include <Third-party/imgui/imgui.h>
#include <GL/gl.h>

namespace gui3d {

class CDisplayImages {

 public:
  CDisplayImages(const std::string& name);
  ~CDisplayImages();

  void setImageView(const cv::Mat& im);
  void render();

 protected:
  std::string m_name;
  double m_view_width, m_view_height;
  cv::Mat m_image;
  GLuint texId = 0;
};
using CDisplayImagesPtr = std::shared_ptr<CDisplayImages>;

} // namespace gui3d

