/**
 * @note
 *      3DSceneView to Sturcture from motion(SFM), such as Pose,Point
 *      you can show onlinely or save as .3dScene and call mobile robot program toolkit(MRPT)
 *      to show 3D Map + PointCloud.
 * @attention
 *      before running Engine, you must be install MRPT lib
 *      https://www.mrpt.org/
 *      If you have some question, please contact oyg5285@arcsoft.com
 */

#include <gui3d/utils/cast_utils.h>
#include <gui3d/render/scene_manager.h>
#include <gui3d/render/model_render.h>
#include <gui3d/window/CDisplayWindow3D.h>
#include <opencv2/core/base.hpp>
#include "gui.h"

namespace gui3d {

static CDisplayWindow3DPtr sCurrentFigure3d = nullptr;
void registerSystemChannelOptions();
bool systemChannelOptions(const Channel& name, tOptions& options);

// ----------------------- Figure Handle ----------------------------//
hObject nFigure(const string& name, int width, int height) {
  CV_Assert(sCurrentFigure3d == nullptr);
  registerSystemChannelOptions();
  sCurrentFigure3d = CDisplayWindow3D::Create(name, width, height);
  return (hObject) sCurrentFigure3d.get();
}

// ----------------------- Render Handle ----------------------------//
hObject renderFrame(const Channel& name, const Pose& Twc, const tOptions& options)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

    CFrustum::Ptr obj = sCurrentFigure3d->hFrame(name);
    auto win = sCurrentFigure3d;
    auto theScene = win->get3DSceneAndLock();
    renderFrame(theScene, obj, Twc, real_options);
    if(obj) obj->setName(name);
    win->unlockAccess3DScene();
    sCurrentFigure3d->mSysFrame[name] = obj;
    return (hObject)(obj.get());
}

hObject renderFrames(const Channel& name, const PoseV& vTwc, const NameV& vLabels, const tOptions& options)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

    CSetOfObjects::Ptr obj = sCurrentFigure3d->hPoseList(name);
    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    renderFrames(theScene, obj, vTwc, vLabels, real_options);
    if(obj) obj->setName(name);
    win->unlockAccess3DScene();
    sCurrentFigure3d->mSysPoseList[name] = obj;
    return (hObject)(obj.get());
}

hObject renderLines(const Channel& name, const Pose& Twq, const Position3dV& vPoint, const tOptions& options)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

    CSetOfLines::Ptr obj = sCurrentFigure3d->hLine(name);
    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    renderLines(theScene, obj, vPoint, real_options);
    obj->setPose(castPose(Twq));
    if(obj) obj->setName(name);
    win->unlockAccess3DScene();
    sCurrentFigure3d->mSysLine[name] = obj;
    return (hObject)(obj.get());
}

hObject renderPath(const Channel& name, const Position3dV& vPoints, const tOptions& options)
{
    Position3dV vLines;
    for(int i = 1; i < vPoints.size(); i++)
    {
        vLines.push_back(vPoints[i-1]);
        vLines.push_back(vPoints[i]);
    }
    return renderLines(name, Pose::Identity(), vLines, options);
}

hObject renderPolygon(const Channel& name, const Pose& Twq, const Position3dV& vPoint, const tOptions& options)
{
    Position3dV vLines;
    const int n_size = (int)vPoint.size();
    for(int i = 0; i < n_size; i++)
    {
        LandMark3d Point1 = vPoint[i];
        LandMark3d Point2 = vPoint[ (i+1) % n_size ];
        vLines.push_back(Point1);
        vLines.push_back(Point2);
    }
    hObject h = renderLines(name, Twq, vLines, options);
    return h;
}

