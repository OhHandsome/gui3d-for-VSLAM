#pragma once

#include <memory>
#include <mrpt/opengl.h>
#include <gui3d/base/type_def.h>
#include <mutex>

namespace gui3d {

using mrpt::opengl::COpenGLScene;
using mrpt::opengl::COpenGLViewport;
using mrpt::opengl::CFrustum;
using mrpt::opengl::CSetOfObjects;
using mrpt::opengl::CPointCloud;
using mrpt::opengl::CPointCloudColoured;
using mrpt::opengl::CSetOfLines;
using mrpt::opengl::CAxis;
using mrpt::opengl::CGridPlaneXY;

class SceneManager
{

public:
  SceneManager();

  /** Gets a reference to the smart shared pointer that holds the internal scene (carefuly read introduction in gui::CDisplayWindow3D before use!)
  *  This also locks the critical section for accesing the scene, thus the window will not be repainted until it is unlocked.
  * \note It is safer to use mrpt::gui::CDisplayWindow3DLocker instead.*/
  mrpt::opengl::COpenGLScene::Ptr& get3DSceneAndLock();

  /** Unlocks the access to the internal 3D scene. It is safer to use mrpt::gui::CDisplayWindow3DLocker instead.
    *  Typically user will want to call forceRepaint after updating the scene. */
  void unlockAccess3DScene();

  // get Figure3d's hObject
	CFrustum::Ptr            hFrame(const Channel& name);
	CSetOfObjects::Ptr       hRobot(const Channel& name);
	CSetOfObjects::Ptr       hPoseList(const Channel& name);
	CPointCloud::Ptr         hMapPoint(const Channel& name);
	CPointCloudColoured::Ptr hPointCloud(const Channel& name);
	CSetOfLines::Ptr         hLine(const Channel& name);
	CSetOfObjects::Ptr       hModel3d(const Channel& name);
	CSetOfObjects::Ptr       hAxis3d(const Channel& name);

	void reset();
	void clear();
	void render_visiable();
	void render_property();

 public:
  COpenGLScene::Ptr m_3Dscene;
  std::mutex        m_access3Dscene;

  CAxis::Ptr        m_Axis3d;
	CGridPlaneXY::Ptr m_ZeroPlane;

	std::map<Channel, CFrustum::Ptr>            mSysFrame;
	std::map<Channel, CSetOfObjects::Ptr>       mSysRobot;
	std::map<Channel, CSetOfObjects::Ptr>       mSysPoseList;
	std::map<Channel, CPointCloud::Ptr>         mSysMapPoint;
	std::map<Channel, CPointCloudColoured::Ptr> mSysPointCloud;
	std::map<Channel, CSetOfLines::Ptr>         mSysLine;
	std::map<Channel, CSetOfObjects::Ptr>       mSysModel3d;

	std::map<Channel, CSetOfObjects::Ptr>       mSysRgbAxis3d;

 protected:
  bool visible_all = true;
};

} // namespace gui3d
