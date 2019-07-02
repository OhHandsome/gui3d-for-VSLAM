#include <gui3d/render/scene_manager.h>
#include <gui3d/render/model_render.h>
#include <Third-party/imgui/imgui.h>

static void ShowHelpMarker(const char* desc) {
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

namespace gui3d {

template<class T>
void removeObject(COpenGLViewport::Ptr mainVP, std::map<Channel, T>& m) {
  for (auto item : m) {
    if (item.second)
      mainVP->removeObject(item.second);
  }
  m.clear();
}

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
        // scale
        float s = obj->getScaleX();
        if (ImGui::InputFloat("scale", &s, 0.1f, 1.0f, "%.3f")) {
          obj->setScale(s);
        }
        // color
        TColorf tmp = obj->getColor();
        float color[3] = {tmp.R, tmp.G, tmp.B};
        if (ImGui::ColorEdit3("color", (float*)&color)) {  // Edit 3 floats representing a color
          obj->setColor(TColorf(color[0], color[1], color[2]));
        }
        ImGui::TreePop();
      }
    }
    ImGui::TreePop();
  }
}

void editObjects(
  const std::string& name,
  std::map<std::string, CSetOfObjects::Ptr>& map_poselist){

  if (map_poselist.empty())
    return;

  if (ImGui::TreeNode(name.c_str())) {
    for (auto& item : map_poselist) {
      auto obj = item.second;
      if (ImGui::TreeNode(obj->getName().c_str())) {
        // scale
        float s = (*obj->begin())->getScaleX();
        if (ImGui::InputFloat("scale", &s, 0.1f, 1.0f, "%.3f")) {
          for (auto itO = obj->begin(); itO != obj->end(); ++itO)
            (*itO)->setScale(s);
        }
        // color
        TColorf tmp = obj->getColor();
        float color[3] = {tmp.R, tmp.G, tmp.B};
        if (ImGui::ColorEdit3("color", (float*)&color)) {  // Edit 3 floats representing a color
          obj->setColor(TColorf(color[0], color[1], color[2]));
        }
        ImGui::TreePop();
      }
    }
    ImGui::TreePop();
  }
}

SceneManager::SceneManager() {
  m_3Dscene = COpenGLScene::Create();
  renderCartesianCoordinate(m_3Dscene, m_Axis3d, m_ZeroPlane, true);
}

void SceneManager::clear() {
  m_3Dscene->clear();
  m_Axis3d.reset();
  m_ZeroPlane.reset();

  mSysFrame.clear();
  mSysRobot.clear();
  mSysPoseList.clear();
  mSysMapPoint.clear();
  mSysPointCloud.clear();
  mSysLine.clear();
  mSysModel3d.clear();
  mSysRgbAxis3d.clear();
}

void SceneManager::reset() {
  if (m_3Dscene) {
    COpenGLViewport::Ptr mainVP = m_3Dscene->getViewport();
    if (mainVP) {
      removeObject(mainVP, mSysFrame);
      removeObject(mainVP, mSysRobot);
      removeObject(mainVP, mSysPoseList);
      removeObject(mainVP, mSysMapPoint);
      removeObject(mainVP, mSysLine);
      removeObject(mainVP, mSysModel3d);
    }
  }
}

COpenGLScene::Ptr& SceneManager::get3DSceneAndLock() {
  m_access3Dscene.lock();
  return m_3Dscene;
}

void SceneManager::unlockAccess3DScene() {
  m_access3Dscene.unlock();
}

