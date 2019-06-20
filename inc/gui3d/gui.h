#pragma once

#include <gui3d/render/options.hpp>
#include <gui3d/render/parameter.h>
#include <gui3d/base/type_def.h>
#include <opencv2/core.hpp>

namespace gui3d {

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

    tOptions& setColor(const Color& color) { _color = color; return *this;}
    tOptions& setScale(const float scale)  { _scale = scale; return *this;}
    tOptions& setMethod(bool method)       { _method = method; return *this;}

    tOptions() : _color(Blue), _scale(0.250f), _method(HOLD_OFF){}
    tOptions(const Color& color, float scale, bool flag) :
        _color(color), _scale(scale), _method(flag) {}

    Color _color;
    float _scale;
    bool  _method;
};


// Gui3d include default window3d for vSLAM
inline string Engine() {return "vSLAM for ArcSoft";}

// Figure for 3d
hObject nFigure(const string& name, int width = -1, int height = -1);
void setAsCurrentFigure(hObject fig); // same with gcf in matlab
void addTextMessage(double x, double y, const string &text, size_t unique_index);
void moveFigure(int x, int y);
void destoryFigure(const string& name);
void destoryFigure(hObject fig);
void play_control();
void waitKey(int delay_ms = 0);

// Image Display
hObject imshow(const string& name, const cv::Mat& im);
hObject imshow(const string& name, const cv::Mat& im, const cv::Mat& array2d_pt3d);
void waitExit(hObject hfig, int delay_ms = 0);

// Render Base Complement
const string& workRoute();
const string& dataRoute();
void setDataRoute(const char* data_path);
void setWorkRoute(const char* cache_path = nullptr); // save debug file
volatile Gui3dOption& Option();
hObject renderFrame     (const Channel& name, const Pose&  Twc,          const tOptions& options = tOptions());
hObject renderFrames    (const Channel& name, const PoseV& vTwc,         const NameV& vLabels = NameV(),
                         const tOptions& options = tOptions());
hObject renderLines     (const Channel& name,
                         const Pose&    Twc , const Position3dV& vPoint, const tOptions& options = tOptions());
hObject renderPath      (const Channel& name, const Position3dV& vPoint, const tOptions& options = tOptions());
hObject renderPolygon   (const Channel& name, // name
                         const Pose&    Twc , const Position3dV& vPoint, const tOptions& options = tOptions());
hObject renderMapPoints (const Channel& name, const LandMark3dV& vPoint, const tOptions& options = tOptions());
hObject renderPointCloud(const Channel& name, const PointCloud& cloud,   const tOptions& options = tOptions());

hObject renderRGBAxis(const Channel &name, const Pose &Twb, const tOptions &options = tOptions());
hObject renderModel3d   (const Channel& name,
                         const PoseV&   Twc , const PointCloudV& cloud,  const tOptions& options = tOptions());
hObject renderRobot     (const Channel& name, const Pose& Twb,           const tOptions& options = tOptions());
hObject viewImage       (const cv::Mat& im);  // Render ViewPort, for example, imshow(), Aux Viewport
hObject viewDepth       (const cv::Mat& array2d_pt3d, const cv::Mat& im = cv::Mat());
hObject viewRgbdNormals (const cv::Mat& array2d_pt3d, const cv::Mat& normals);
hObject auxViewAt       (const Pose& pose);    // GL Camera Pose in Aux ViewPort for Multi-View
void    update          (hObject obj, const Pose& Twc);
void    clear();
void    repaint();

void collectCloudFromRGBD(const cv::Mat& im, const cv::Mat& depth, PointCloud& cloud);
void collectCloudFromDepth(const cv::Mat& depth, LandMark3dV& cloud);
void collectLinesFromRGBDNormal(const cv::Mat& depth, const cv::Mat& normal, Position3dV& vecN, const float l = 0.02);
void saveAsPLY(const std::string& name, const Pose& Twc, const PointCloud& cloud);
void saveAsPLY(const std::string& name, const Pose& Twc, const cv::Mat& im, const cv::Mat& depth);


}
