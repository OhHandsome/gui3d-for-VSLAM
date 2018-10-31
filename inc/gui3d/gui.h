#pragma once

#include <gui3d/render/options.hpp>
#include <gui3d/render/parameter.h>
#include <gui3d/base/type_def.h>

namespace cv{ class Mat; };

namespace gui3d{

// System include base Channel for SLAM's Render
/*
 SysChannel:
	"CurCamera",       // Camera Channel
	"PrediectCamera",
	"RefMapPoints",    // MapPoint Channel
	"GlobalMapPoints",
	"LocalFrames",     // PoseList Channel
	"KeyFrames",
	"Depth",     // PointCloud Channel
	"Model3d",
*/

// Render Parameter
struct tOptions{
    enum { HOLD_ON, HOLD_OFF };

    tOptions& setColor(const Color& color) { _color = color; }
    tOptions& setScale(const float scale)  { _scale = scale; }
    tOptions& setMethod(bool method)       { _method = method; }

    tOptions() : _color(Color::Zero()), _scale(1.0f), _method(HOLD_OFF){}
    tOptions(const Color& color, float scale, bool flag) :
        _color(color), _scale(scale), _method(flag) {}

    Color _color;
    float _scale;
    bool  _method;
};



inline string Engine() {return "vSLAM for ArcSoft";}

// Figure for 3d
hObject nFigure(const string& name, int width = 640, int height = 480);
void setAsOutputFigure(hObject fig);
void addTextMessage(double x, double y, const string &text, size_t unique_index);
void moveFigure(int x, int y);
void destoryFigure(const string& name);
void destoryFigure(hObject fig);
void play_control();
void waitKey(int delay_ms = 0);

// Render Base Complement
const string& workRoute();
void setDataRoute(const char* data_path);
volatile Gui3dOption& Option();
hObject renderFrame     (const Channel& name, const Pose&  Twc,          const tOptions& options);
hObject renderFrames    (const Channel& name, const PoseV& vTwc,         const tOptions& options,
                         const NameV& vLabels = NameV());
hObject renderLines     (const Channel& name, const Position3dV& vPoint, const tOptions& options);
hObject renderPath      (const Channel& name, const PoseV& vTwc,         const tOptions& options);
hObject renderMapPoints (const Channel& name, const LandMark3dV& vPoint, const tOptions& options);
hObject renderPointCloud(const Channel& name, const PointCloud& cloud,   const tOptions& options);
hObject renderModel3d   (const Channel& name, const PoseV& Twc, const PointCloudV& cloud, const tOptions& options);
hObject renderRobot     (const Channel& name, const Pose& Twb,           const tOptions& options);
hObject viewImage       (const cv::Mat& im);  // Render ViewPort, for example, imshow(), Aux Viewport
hObject viewDepth       (const cv::Mat& im, const cv::Mat& depth);
hObject auxViewAt       (const Pose& pose);    // GL Camera Pose in Aux ViewPort for Multi-View
void    update          (hObject obj, const Pose& Twc);
void    repaint();

// Image Display
hObject im_show(const string& name, const cv::Mat& im);
hObject im_show(const string& name, const cv::Mat& im, const cv::Mat& depth);
void waitExit(hObject hfig, int delay_ms = 0);

void collectCloudFromRGBD(const cv::Mat& im, const cv::Mat& depth, PointCloud& cloud);
void saveAsPLY(const std::string& name, const Pose& Twc, const PointCloud& cloud);
void saveAsPLY(const std::string& name, const Pose& Twc, const cv::Mat& im, const cv::Mat& depth);


}