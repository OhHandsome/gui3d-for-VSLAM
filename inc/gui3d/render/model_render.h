#pragma once
#include <gui3d/utils/dependency.hpp>
#include <gui3d/gui.h>

namespace gui3d{

typedef COpenGLScenePtr GLScenePtr;

// Construct Model
CFrustumPtr        ModelFrame (int width, int height, float fx);    // Create Camera Model
void               ModelFrames(CSetOfObjects* objs, const PoseV& vPose, const NameV& vLabel, const tOptions& options);

CSetOfObjectsPtr   ModelRobot ();                                    // Create iRobot Model

void               ZoomObjects(CSetOfObjectsPtr objs, float scale);
void               ZoomAxis   (GLScenePtr theScene, float freq);
bool               VisibleText(GLScenePtr theScene, bool bShow);
void               SaveScene  (GLScenePtr theScene, const std::string& route = ".");

// Render Utils
bool renderFrame(GLScenePtr theScene, CFrustumPtr     & obj, const Pose& Twc, const tOptions& options);
bool renderRobot(GLScenePtr theScene, CSetOfObjectsPtr& obj, const Pose& Twb, const tOptions& options);

bool renderFrames(GLScenePtr theScene, CSetOfObjectsPtr& obj, const PoseV& vPose, const NameV& vLabels, const tOptions& options);
bool renderLines (GLScenePtr theScene, CSetOfLinesPtr&   obj, const Position3dV& vvPoints, const tOptions& options);

bool renderMapPoints (GLScenePtr theScene, CPointCloudPtr&         obj, const Pose& Twc, const LandMark3dV& vPoints, const tOptions& options);
bool renderPointCloud(GLScenePtr theScene, CPointCloudColouredPtr& obj, const Pose& Twc, const PointCloud& vPoints, const tOptions& options);

// Render ViewPort
bool auxViewAt(GLScenePtr theScene, COpenGLViewportPtr& gl_view_Camera, const Pose& pose);
bool viewImage(GLScenePtr theScene, COpenGLViewportPtr& gl_view_Image,  const cv::Mat& im, const std::string& name = "Image");


template <class T>
void VisiableModel(T obj, bool bShow)
{
    if(!obj)  return;
    obj->setVisibility(bShow);
}


} // namespace gui3d

