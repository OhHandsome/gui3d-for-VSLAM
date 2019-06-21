#pragma once

#include <opencv2/core.hpp>

namespace gui3d {

class CDisplayImages {

public:
  CDisplayImages(const std::string& name)
      : m_name(name) {}

  void setImageView_fast(const cv::Mat& im) {
    m_image = im;
  }

  const std::string& getName() {
    return m_name;
  }

  void setViewPosition(
      const double x, const double y,
      const double width, const double height) {
    m_view_x = x;
    m_view_y = y;
    m_view_width = width;
    m_view_height = height;
  }

  void getViewPosition(
      double& x, double& y,
      double& width, double& height) {
    x = m_view_x;
    y = m_view_y;
    width = m_view_width;
    height = m_view_height;
  }

protected:
  std::string m_name;
  double m_view_x, m_view_y, m_view_width, m_view_height;
  cv::Mat m_image;

  friend class CDisplayWindow3D;
};

using CDisplayImagesPtr = std::shared_ptr<CDisplayImages>;
} // namespace gui3d

