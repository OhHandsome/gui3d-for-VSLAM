#pragma once

#ifndef M_PI
#define M_PI 3.1415926
#endif

#define White  Color(255,255,255,128)
#define Black  Color(0  ,0  ,0  ,255)
#define Red    Color(255,0  ,0  ,255)
#define Green  Color(0  ,255,0  ,255)
#define Blue   Color(0  ,0  ,255,255)
#define Yellow Color(255,255,0  ,255)
#define Orange Color(255,110,0  ,255)
#define Cyan   Color(0  ,255,255,255)
#define Purple Color(160, 32,240,255)
#define Plum   Color(221,160,221,255)
#define SeaGreen Color(84,255,159,255)
#define OrangeRed Color(255,69,0,255)
#define Snow   Color(255,250,250,255)

enum TextID
{
    RUN_STATE = 0,
    POSITION,
    FRAME,
    LOCALMAP,
    GLOBALMAP,
    VSLAM_STATUS,
    RENDER_FPS,
    NEED_KEYFRAME,
    CAPTURE_IMAGES

};

enum EVENT_TYPE {
    RESCALE_KF = 0x0001,
    VISIABLE_AXIS = 0x0002,
    CONNECTIONS = 0x0004,
    HIDE_VIEWS = 0x0008,
    SWITCH_VIEW = 0x0010,
};


// Text Height
#define LINE_WIDTH              30

// Text X
#define TEXT_X                  (800-50)
#define TEXT_RUN_STATE_X        (TEXT_X + 180)

// Text Y
#define TEXT_RUN_STATE_Y        50
#define TEXT_POSITION_Y         (TEXT_RUN_STATE_Y + LINE_WIDTH)
#define TEXT_LOCALMAP_Y         (TEXT_POSITION_Y + LINE_WIDTH)
#define TEXT_GLOBALMAP_Y        (TEXT_LOCALMAP_Y + LINE_WIDTH)
#define TEXT_SYSTEM_STATUS_Y    (TEXT_GLOBALMAP_Y + LINE_WIDTH)
#define TEXT_FREQ_Y             (TEXT_SYSTEM_STATUS_Y + LINE_WIDTH)
#define TEXT_NEED_KEYFRAME_Y    (TEXT_FREQ_Y + LINE_WIDTH)
#define TEXT_CAPTURE_IMAGE_Y    (TEXT_NEED_KEYFRAME_Y + LINE_WIDTH)





static const char* sysChannel[] = {
    "CurCamera",       // Camera Channel
    "PrediectCamera",
    "GTCamera",
    "RefMapPoints",    // MapPoint Channel
    "GlobalMapPoints",
    "LocalFrames",     // PoseList Channel
    "KeyFrames",
    "Path",
    "DepthPointCloud", // PointCloud Channel
    "Model3d"
};

enum SysChannelType{
    CurCamera = 0,
    PrediectCamera,
    GTCamera,
    RefMapPoints,    // MapPoint Channel
    GlobalMapPoints,
    LocalFrames,     // PoseList Channel
    KeyFrames,
    Path,
    DepthPointCloud,     // PointCloud Channel
    Model3d
};

