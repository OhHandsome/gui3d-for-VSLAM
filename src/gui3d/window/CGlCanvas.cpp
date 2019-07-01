#include <gui3d/window/CGlCanvas.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

using namespace mrpt;
using namespace mrpt::opengl;
using namespace std;

namespace gui3d {
float  CGlCanvas::SENSIBILITY_DEG_PER_PIXEL = 0.1f;

void CGlCanvas::OnMouseDown(MouseEvent& event)
{
  mouseClickX = (int) event.GetX();
  mouseClickY = (int) event.GetY();
  mouseClicked = true;
}

void CGlCanvas::OnMouseUp(MouseEvent& event)
{
  mouseClicked = false;
}

void CGlCanvas::OnMouseMove(MouseEvent& event)
{
  int X = m_mouseLastX = event.GetX();
  int Y = m_mouseLastY = event.GetY();

  // Proxy variables to cache the changes:
  float	cameraPointingX = this->cameraPointingX;
  float	cameraPointingY = this->cameraPointingY;
  float 	cameraPointingZ = this->cameraPointingZ;
  float	cameraZoomDistance = this->cameraZoomDistance;
  float	cameraElevationDeg = this->cameraElevationDeg;
  float 	cameraAzimuthDeg = this->cameraAzimuthDeg;

  if (!mouseClicked)  return;
  if (event.LeftIsDown() )
  {
    if ( event.ShiftDown()  )
    {
      // Zoom:
      cameraZoomDistance *= exp(0.01*(Y - mouseClickY));
      if (cameraZoomDistance<0.01) cameraZoomDistance = 0.01f;

      float	Az = -0.05*(X - mouseClickX);
      float	D = 0.001*cameraZoomDistance;
      cameraPointingZ += D*Az;
    }
    else
    if (event.ControlDown())
    {
      // Rotate camera pointing direction:
      const float dis = max(0.01f,(cameraZoomDistance));
      float	eye_x = cameraPointingX +  dis * cos(DEG2RAD(cameraAzimuthDeg))*cos(DEG2RAD(cameraElevationDeg));
      float	eye_y = cameraPointingY +  dis * sin(DEG2RAD(cameraAzimuthDeg))*cos(DEG2RAD(cameraElevationDeg));
      float	eye_z = cameraPointingZ +  dis * sin(DEG2RAD(cameraElevationDeg));

      float A_AzimuthDeg = -SENSIBILITY_DEG_PER_PIXEL*(X - mouseClickX);
      float A_ElevationDeg = SENSIBILITY_DEG_PER_PIXEL*(Y - mouseClickY);

      // Orbit camera:
      cameraAzimuthDeg += A_AzimuthDeg;
      cameraElevationDeg += A_ElevationDeg;
      if (cameraElevationDeg<-90) cameraElevationDeg = -90;
      if (cameraElevationDeg>90) cameraElevationDeg = 90;

      // Move cameraPointing pos:
      cameraPointingX = eye_x - dis * cos(DEG2RAD(cameraAzimuthDeg))*cos(DEG2RAD(cameraElevationDeg));
      cameraPointingY = eye_y - dis * sin(DEG2RAD(cameraAzimuthDeg))*cos(DEG2RAD(cameraElevationDeg));
      cameraPointingZ = eye_z - dis * sin(DEG2RAD(cameraElevationDeg));
    }
    else
    {
      // Orbit camera:
      cameraAzimuthDeg -= 0.2*(X - mouseClickX);
      cameraElevationDeg += 0.2*(Y - mouseClickY);
      if (cameraElevationDeg<-90) cameraElevationDeg = -90;
      if (cameraElevationDeg>90) cameraElevationDeg = 90;
    }

    mouseClickX = X;
    mouseClickY = Y;

    // Potential user filter:
    OnUserManuallyMovesCamera(cameraPointingX, cameraPointingY, cameraPointingZ, cameraZoomDistance, cameraElevationDeg, cameraAzimuthDeg);
  }
  else
  if ( event.RightIsDown() )
  {
    float	Ay = -(X - mouseClickX);
    float	Ax = -(Y - mouseClickY);
    float	D = 0.001*cameraZoomDistance;
    cameraPointingX += D*(Ax*cos(DEG2RAD(cameraAzimuthDeg)) - Ay*sin(DEG2RAD(cameraAzimuthDeg)));
    cameraPointingY += D*(Ax*sin(DEG2RAD(cameraAzimuthDeg)) + Ay*cos(DEG2RAD(cameraAzimuthDeg)));

    mouseClickX = X;
    mouseClickY = Y;

    // Potential user filter:
    OnUserManuallyMovesCamera(cameraPointingX, cameraPointingY, cameraPointingZ, cameraZoomDistance, cameraElevationDeg, cameraAzimuthDeg);
  }
}

void CGlCanvas::OnMouseWheel(MouseEvent& event)
{
  float cameraZoomDistance = this->cameraZoomDistance;

  cameraZoomDistance *= 1 - 0.03f*(event.GetWheelRotation());

  // Potential user filter:
  OnUserManuallyMovesCamera(cameraPointingX, cameraPointingY, cameraPointingZ, cameraZoomDistance, cameraElevationDeg, cameraAzimuthDeg);
}

CGlCanvas::CGlCanvas(mrpt::opengl::COpenGLScene::Ptr parent) :
  m_init(false),
  m_mouseLastX(0),
  m_mouseLastY(0),
  m_openGLScene(parent)
{
  mouseClickX=0;
  mouseClickY=0;
  mouseClicked = false;

  // Initialize variables:
  cameraPointingX 	= 0;
  cameraPointingY 	= 0;
  cameraPointingZ 	= 0;

  cameraIsProjective = true;

  useCameraFromScene	= false;

  cameraZoomDistance	= 5;
  cameraFOV           = 30;
  cameraElevationDeg 	= 45;
  cameraAzimuthDeg	= 45;
}

CGlCanvas::~CGlCanvas()
{
  m_openGLScene.reset();
}

void CGlCanvas::Render()
{
  static bool GLUT_INIT_DONE = false;
  if (!GLUT_INIT_DONE)
  {
    GLUT_INIT_DONE = true;

    int argc=1;
    char *argv[1] = { NULL };
    glutInit( &argc, argv );
  }

  try
  {
    // Call PreRender user code:
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Set static configs:
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);

    // PART 1a: Set the viewport
    // --------------------------------------
    if (m_openGLScene)
    {
      // Set the camera params in the scene:
      if (!useCameraFromScene)
      {
        COpenGLViewport::Ptr view= m_openGLScene->getViewport("main");
        if (!view)
        {
          THROW_EXCEPTION("Fatal error: there is no 'main' viewport in the 3D scene!");
        }

        mrpt::opengl::CCamera & cam = view->getCamera();
        cam.setPointingAt( cameraPointingX, cameraPointingY, cameraPointingZ );
        cam.setZoomDistance(cameraZoomDistance);
        cam.setAzimuthDegrees( cameraAzimuthDeg );
        cam.setElevationDegrees(cameraElevationDeg);
        cam.setProjectiveModel( cameraIsProjective );
        cam.setProjectiveFOVdeg( cameraFOV );
        //std::cout << cam.getPose() << std::endl;
        //std::cout << cameraZoomDistance << std::endl;
      }

      // PART 2: Set the MODELVIEW matrix
      // --------------------------------------
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      // PART 3: Draw primitives:
      // --------------------------------------
      m_openGLScene->render();

    } // end if "m_openGLScene!=NULL"

    // Flush & swap buffers to disply new image:
    glFlush();
    glPopAttrib();
  }
  catch (std::exception &e)
  {
    glPopAttrib();
    const std::string err_msg = std::string("[CGlCanvas::Render] Exception!: ") +std::string(e.what());
    std::cerr << err_msg;
  }
  catch (...)
  {
    glPopAttrib();
    std::cerr << "Runtime error!" << std::endl;
  }
}

void CGlCanvas::OnPaint()
{
  Render();
}

} // namespace gui3d
