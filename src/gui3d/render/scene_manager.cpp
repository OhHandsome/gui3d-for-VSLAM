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
  mSysAxis3d.clear();
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
         (*itO)->getName() == m_ZeroPlane->getName())
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
       (*itO)->getName() == m_ZeroPlane->getName())
      continue;

    bool visible = (*itO)->isVisible();
    if (ImGui::Checkbox((*itO)->getName().c_str(), &visible))
      (*itO)->setVisibility(visible);
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
  auto it = mSysAxis3d.find(name);
  if (it != mSysAxis3d.end()) obj = it->second;
  return obj;
}

} // namespace gui3d

