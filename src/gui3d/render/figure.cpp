#include <gui3d/utils/dependency.hpp>
#include <gui3d/render/figure.h>
#include <gui3d/render/model_render.h>
#include <gui3d/render/observer.h>
#include <gui3d/render/style.h>
#if HAS_IMGUI
#include <gui3d/window/CDisplayWindow3D.h>
#endif
namespace gui3d{
template <class T>
void removeObject(COpenGLViewport::Ptr mainVP, std::map<Channel, T>& m)
{
  for (auto item : m) {
    if (item.second)
      mainVP->removeObject(item.second);
  }
  m.clear();
}

int Figure::mNextID = 1;
Figure::Figure(const string &name, int width, int height)
{
#if HAS_IMGUI == 1
  // Create 3D Windows
  gui3d::CDisplayWindow3DPtr win = gui3d::CDisplayWindow3DPtr(new CDisplayWindow3D(name, width, height));
  mMainWindow = win;
  this->name = name;
  mFigureID = mNextID++;
#else
  // Create 3D Windows
  mrpt::gui::CDisplayWindow3D::Ptr win = mrpt::gui::CDisplayWindow3D::Ptr(new mrpt::gui::CDisplayWindow3D(name, width, height));
  mMainWindow = win;
  this->name = name;
  mFigureID = mNextID++;
  init();

  win->setCameraAzimuthDeg(-53);
  win->setCameraElevationDeg(32);
  win->setCameraZoom(12);

  // Create an instance of the observer:
  mObserver = std::make_shared<GuiObserver>(this);

  // and subscribe to window events,
  mObserver->observeBegin(*win);
  // and opengl viewport events:
  {
    COpenGLScene::Ptr &theScene = win->get3DSceneAndLock();
    COpenGLViewport::Ptr the_main_view = theScene->getViewport("main");
    mObserver->observeBegin(*the_main_view);
    win->unlockAccess3DScene();
  }
  win->repaint();
#endif
}

void Figure::init()
{
#if HAS_IMGUI == 1
  mMainWindow->InitScene();
#else
  auto &bAxis3d  = mOption.figOpt.bAxis3d;
  auto &bPlaneXY = mOption.figOpt.bPlaneXY;
  //  Get the smart pointer to the main viewport object in this mMainWindow:
  {
    COpenGLScene::Ptr &theScene = mMainWindow->get3DSceneAndLock();
    CAxis::Ptr Axis = CAxis::Create(-AXISLength, -AXISLength, -AXISLength,
                                  AXISLength, AXISLength /* / 2.0 + 1*/, AXISLength /* / 2.0 + 1*/, 4, 2, true);
    Axis->setLocation(0, 0, 0);
//        Axis->setTextScale(0.1f);
    Axis->setName("CXYZ");
    Axis->enableTickMarks();
    theScene->insert(Axis);
    mAxis3d = Axis;
    VisiableModel(mAxis3d, bAxis3d);

    auto XY = CGridPlaneXY::Create(-AXISLength, AXISLength, -AXISLength, AXISLength);
    XY->setName("CXY");
    theScene->insert(XY);
    mGridXY = XY;
    VisiableModel(mGridXY, bPlaneXY);

    mScene = theScene;

    // IMPORTANT!!! IF NOT UNLOCKED, THE WINDOW WILL NOT BE UPDATED!
    mMainWindow->unlockAccess3DScene();
  }
#endif
}

void Figure::clear()
{
  if (!mMainWindow)
    return;

  mScene = mMainWindow->get3DSceneAndLock();
  if (!mGLViewImage) mGLViewImage.reset();
  if (!mGLSubView)   mGLSubView.reset();

  if (mScene)
  {
    COpenGLViewport::Ptr mainVP = mScene->getViewport();
    if (mainVP)
    {
      removeObject(mainVP, mSysFrame);
      removeObject(mainVP, mSysRobot);
      removeObject(mainVP, mSysPoseList);
      removeObject(mainVP, mSysMapPoint);
      removeObject(mainVP, mSysLine);
      removeObject(mainVP, mSysModel3d);
    }
  }

  mMainWindow->unlockAccess3DScene();
}

volatile Gui3dOption& Figure::Options()
{
#if HAS_IMGUI
  return mMainWindow->Options();
#else
  return mOption;
#endif
}


// Find Channel's hObject
CFrustum::Ptr Figure::hFrame(const Channel& name)
{
  CFrustum::Ptr obj;
  auto it = mSysFrame.find(name);
  if(it != mSysFrame.end())  obj = it->second;
  return obj;
}

CSetOfObjects::Ptr Figure::hRobot(const Channel& name)
{
  CSetOfObjects::Ptr obj;
  auto it = mSysRobot.find(name);
  if(it != mSysRobot.end())  obj = it->second;
  return obj;
}

CSetOfObjects::Ptr Figure::hPoseList(const Channel& name)
{
  CSetOfObjects::Ptr obj;
  auto it = mSysPoseList.find(name);
  if(it != mSysPoseList.end())  obj = it->second;
  return obj;
}

CPointCloud::Ptr Figure::hMapPoint(const Channel& name)
{
  CPointCloud::Ptr obj;
  auto it = mSysMapPoint.find(name);
  if(it != mSysMapPoint.end())  obj = it->second;
  return obj;
}

CPointCloudColoured::Ptr Figure::hPointCloud(const Channel& name)
{
  CPointCloudColoured::Ptr obj;
  auto it = mSysPointCloud.find(name);
  if(it != mSysPointCloud.end())  obj = it->second;
  return obj;
}

CSetOfLines::Ptr Figure::hLine(const Channel& name)
{
  CSetOfLines::Ptr obj;
  auto it = mSysLine.find(name);
  if(it != mSysLine.end())  obj = it->second;
  return obj;
}

CSetOfObjects::Ptr Figure::hModel3d(const Channel& name)
{
  CSetOfObjects::Ptr obj;
  auto it = mSysModel3d.find(name);
  if(it != mSysModel3d.end())  obj = it->second;
  return obj;
}

CSetOfObjects::Ptr Figure::hAxis3d(const Channel& name)
{
  CSetOfObjects::Ptr obj;
  auto it = mSysAxis3d.find(name);
  if(it != mSysAxis3d.end())  obj = it->second;
  return obj;
}

} // namespace gui3d

