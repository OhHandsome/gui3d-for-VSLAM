#include <gui3d/utils/dependency.hpp>
#include <gui3d/render/figure.h>
#include <gui3d/render/model_render.h>
#include <gui3d/render/observer.h>
#include <gui3d/render/style.h>

#if HAS_IMGUI
#include <gui3d/window/CDisplayWindow3D.h>
#include <imgui.h>
#endif

//-------------------    Local Function   ---------------------------//
template<class T>
void removeObject(COpenGLViewportPtr mainVP, std::map<std::string, T>& m) {
  for (auto item : m) {
    if (item.second)
      mainVP->removeObject(item.second);
  }
  m.clear();
}

#if HAS_IMGUI
// common edit
template <class Object>
void editObjects(
    const std::string& name,
    std::map<std::string, Object>& map_objects)
{
  if (map_objects.empty())
    return;

  if (ImGui::TreeNode(name.c_str())) {
    for (auto& item : map_objects) {
      auto obj = item.second;
      if (ImGui::TreeNode(obj->getName().c_str())) {
        float s = obj->getScaleX();
        ImGui::InputFloat("scale", &s, 0.1f, 1.0f, "%.3f");
        obj->setScale(s);
        ImGui::TreePop();
      }
    }
    ImGui::TreePop();
  }
}

void editPoseList(
    const std::string& name,
    std::map<std::string, CSetOfObjectsPtr>& map_poselist){

  if (map_poselist.empty())
    return;

  if (ImGui::TreeNode(name.c_str())) {
    for (auto& item : map_poselist) {
      auto obj = item.second;
      if (ImGui::TreeNode(obj->getName().c_str())) {
        float s = (*obj->begin())->getScaleX();
        ImGui::InputFloat("scale", &s, 0.1f, 1.0f, "%.3f");
        for (auto itO = obj->begin(); itO != obj->end(); ++itO) {
          (*itO)->setScale(s);
        }
        ImGui::TreePop();
      }
    }
    ImGui::TreePop();
  }
}

void editTheScene(void* hObject) {

  gui3d::Figure* figure = (gui3d::Figure*)(hObject);
  const auto& window = figure->mMainWindow;
  if (ImGui::CollapsingHeader("TheScene")) {
    editObjects("Frame",        figure->mSysFrame);
    editPoseList("Frames",      figure->mSysPoseList);
    editObjects("Robot",        figure->mSysRobot);
    editObjects("MapPoint",     figure->mSysMapPoint);
    editObjects("PointCloud",   figure->mSysPointCloud);
    editObjects("Model",        figure->mSysModel3d);
    editObjects("RgbAxis3d",    figure->mSysRgbAxis3d);
  }
  window->unlockAccess3DScene();
}
#endif

namespace gui3d {

int Figure::mNextID = 1;
Figure::Figure(const string& name, int width, int height) {
#if HAS_IMGUI == 1
  // Create 3D Windows
  gui3d::CDisplayWindow3DPtr win = gui3d::CDisplayWindow3DPtr(new CDisplayWindow3D(name, width, height));
  mMainWindow = win;
  this->name = name;
  mFigureID = mNextID++;
  win->setEditTheScene(editTheScene, this);
#else
  // Create 3D Windows
  mrpt::gui::CDisplayWindow3DPtr win = mrpt::gui::CDisplayWindow3DPtr(new mrpt::gui::CDisplayWindow3D(name, width, height));
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
    COpenGLScenePtr &theScene = win->get3DSceneAndLock();
    COpenGLViewportPtr the_main_view = theScene->getViewport("main");
    mObserver->observeBegin(*the_main_view);
    win->unlockAccess3DScene();
  }
  win->repaint();
#endif
}

void Figure::init() {
#if HAS_IMGUI == 1
  mMainWindow->InitScene();
#else
  auto &bAxis3d  = mOption.figOpt.bAxis3d;
  auto &bPlaneXY = mOption.figOpt.bPlaneXY;
  //  Get the smart pointer to the main viewport object in this mMainWindow:
  {
    COpenGLScenePtr &theScene = mMainWindow->get3DSceneAndLock();
    CAxisPtr Axis = CAxis::Create(-AXISLength, -AXISLength, -AXISLength,
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

void Figure::clear() {
  if (!mMainWindow)
    return;

  mScene = mMainWindow->get3DSceneAndLock();
  if (!mGLViewImage) mGLViewImage.clear();
  if (!mGLSubView) mGLSubView.clear();

  if (mScene) {
    COpenGLViewportPtr mainVP = mScene->getViewport();
    if (mainVP) {
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

volatile Gui3dOption& Figure::Options() {
#if HAS_IMGUI
  return mMainWindow->Options();
#else
  return mOption;
#endif
}

// Find Channel's hObject
CFrustumPtr Figure::hFrame(const Channel& name) {
  CFrustumPtr obj;
  auto it = mSysFrame.find(name);
  if (it != mSysFrame.end()) obj = it->second;
  return obj;
}

CSetOfObjectsPtr Figure::hRobot(const Channel& name) {
  CSetOfObjectsPtr obj;
  auto it = mSysRobot.find(name);
  if (it != mSysRobot.end()) obj = it->second;
  return obj;
}

CSetOfObjectsPtr Figure::hPoseList(const Channel& name) {
  CSetOfObjectsPtr obj;
  auto it = mSysPoseList.find(name);
  if (it != mSysPoseList.end()) obj = it->second;
  return obj;
}

CPointCloudPtr Figure::hMapPoint(const Channel& name) {
  CPointCloudPtr obj;
  auto it = mSysMapPoint.find(name);
  if (it != mSysMapPoint.end()) obj = it->second;
  return obj;
}

CPointCloudColouredPtr Figure::hPointCloud(const Channel& name) {
  CPointCloudColouredPtr obj;
  auto it = mSysPointCloud.find(name);
  if (it != mSysPointCloud.end()) obj = it->second;
  return obj;
}

CSetOfLinesPtr Figure::hLine(const Channel& name) {
  CSetOfLinesPtr obj;
  auto it = mSysLine.find(name);
  if (it != mSysLine.end()) obj = it->second;
  return obj;
}

CSetOfObjectsPtr Figure::hModel3d(const Channel& name) {
  CSetOfObjectsPtr obj;
  auto it = mSysModel3d.find(name);
  if (it != mSysModel3d.end()) obj = it->second;
  return obj;
}

CSetOfObjectsPtr Figure::hAxis3d(const Channel& name) {
  CSetOfObjectsPtr obj;
  auto it = mSysRgbAxis3d.find(name);
  if (it != mSysRgbAxis3d.end()) obj = it->second;
  return obj;
}

} // namespace gui3d

