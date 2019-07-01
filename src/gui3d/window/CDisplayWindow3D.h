#pragma once

#include <gui3d/render/scene_manager.h>
#include <gui3d/window/CGlCanvas.h>
#include <gui3d/window/CDisplayImages.h>
#include <string>
#include <thread>

class GLFWwindow;
class ImGuiContext;

namespace gui3d {

//!< Type for the callback function used in pushRenderCallback
typedef void (* TCallbackRender) (void* userParam);
struct HookFunc {
  TCallbackRender userFunction;
  void* userParam = nullptr;
  void run() {
    if (userFunction)
      userFunction(userParam);
  }
};

class CDisplayWindow3D;
using CDisplayWindow3DPtr = std::shared_ptr<CDisplayWindow3D>;

class CDisplayWindow3D : public SceneManager {
 public:
 /** Class factory returning a smart pointer */
  static CDisplayWindow3DPtr Create(
    const std::string	&windowCaption,
    unsigned int		initialWindowWidth = 1080,
    unsigned int		initialWindowHeight = 720 );

 public:
  CDisplayWindow3D(
    const std::string	&windowCaption = "Arcsoft3d",
    unsigned int		initialWindowWidth = 1080,
    unsigned int		initialWindowHeight = 720 );

  virtual ~CDisplayWindow3D();

  bool WindowClosed() const;
  CDisplayImagesPtr createViewImage(const std::string& name);
  CDisplayImagesPtr getViewImage() { return m_subview_image; }

    /** Must be called to have a callback
    *   when the user selects one of the user-defined entries
    */
  void pushRenderCallBack(TCallbackRender userFunction, void* userParam = NULL);

 private:
  void forceRepaint(); //!< Repaints the window. forceRepaint, repaint and updateWindow are all aliases of the same method
  void OnPreRender();
  void OnPostRender();
  void OnEyeShotRender();    // handle eye shot from mouse wheel
  void OnImGuiRender();
  void loadSceneFrom(const char* fileName);
  void backThreadRun();
  void RunOnce();

  std::string                           m_windowCaption;
  int                                   m_initialWindowWidth;
  int                                   m_initialWindowHeight;
  volatile bool                         m_ReadyContext = false;

  CDisplayImagesPtr                     m_subview_image = nullptr;
  std::vector<HookFunc>                 m_hookFuncs;

  bool                                   RequestToRefresh3DView;
  float                                  m_lastWheelRotation;
  std::thread                            m_renderLoopThread;
  GLFWwindow*                            m_Window;
  ImGuiContext*                          m_ImGuiContext;
  CGlCanvas*                             m_GlCanvas;  //!< Internal Mouse View object
};

} // namespace gui3d