hObject renderMapPoints(const Channel& name, const LandMark3dV& vPoint,const tOptions& options)
{
	if(!sCurrentFigure3d)
		nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

	CPointCloud::Ptr obj = sCurrentFigure3d->hMapPoint(name);
	auto win = sCurrentFigure3d->mMainWindow;
	auto theScene = win->get3DSceneAndLock();
    const Pose Twc = Pose::Identity();
    renderMapPoints(theScene, obj, Twc, vPoint, real_options);
    if(obj) obj->setName(name);
    win->unlockAccess3DScene();
    sCurrentFigure3d->mSysMapPoint[name] = obj;
    return (hObject)(obj.get());
}

hObject renderPointCloud(const Channel& name, const PointCloud& cloud,  const tOptions& options)
{
	if(!sCurrentFigure3d)
		nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

    CPointCloudColoured::Ptr obj = sCurrentFigure3d->hPointCloud(name);
	auto win = sCurrentFigure3d->mMainWindow;
	auto theScene = win->get3DSceneAndLock();
    const Pose Twc = Pose::Identity();
    renderPointCloud(theScene, obj, Twc, cloud, real_options);
    if(obj) obj->setName(name);
    win->unlockAccess3DScene();
    sCurrentFigure3d->mSysPointCloud[name] = obj;
    return (hObject)(obj.get());
}

hObject renderRGBAxis(const Channel &name, const Pose &Twb, const tOptions &options)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

    CSetOfObjects::Ptr obj = sCurrentFigure3d->hAxis3d(name);
    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    renderRGBAxis(theScene, obj, Twb, real_options);
    if(obj) obj->setName(name);
    win->unlockAccess3DScene();
    sCurrentFigure3d->mSysAxis3d[name] = obj;
    return (hObject)(obj.get());
}

hObject renderModel3d(const Channel& name, const PoseV& vTwc, const PointCloudV& cloud, const tOptions& options)
{
    assert(vTwc.size() == cloud.size());
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

    CSetOfObjects::Ptr obj = sCurrentFigure3d->hModel3d(name);
    if(obj) obj->clear();
    else    obj = CSetOfObjects::Create();
    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    for(int i = 0; i < vTwc.size(); i++)
    {
        CPointCloudColoured::Ptr o;
        renderPointCloud(theScene, o, vTwc[i], cloud[i], real_options);
        o->setName(cv::format("%s/component_%d", name.c_str(), i));
        obj->insert(o);
    }
    if(obj) obj->setName(name);
    win->unlockAccess3DScene();
    sCurrentFigure3d->mSysModel3d[name] = obj;
    return (hObject)(obj.get());
}

hObject renderRobot(const Channel& name, const Pose& Twb, const tOptions& options)
{
	if(!sCurrentFigure3d)
		nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

	CSetOfObjects::Ptr obj = sCurrentFigure3d->hRobot(name);
	auto win = sCurrentFigure3d->mMainWindow;
	auto theScene = win->get3DSceneAndLock();
	renderRobot(theScene, obj, Twb, real_options);
    if(obj) obj->setName(name);
    win->unlockAccess3DScene();
	sCurrentFigure3d->mSysRobot[name] = obj;
	return (hObject)(obj.get());
}

hObject plot3D(const Channel& name, const cv::Mat& array2d)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    if (array2d.empty() || array2d.type() != CV_32FC1)
        return nullptr;

    tOptions options;
    systemChannelOptions(name, options);

    cv::Mat blur;
    cv::medianBlur(array2d, blur, 5);
    double minVal, maxVal;
    cv::minMaxLoc(blur, &minVal, &maxVal, nullptr, nullptr);
    const float s = 255 / (maxVal - minVal);
    blur.convertTo(blur, CV_8UC1, s);

    cv::Mat array_color;
    cv::applyColorMap(blur, array_color, cv::COLORMAP_HSV);
    PointCloud cloud;
    for (int y = 0; y < array2d.rows; y++)
    {
        auto parray2d = array2d.ptr<float>(y);
        auto pcolor = array_color.ptr<cv::Vec3b>(y);
        for (int x = 0; x < array2d.cols; ++x)
        {
            Eigen::Matrix<float, 7, 1> info;
            info[0] = x - array2d.cols/2;
            info[1] = y - array2d.rows/2;
            info[2] = parray2d[x];

            info[3] = pcolor[x][0]/ 255.f; // R
            info[4] = pcolor[x][1]/ 255.f; // G
            info[5] = pcolor[x][2]/ 255.f; // B
            info[6] = 1.0f;
            cloud.push_back(info);
        }
    }

    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    CPointCloudColoured::Ptr obj = sCurrentFigure3d->hPointCloud(name);
    renderPointCloud(theScene, obj, Pose::Identity(), cloud, options);
    if(obj)
    {
        obj->setName(name);
        if (maxVal != minVal)
            obj->setScale(2.f/array2d.cols, 2.f/array2d.rows, 2.f/(maxVal - minVal));
    }
    sCurrentFigure3d->mSysPointCloud[name] = obj;
    win->unlockAccess3DScene();
    return (hObject)(obj.get());
}

