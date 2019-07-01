#pragma once

#include <gui3d/utils/dependency.hpp>

namespace gui3d {

typedef COpenGLScene::Ptr GLScenePtr;
struct tOptions;

bool renderCartesianCoordinate(GLScenePtr theScene, CAxis::Ptr& Axis, CGridPlaneXY::Ptr& horizon, const bool visible);

// Construct Model
CFrustum::Ptr        ModelFrame (int width, int height, float fx);    // Create Camera Model
void                 ModelFrames(CSetOfObjects* objs, const PoseV& vPose, const NameV& vLabel, const tOptions& options);
CSetOfObjects::Ptr   ModelRGBAxis(const float length);
CSetOfObjects::Ptr   ModelRobot ();                                    // Create iRobot Model

bool               VisibleText(GLScenePtr theScene, bool bShow);
void               saveToPlyFile(const std::string& name, CPointCloud::Ptr landmark, bool have_color = false);
void               saveToPlyFile(const std::string& name, CPointCloudColoured::Ptr landmark, bool have_color = true);
void               SaveScene  (GLScenePtr theScene, const std::string& route = ".");
void               SaveSceneAsPLY(GLScenePtr theScene, const std::string& route = ".");

// Render Utils
bool renderFrame(GLScenePtr theScene, CFrustum::Ptr     & obj, const Pose& Twc, const tOptions& options);
bool renderRobot(GLScenePtr theScene, CSetOfObjects::Ptr& obj, const Pose& Twb, const tOptions& options);
bool renderRGBAxis(GLScenePtr theScene, CSetOfObjects::Ptr &obj, const Pose &Twb, const tOptions &options);

bool renderFrames(GLScenePtr theScene, CSetOfObjects::Ptr& obj, const PoseV& vPose, const NameV& vLabels, const tOptions& options);
bool renderLines (GLScenePtr theScene, CSetOfLines::Ptr&   obj, const Position3dV& vvPoints, const tOptions& options);

bool renderMapPoints (GLScenePtr theScene, CPointCloud::Ptr&         obj, const Pose& Twc, const LandMark3dV& vPoints, const tOptions& options);
bool renderPointCloud(GLScenePtr theScene, CPointCloudColoured::Ptr& obj, const Pose& Twc, const PointCloud& vPoints, const tOptions& options);


} // namespace gui3d

