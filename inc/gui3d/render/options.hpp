#pragma once
#include <gui3d/base/type_def.h>
#define FRAME_GAP_LENGTH 1

namespace gui3d {

struct FigureOption
{
    bool bWaitKey     = false;              // For waitKey
    bool bExit        = false;              // Exit GUI Window
    bool bSave3DScene = false;              // Save .3DScene
    bool RequestToRefresh3DView = true;     // Refresh 3D View


    bool bPlaneXY     = false;              // switch show or hide XY Plane
    bool bAxis3d      = true;               // switch show or hide Axis3d
    float AxisFrq     = 0.25f;
    bool bViewPort    = true;               // switch show or hide viewport
    bool bViewFrames  = true;               // switch show or hide Frames
    bool bViewMapPoint= true;               // switch show or hide MapPoint
};

// Engine provide status for ui-interface, for example
//     case 1: flag for control running, sleep
//     case 2: flag for switch show or hide rendering model
// Option in Program Control Stream
struct ControlOption
{
    // Other toggles
    int ReadFrameGap = FRAME_GAP_LENGTH;   // Read n Frame and Stop when ReadNextFrame = false
    bool ReadNextFrame = true;             // Read Next Frame

    bool StopWhenLost = false;             // For vSLAM's tracking status
    bool StopWhenInitOK = true;
    bool bCacheIm = false;                 // Save viewImage()'s image
};

// Option in 3d SLAM's Scene
struct SceneOption
{
    // Display toggles
    bool bViewKeyframes = true;
	bool bViewPoseNames = true;
    bool bViewLocalFrames = false;
    bool bViewFramesPath = false;

    bool bViewGlobalMapPoints = false;
    bool bViewRefMapPoints = true;
    bool bViewPointCloud = true;

	bool bViewConnections = false;
	bool bViewLoopClosed = true;
    bool bOpenOptimizerPlot = false;

    // parameter of model
    float KFScale = 0.25;
    float CurKFScale = 1;
};

struct Gui3dOption
{
    FigureOption  figOpt;
    ControlOption conOpt;
    SceneOption   sceneOpt;
};

} // namespace gui3d
