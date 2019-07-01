#include <gui3d/window/CDisplayImages.h>
#include <Third-party/imgui/imgui.h>
#include <GL/gl.h>

namespace gui3d {

CDisplayImages::CDisplayImages(const std::string& name)
  : m_name(name) {
}

CDisplayImages::~CDisplayImages() {
  if (texId)
    glDeleteTextures(1, &texId);
}

void CDisplayImages::setImageView(const cv::Mat& im) {
  m_image = im;

  if (texId == 0) {
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glPixelStoref(GL_UNPACK_ALIGNMENT, 1); // default is 4
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, im.cols, im.rows, 0,
                 GL_BGR_EXT, GL_UNSIGNED_BYTE, im.data);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_view_width = im.cols;
    m_view_height = im.rows;
  }
}

void CDisplayImages::render() {
  const float BX = 16;
  const float BY = 36;
  const float ZoomOfImage = 1.0;

  if (!ImGui::Begin(m_name.c_str(),
                    nullptr,
                    ImVec2(m_view_width + BX, m_view_height + BY),
                    0.5f)) {
    ImGui::End();
    return;
  }

  if (!m_image.empty()) {
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_image.cols, m_image.rows,
                    GL_BGR_EXT, GL_UNSIGNED_BYTE, m_image.data);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  ImVec2 showSize(ImGui::GetWindowWidth() - BX, ImGui::GetWindowHeight() - BY);
  ImGui::Image((void*) texId, showSize);
  if (ImGui::BeginPopupContextWindow()) {
    if (ImGui::Button("Reset Size")) {
      ImGui::SetWindowSize(m_name.c_str(),
                           ImVec2(m_image.cols * ZoomOfImage + BX,
                                  m_image.rows * ZoomOfImage + BY));
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  ImGui::End();
}

} // namespace gui3d