hObject viewImage(const cv::Mat &im)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    COpenGLViewport::Ptr &obj = sCurrentFigure3d->mGLViewImage;
    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
#if HAS_IMGUI
    CDisplayImagesPtr sub_view = win->getViewImage();
    if (!sub_view) {
        sub_view = win->createViewImage("im");
        sub_view->setViewPosition(MainWidth  - im.cols * ZoomOfImage,
                                  0,
                                  im.cols * ZoomOfImage,
                                  im.rows * ZoomOfImage);
    }
    sub_view->setImageView_fast(im);
#else
    viewImage(theScene, obj, im);
#endif
    win->unlockAccess3DScene();

    static size_t num = 0;
    num++;
    auto &bCacheIm = sCurrentFigure3d->mOption.conOpt.bCacheIm;
    if (bCacheIm)
        cv::imwrite(mFileRoute + cv::format("/%d.jpg", num), im);
    return (hObject)(obj.get());
}

hObject viewDepth(const cv::Mat& depth_pts, const cv::Mat& im)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    const std::string& name = sysChannel[DepthPointCloud];
    tOptions options;
    systemChannelOptions(name, options);
    const Pose Twc = Pose::Identity();
    if (!im.empty())
    {
        auto win = sCurrentFigure3d->mMainWindow;
        auto theScene = win->get3DSceneAndLock();
        CPointCloudColoured::Ptr obj = sCurrentFigure3d->hPointCloud(name);
        PointCloud cloud;
        collectCloudFromRGBD(im, depth_pts, cloud);
        renderPointCloud(theScene, obj, Twc, cloud, options);
        if(obj) obj->setName("Color" + name);
        sCurrentFigure3d->mSysPointCloud[name] = obj;
        win->unlockAccess3DScene();
        return (hObject)(obj.get());
    }
    else
    {
        auto win = sCurrentFigure3d->mMainWindow;
        auto theScene = win->get3DSceneAndLock();
        CPointCloud::Ptr obj = sCurrentFigure3d->hMapPoint(name);
        LandMark3dV cloud;
        collectCloudFromDepth(depth_pts, cloud);
        renderMapPoints(theScene, obj, Twc, cloud, options);
        if(obj) obj->setName(name);
        sCurrentFigure3d->mSysMapPoint[name] = obj;
        win->unlockAccess3DScene();
        return (hObject)(obj.get());
    }
}

hObject viewRgbdNormals(const cv::Mat& array2d_pt3d, const cv::Mat& normals)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    const std::string& name = sysChannel[RgbdNormals];
    tOptions options;
    systemChannelOptions(name, options);

    const Pose Twc = Pose::Identity();
    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    CSetOfLines::Ptr obj = sCurrentFigure3d->hLine(name);
    Position3dV lines;
    collectLinesFromRGBDNormal(array2d_pt3d, normals, lines);
    renderLines(theScene, obj, lines, options);
    if(obj) obj->setName(name);
    sCurrentFigure3d->mSysLine[name] = obj;
    win->unlockAccess3DScene();
    return (hObject)(obj.get());
}



