#pragma once

#define White            gui3d::Color(255,255,255,128)
#define Black            gui3d::Color(0  ,0  ,0  ,255)
#define Red              gui3d::Color(255,0  ,0  ,255)
#define Green            gui3d::Color(0  ,255,0  ,255)
#define Blue             gui3d::Color(0  ,0  ,255,255)
#define Yellow           gui3d::Color(255,255,0  ,255)
#define Orange           gui3d::Color(255,110,0  ,255)
#define Cyan             gui3d::Color(0  ,255,255,255)
#define Purple           gui3d::Color(160, 32,240,255)
#define Plum             gui3d::Color(221,160,221,255)
#define SeaGreen         gui3d::Color(84,255,159,255)
#define OrangeRed        gui3d::Color(255,69,0,255)
#define Snow             gui3d::Color(255,250,250,255)

#define I44              gui3d::Pose::Identity()

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
    "Model3d",
    "RgbdNormals"
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
    Model3d,
    RgbdNormals
};