void SceneManager::render_visiable() {

  COpenGLViewport::Ptr mainVP = m_3Dscene->getViewport();

  // 1. Firstly, check visible_all
  visible_all = true;
  for (COpenGLViewport::const_iterator itO = mainVP->begin();
       itO!= mainVP->end();
       ++itO) {
    if((*itO)->getName() == m_Axis3d->getName() ||
       (*itO)->getName() == m_ZeroPlane->getName()) // remove Axis and ZeroPlane
      continue;

    if (!(*itO)->isVisible()) {
      visible_all = false;
      break;
    }
  }

  // 2. Secondly, check box
  if (ImGui::Checkbox("[ALL]", &visible_all)) {
    for (COpenGLViewport::iterator itO = mainVP->begin();
         itO!= mainVP->end();
         ++itO) {
      if((*itO)->getName() == m_Axis3d->getName() ||
         (*itO)->getName() == m_ZeroPlane->getName() ||
         (*itO)->getName().empty())
        continue;
      (*itO)->setVisibility(visible_all);
    }
  }
  ImGui::SameLine();
  ShowHelpMarker("Toggle visibility's of all objects");
  for (COpenGLViewport::iterator itO = mainVP->begin();
       itO!= mainVP->end();
       ++itO) {
    if((*itO)->getName() == m_Axis3d->getName() ||
       (*itO)->getName() == m_ZeroPlane->getName() ||
      (*itO)->getName().empty())
      continue;

    bool visible = (*itO)->isVisible();
    if (ImGui::Checkbox((*itO)->getName().c_str(), &visible))
      (*itO)->setVisibility(visible);
  }
}

void SceneManager::render_property() {

  // Visible about Axis3d and ZeroPlane
  bool visible = m_Axis3d->isVisible();
  if (ImGui::Checkbox("Axis3d", &visible))
    m_Axis3d->setVisibility(visible);
  ImGui::SameLine();
  visible = m_ZeroPlane->isVisible();
  if (ImGui::Checkbox("XY", &visible))
    m_ZeroPlane->setVisibility(visible);

  ImGui::Text("Label:");
  ImGui::SameLine();
  int v[2];
  v[0] = m_Axis3d->getFrequency();
  v[1] = m_ZeroPlane->getGridFrequency();
  ImGui::SliderInt2("FREQ", v, 1, 8); // Edit 1 Int using a slider from 1 to 8
  m_Axis3d->setFrequency(v[0]);
  m_ZeroPlane->setGridFrequency(v[1]);

  if (ImGui::CollapsingHeader("TheScene", true)) {
    editObjects("Frame",        mSysFrame);
    editObjects("Frames",       mSysPoseList);
    editObjects("Robot",        mSysRobot);
    editObjects("MapPoint",     mSysMapPoint);
    editObjects("PointCloud",   mSysPointCloud);
    editObjects("Model",        mSysModel3d);
    editObjects("RgbAxis3d",    mSysRgbAxis3d);
  }
}

// Find Channel's hObject
CFrustum::Ptr SceneManager::hFrame(const Channel& name) {
  CFrustum::Ptr obj;
  auto it = mSysFrame.find(name);
  if (it != mSysFrame.end()) obj = it->second;
  return obj;
}

CSetOfObjects::Ptr SceneManager::hRobot(const Channel& name) {
  CSetOfObjects::Ptr obj;
  auto it = mSysRobot.find(name);
  if (it != mSysRobot.end()) obj = it->second;
  return obj;
}

CSetOfObjects::Ptr SceneManager::hPoseList(const Channel& name) {
  CSetOfObjects::Ptr obj;
  auto it = mSysPoseList.find(name);
  if (it != mSysPoseList.end()) obj = it->second;
  return obj;
}

CPointCloud::Ptr SceneManager::hMapPoint(const Channel& name) {
  CPointCloud::Ptr obj;
  auto it = mSysMapPoint.find(name);
  if (it != mSysMapPoint.end()) obj = it->second;
  return obj;
}

CPointCloudColoured::Ptr SceneManager::hPointCloud(const Channel& name) {
  CPointCloudColoured::Ptr obj;
  auto it = mSysPointCloud.find(name);
  if (it != mSysPointCloud.end()) obj = it->second;
  return obj;
}

CSetOfLines::Ptr SceneManager::hLine(const Channel& name) {
  CSetOfLines::Ptr obj;
  auto it = mSysLine.find(name);
  if (it != mSysLine.end()) obj = it->second;
  return obj;
}

CSetOfObjects::Ptr SceneManager::hModel3d(const Channel& name) {
  CSetOfObjects::Ptr obj;
  auto it = mSysModel3d.find(name);
  if (it != mSysModel3d.end()) obj = it->second;
  return obj;
}

CSetOfObjects::Ptr SceneManager::hAxis3d(const Channel& name) {
  CSetOfObjects::Ptr obj;
  auto it = mSysRgbAxis3d.find(name);
  if (it != mSysRgbAxis3d.end()) obj = it->second;
  return obj;
}

} // namespace gui3d