hObject auxViewAt(const Pose &pose)
{
    if(!sCurrentFigure3d)
		throw "None Found Figure3d";

	COpenGLViewport::Ptr &obj = sCurrentFigure3d->mGLSubView;
	auto win = sCurrentFigure3d->mMainWindow;
	auto theScene = win->get3DSceneAndLock();
    auxViewAt(theScene, obj, pose);
	win->unlockAccess3DScene();
	return (hObject)(obj.get());
}

void update(hObject obj, const Pose& Twc)
{
    if(!sCurrentFigure3d)
        return;
    if(obj == nullptr)
        return;
    CRenderizable* o = reinterpret_cast<CRenderizable *>(obj);
    sCurrentFigure3d->lock();
    o->setPose(castPose(Twc));
    sCurrentFigure3d->unlock();
}

void repaint()
{
//	if(!sCurrentFigure3d)
//		return;
//
//  sCurrentFigure3d->mMainWindow->repaint();
  Viz::instance().repaint();
	//sCurrentFigure3d->mMainWindow->RunOnce();
}

void clear()
{
    if(!sCurrentFigure3d)
        return;

    sCurrentFigure3d->clear();
}


// ---------------------------- image display ----------------------------//
// Same as OpenCV::im_show()
hObject imshow(const string& name, const cv::Mat& im)
{
#if HAS_IMGUI == 0
    FigurePtr fig = Viz::instance().findFigure(name);
    if (!fig)
    {
        fig = std::make_shared<Figure>(name, im.cols, im.rows);
        Viz::instance().add(name, fig);
    }

    sCurrentFigure3d = fig.get();
    auto win = fig->mMainWindow;
    CImage cim = castImage_clone(im);
    win->setImageView(cim);
    win->resize((unsigned int)im.cols, (unsigned int)im.rows);
    win->repaint();
    return (hObject)fig.get();
#else
    cv::imshow(name, im);
    cv::waitKey(10);
#endif
}

hObject imshow(const string& name, const cv::Mat& im, const cv::Mat& depth)
{
    FigurePtr fig = Viz::instance().findFigure(name);
    if (!fig)
    {
        fig = std::make_shared<Figure>(name, im.cols, im.rows);
        Viz::instance().add(name, fig);
    }
    sCurrentFigure3d = fig.get();

    PointCloud cloud;
    collectCloudFromRGBD(im, depth, cloud);
    const Pose Twc = Pose::Identity();
    auto win = fig->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    CPointCloudColoured::Ptr obj;
    tOptions options;
    systemChannelOptions(sysChannel[DepthPointCloud], options);
    renderPointCloud(theScene, obj, Twc, cloud, options);
    win->unlockAccess3DScene();
    win->repaint();
    return (hObject)fig.get();
}



// ---------------------------- Engine Utils -----------------------------//
volatile Gui3dOption& Option()
{
  if (!sCurrentFigure3d)
      throw "None Found Figure3d";

	return sCurrentFigure3d->Options();
}

const std::string& workRoute()
{
	return mFileRoute;
}

const std::string& dataRoute()
{
    return mDataRoute;
}

void setDataRoute(const char* data_path)
{
	mDataRoute = std::string(data_path);
}

// save debug file
void setWorkRoute(const char* cache_path)
{
    if(cache_path)
    {
        mFileRoute = std::string(cache_path);
    }
    removeAllFile(mFileRoute);
}

void addTextMessage(double x, double y, const string &text, size_t unique_index)
{
#if HAS_IMGUI == 0
    if(!sCurrentFigure3d)
        throw "None Found Figure3d";
    auto& win = sCurrentFigure3d->mMainWindow;
    win->addTextMessage(x, y, text, TColorf(0, 1, 1), unique_index, MRPT_GLUT_BITMAP_HELVETICA_12);
#endif
}

}
