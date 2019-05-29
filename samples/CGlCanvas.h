#pragma once

struct ImGuiIO;
#include <mrpt/opengl/COpenGLScene.h>

class MouseEvent{
 public:
  float GetX() const { return m_x;}
  float GetY() const { return m_y;}

  // accessors for the pressed buttons
  bool LeftIsDown()    const { return m_leftDown; }
  bool RightIsDown()   const { return m_rightDown; }

  // accessors for individual modifier keys
  bool ControlDown() const { return m_controlDown; }
  bool ShiftDown() const { return m_shiftDown; }

  // Get wheel rotation, positive or negative indicates direction of
  // rotation.  Current devices all send an event when rotation is equal to
  // +/-WheelDelta, but this allows for finer resolution devices to be
  // created in the future.  Because of this you shouldn't assume that one
  // event is equal to 1 line or whatever, but you should be able to either
  // do partial line scrolling or wait until +/-WheelDelta rotation values
  // have been accumulated before scrolling.
  float GetWheelRotation() const { return m_wheelRotation; }

  void setFromIO(const ImGuiIO& io);

 private:
  bool m_leftDown    =  false;
  bool m_rightDown   =  false;
  bool m_controlDown =  false;
  bool m_shiftDown   =  false;

  float m_wheelRotation = 0;
  float m_x, m_y;
};

/** This class implements a OpenGL canvas,
 *   and it's used in gui3d and a number of standalone applications in the project.
  *  There is a filter to control the user capability of moving the camera with the mouse.
  *  See OnUserManuallyMovesCamera
  */
class CGlCanvas
{
public:
  explicit CGlCanvas(mrpt::opengl::COpenGLScenePtr parent);
  virtual ~CGlCanvas();

  void OnPaint();
  void OnMouseDown(MouseEvent& event);
  void OnMouseMove(MouseEvent& event);
  void OnMouseUp(MouseEvent& event);
  void OnMouseWheel(MouseEvent& event);
  void Render();

  // Visualization params:
  float	cameraPointingX,cameraPointingY,cameraPointingZ;
  float	cameraZoomDistance;
  float	cameraElevationDeg,cameraAzimuthDeg;
  bool	cameraIsProjective;
  float   cameraFOV;

  /** If set to true (default=false), the cameraPointingX,... parameters are ignored and the camera stored in the 3D scene is used instead.
    */
  bool    useCameraFromScene;

  /** Set the camera from a CPose3D, which defines the +X,+Y axis as image place RIGHT and UP dirctions, and -Z as towards the pointing direction.
    */
  //void setCameraPose(const mrpt::poses::CPose3D &camPose);

  static float  SENSIBILITY_DEG_PER_PIXEL;		// Default = 0.1

  /** Overload this method to limit the capabilities of the user to move the camera using the mouse.
    *  For all these variables:
    *  - cameraPointingX
    *  - cameraPointingY
    *  - cameraPointingZ
    *  - cameraZoomDistance
    *  - cameraElevationDeg
    *  - cameraAzimuthDeg
    *
    *  A "new_NAME" variable will be passed with the temptative new value after the user action.
    *   The default behavior should be to copy all the new variables to the variables listed above
    *   but in the middle any find of user-defined filter can be implemented.
    */
  virtual void OnUserManuallyMovesCamera(
    float	new_cameraPointingX,
    float 	new_cameraPointingY,
    float 	new_cameraPointingZ,
    float	new_cameraZoomDistance,
    float	new_cameraElevationDeg,
    float	new_cameraAzimuthDeg ) {
    cameraPointingX 	= new_cameraPointingX;
    cameraPointingY 	= new_cameraPointingY;
    cameraPointingZ 	= new_cameraPointingZ;
    cameraZoomDistance 	= new_cameraZoomDistance;
    cameraElevationDeg 	= new_cameraElevationDeg ;
    cameraAzimuthDeg 	= new_cameraAzimuthDeg;
  }

  inline void getLastMousePosition(int &x,int& y) const {
    x =m_mouseLastX;
    y =m_mouseLastY;
  }

  /**  At constructor an empty scene is created. The object is freed at GL canvas destructor.
    */
  mrpt::opengl::COpenGLScenePtr		m_openGLScene;

protected:
  bool   m_init;

  int 	m_mouseLastX,m_mouseLastY;

  int 	mouseClickX,mouseClickY;
  bool 	mouseClicked;

  long           m_Key;
  unsigned long  m_StartTime;
  unsigned long  m_LastTime;
  unsigned long  m_LastRedraw;
};

