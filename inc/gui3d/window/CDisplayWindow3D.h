#pragma once
#include <mrpt/opengl/COpenGLScene.h>
#include <mrpt/opengl/COpenGLViewport.h>
#include <mrpt/opengl/CAxis.h>
#include <mrpt/opengl/CGridPlaneXY.h>

#include <gui3d/render/options.hpp>
#include <gui3d/window/CGlCanvas.h>
#include <gui3d/window/CDisplayImages.h>
#include <string>
#include <mutex>
#include <thread>

class GLFWwindow;
class ImGuiContext;

namespace gui3d{

class CDisplayWindow3D;
using CDisplayWindow3DPtr = std::shared_ptr<CDisplayWindow3D>;

//!< Type for the callback function used in pushRenderCallback
typedef void (* TCallbackRender) (void* userParam);

class CDisplayWindow3D {
 public:
  /** Constructor */
  CDisplayWindow3D(
    const std::string	&windowCaption = "Arcsoft3d",
    unsigned int		initialWindowWidth = 1080,
    unsigned int		initialWindowHeight = 720 );

  /** Class factory returning a smart pointer */
  static CDisplayWindow3DPtr Create(
    const std::string	&windowCaption,
    unsigned int		initialWindowWidth = 1080,
    unsigned int		initialWindowHeight = 720 );

  /** Destructor */
  virtual ~CDisplayWindow3D();

  /** Gets a reference to the smart shared pointer that holds the internal scene (carefuly read introduction in gui::CDisplayWindow3D before use!)
    *  This also locks the critical section for accesing the scene, thus the window will not be repainted until it is unlocked.
    * \note It is safer to use mrpt::gui::CDisplayWindow3DLocker instead.*/
  mrpt::opengl::COpenGLScenePtr& get3DSceneAndLock();

  /** Unlocks the access to the internal 3D scene. It is safer to use mrpt::gui::CDisplayWindow3DLocker instead.
    *  Typically user will want to call forceRepaint after updating the scene. */
  void unlockAccess3DScene();
  void resize(int width, int height) {}
  void setPos(int x, int y) {}
  void repaint() { forceRepaint(); }
  void addTextMessage(int , int , int, int , int ) {}
  volatile Gui3dOption& Options() { return m_Observer; }
  bool WindowClosed() const;
  void InitScene();
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

  struct HookFunc{
      TCallbackRender userFunction;
      void* userParam = nullptr;
  };

  std::string                           m_windowCaption;
  int                                   m_initialWindowWidth;
  int                                   m_initialWindowHeight;
  volatile bool                         m_ReadyContext = false;

  CDisplayImagesPtr                     m_subview_image = nullptr;
  mrpt::opengl::CAxisPtr                m_Axis3d;
  mrpt::opengl::CGridPlaneXYPtr         m_ZeroPlane;
  std::vector<HookFunc>                 m_hookFuncs;

  volatile Gui3dOption                   m_Observer;

  bool                                   RequestToRefresh3DView;
  float                                  m_lastWheelRotation;
  std::mutex                             m_access3Dscene;
  std::thread                            m_renderLoopThread;
  GLFWwindow*                            m_Window;
  ImGuiContext*                          m_ImGuiContext;
  mrpt::opengl::COpenGLScenePtr          m_3Dscene;   //!< Internal OpenGL object (see general discussion in about usage of this object)
  CGlCanvas*                             m_GlCanvas;  //!< Internal Mouse View object
};

} // namespace gui3d
