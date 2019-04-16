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
#include <gui3d/render/figure.h>
#include <gui3d/render/model_render.h>
#include <gui3d/render/style.h>
#include <gui3d/base/timer.h>
#include <gui3d/base/log.h>
#include <gui3d/base/io.h>

#if HAS_IMGUI
#include <GLFW/glfw3.h>
#endif

namespace gui3d {

Figure* sCurrentFigure3d = nullptr;
// Engine
class Viz
{
    std::map<string, FigurePtr> sSystemFigure3d;

public:
    static Viz& instance()
    {
        static Viz ins;
        return ins;
    }

    Viz()
    {
#if HAS_IMGUI
        glfwInit();
#endif
    }

    ~Viz()
    {
        sSystemFigure3d.clear();
#if HAS_IMGUI
        glfwTerminate();
#endif
    }

    void add(const std::string& name, FigurePtr fig)
    {
        sSystemFigure3d[name] = fig;
        sCurrentFigure3d = fig.get();
    }

    FigurePtr findFigure(const std::string& name)
    {
        auto it = sSystemFigure3d.find(name);
        if(it != sSystemFigure3d.end())
        {
            return it->second;
        }
        return nullptr;
    }

    void setAsCurrentFigure(hObject fig)
    {
        if(!bValidFigure(fig))
        {
            LOGE("None Valid Figure!!");
            return;
        }
        sCurrentFigure3d = (Figure *)fig;
    }

    void destory(const std::string& name)
    {
        if (sSystemFigure3d.empty())
            return;

        auto it = sSystemFigure3d.find(name);
        if(it != sSystemFigure3d.end())
        {
            LOGD("Destory Figure: %s", name.c_str());
            FigurePtr fig = it->second;
            if (fig.get() == sCurrentFigure3d)
                sCurrentFigure3d = nullptr;

            sSystemFigure3d.erase(it);
            csCurFigureFromExistWindows();
            return;
        }
        LOGI("None Figure: %s", name.c_str());
    }

    void destory(hObject hfig)
    {
        if(!bValidFigure(hfig))
            return;

        Figure* fig = (Figure*)hfig;
        destory(fig->name);
    }

private:
    // when delete figure, need choose and set sCurrentFigure3d
    void csCurFigureFromExistWindows()
    {
        if(sSystemFigure3d.empty())
        {
            sCurrentFigure3d = nullptr;
            LOGI("None sCurrentFigure3d!");
            return;
        }

        // Engine Main Figure as sCurrentFigure3d
        string AppName = Engine();
        FigurePtr win = Viz::instance().findFigure(AppName);
        if(win)
        {
            sCurrentFigure3d = win.get();
            //LOGI("set sCurrentFigure3d: %s", win.c_str());
            return;
        }

        // Latest Figure3d as sCurrentFigure3d
        int max_id = -1;
        for(auto& item : sSystemFigure3d)
        {
            if(item.second->mFigureID > max_id)
            {
                max_id = item.second->mFigureID;
                sCurrentFigure3d = item.second.get();
                LOGI("set sCurrentFigure3d: %s", item.first.c_str());
            }
        }
    }

    bool bValidFigure(hObject fig)
    {
        if(fig == nullptr) return false;
        for(auto& item : sSystemFigure3d)
        {
            if(fig == item.second.get())
                return true;
        }
        return false;
    }

};

std::string mFileRoute =
#ifdef _WIN32
    "D:/gitRespo/pratice/ImCache";
#else
    "/media/oyg5285/developer/gitRespo/pratice/ImCache";
#endif
std::string mDataRoute = mFileRoute;

void registerSystemChannelOptions();
bool systemChannelOptions(const Channel& name, tOptions& options);

// ----------------------- Engine Handle ----------------------------//
// set fig as sCurrentFigure3d
void setAsCurrentFigure(hObject fig)
{
    Viz::instance().setAsCurrentFigure(fig);
}

void destoryFigure(const string& name)
{
    Viz::instance().destory(name);
}

void destoryFigure(hObject hfig)
{
    Viz::instance().destory(hfig);
}

// ----------------------- Figure Handle ----------------------------//
hObject nFigure(const string& name, int width, int height)
{
    if (width < 0) width = MainWidth;
    if (height < 0) height = MainHeight;
    FigurePtr win = Viz::instance().findFigure(name);
    if(win)
    {
        win->mMainWindow->resize(width, height);
        return (hObject)win.get();
    }

    registerSystemChannelOptions();
    LOGD("New     Figure: %s", name.c_str());
    FigurePtr fig = std::make_shared<Figure>(name, width, height);
    Viz::instance().add(name, fig);
    return (hObject)fig.get();
}

void moveFigure(int x, int y)
{
	if (!sCurrentFigure3d)  return;
    auto& win = sCurrentFigure3d->mMainWindow;
    win->setPos(x, y);
}

void play_control()
{
    if(!sCurrentFigure3d)
        throw "None Found Figure3d";

    volatile Gui3dOption& guiOpt = sCurrentFigure3d->mOption;
    auto &bReadNextFrame = guiOpt.conOpt.ReadNextFrame;
    auto &ReadFrameGap   = guiOpt.conOpt.ReadFrameGap;
    auto &RequestToRefresh3DView = guiOpt.figOpt.RequestToRefresh3DView;

    auto& win = sCurrentFigure3d->mMainWindow;
    bool bNeedRefresh3DView = true;
    if(ReadFrameGap > 0 )  ReadFrameGap--;
    do
    {
        if(RequestToRefresh3DView || bNeedRefresh3DView)
        {
            win->repaint();
            RequestToRefresh3DView = false;
            bNeedRefresh3DView = false;
        }
        usleep(1000 * 10);
    }
    while (!(bReadNextFrame || ReadFrameGap > 0));
}

void waitKey(int delay_ms)
{
	if(!sCurrentFigure3d)
		throw "None Found Figure3d";

    volatile Gui3dOption& guiOpt = sCurrentFigure3d->mOption;
    volatile bool &bWaitKey = guiOpt.figOpt.bWaitKey;
    volatile bool &bExit    = guiOpt.figOpt.bExit;
    volatile bool &RequestToRefresh3DView = guiOpt.figOpt.RequestToRefresh3DView;

#if HAS_IMGUI == 0
	auto& win = sCurrentFigure3d->mMainWindow;
	win->addTextMessage(TEXT_RUN_STATE_X, TEXT_RUN_STATE_Y,
                        "VSLAM Stop", TColorf(1, 0, 0),
                        TextID::RUN_STATE, MRPT_GLUT_BITMAP_HELVETICA_12);
    win->repaint();
#endif
    gui3d::Timer timer;
    do {
        if (RequestToRefresh3DView) {
            repaint();
            RequestToRefresh3DView = false;
        }

        if (bWaitKey || bExit)
            break;

        if (delay_ms > 0 && timer.timeSinceStart() * 1e-3 > delay_ms) {
            break;
        }
    }
	while (1);
    bWaitKey = false;
    bExit = false;
}

void waitExit(hObject hfig, int delay_ms)
{
    Figure* fig = (Figure *)(hfig);
    volatile Gui3dOption& guiOpt = fig->mOption;
    auto &bExit = guiOpt.figOpt.bExit;
    auto &RequestToRefresh3DView = guiOpt.figOpt.RequestToRefresh3DView;
    auto& win = fig->mMainWindow;

#if HAS_IMGUI == 0
    win->addTextMessage(TEXT_RUN_STATE_X, TEXT_RUN_STATE_Y,
                        "VSLAM Stop", TColorf(1, 0, 0),
                        TextID::RUN_STATE, MRPT_GLUT_BITMAP_HELVETICA_12);
    win->repaint();
#endif
    gui3d::Timer timer;
    do {
        if (RequestToRefresh3DView) {
            win->repaint();
            RequestToRefresh3DView = false;
        }

        if (bExit)
            break;

        if (delay_ms > 0 && timer.timeSinceStart() * 1e-3 > delay_ms)
            break;
    }
    while (1);
}

// ----------------------- Render Handle ----------------------------//
hObject renderFrame(const Channel& name, const Pose& Twc, const tOptions& options)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

    CFrustumPtr obj = sCurrentFigure3d->hFrame(name);
    auto win = sCurrentFigure3d->mMainWindow;
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

    CSetOfObjectsPtr obj = sCurrentFigure3d->hPoseList(name);
    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    renderFrames(theScene, obj, vTwc, vLabels, real_options);
    if(obj) obj->setName(name);
    win->unlockAccess3DScene();
    sCurrentFigure3d->mSysPoseList[name] = obj;
    return (hObject)(obj.get());
}

hObject renderLines(const Channel& name, const Position3dV& vPoint, const tOptions& options)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

    CSetOfLinesPtr obj = sCurrentFigure3d->hLine(name);
    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    renderLines(theScene, obj, vPoint, real_options);
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
    return renderLines(name, vLines, options);
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
    hObject h = renderLines(name, vLines, options);
    CSetOfLines* obj = (CSetOfLines*)(h);
    obj->setPose(castPose(Twq));
    return h;
}

hObject renderMapPoints(const Channel& name, const LandMark3dV& vPoint,const tOptions& options)
{
	if(!sCurrentFigure3d)
		nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

	CPointCloudPtr obj = sCurrentFigure3d->hMapPoint(name);
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

    CPointCloudColouredPtr obj = sCurrentFigure3d->hPointCloud(name);
	auto win = sCurrentFigure3d->mMainWindow;
	auto theScene = win->get3DSceneAndLock();
    const Pose Twc = Pose::Identity();
    renderPointCloud(theScene, obj, Twc, cloud, real_options);
    if(obj) obj->setName(name);
    win->unlockAccess3DScene();
    sCurrentFigure3d->mSysPointCloud[name] = obj;
    return (hObject)(obj.get());
}

hObject renderModel3d(const Channel& name, const PoseV& vTwc, const PointCloudV& cloud, const tOptions& options)
{
    assert(vTwc.size() == cloud.size());
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    tOptions real_options = options;
    systemChannelOptions(name, real_options);

    CSetOfObjectsPtr obj = sCurrentFigure3d->hModel3d(name);
    if(obj) obj->clear();
    else    obj = CSetOfObjects::Create();
    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    for(int i = 0; i < vTwc.size(); i++)
    {
        CPointCloudColouredPtr o;
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

	CSetOfObjectsPtr obj = sCurrentFigure3d->hRobot(name);
	auto win = sCurrentFigure3d->mMainWindow;
	auto theScene = win->get3DSceneAndLock();
	renderRobot(theScene, obj, Twb, real_options);
    if(obj) obj->setName(name);
    win->unlockAccess3DScene();
	sCurrentFigure3d->mSysRobot[name] = obj;
	return (hObject)(obj.get());
}

hObject viewImage(const cv::Mat &im)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    COpenGLViewportPtr &obj = sCurrentFigure3d->mGLViewImage;
    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    viewImage(theScene, obj, im);
    win->unlockAccess3DScene();

    static size_t num = 0;
    num++;
    auto &bCacheIm = sCurrentFigure3d->mOption.conOpt.bCacheIm;
    if (bCacheIm)
        cv::imwrite(mFileRoute + cv::format("/%d.jpg", num), im);
    return (hObject)(obj.get());
}

hObject viewDepth(const cv::Mat& im, const cv::Mat& depth_pts)
{
    if(!sCurrentFigure3d)
        nFigure("default", 640, 480);

    const std::string& name = sysChannel[DepthPointCloud];
    tOptions options;
    systemChannelOptions(name, options);

    CPointCloudColouredPtr obj = sCurrentFigure3d->hPointCloud(name);
    auto win = sCurrentFigure3d->mMainWindow;
    auto theScene = win->get3DSceneAndLock();
    const Pose Twc = Pose::Identity();
    PointCloud cloud;
    collectCloudFromRGBD(im, depth_pts, cloud);
    renderPointCloud(theScene, obj, Twc, cloud, options);
    win->unlockAccess3DScene();
    sCurrentFigure3d->mSysPointCloud[name] = obj;
    return (hObject)(obj.get());
}

hObject auxViewAt(const Pose &pose)
{
    if(!sCurrentFigure3d)
		throw "None Found Figure3d";

	COpenGLViewportPtr &obj = sCurrentFigure3d->mGLSubView;
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
	if(!sCurrentFigure3d)
		return;

	sCurrentFigure3d->mMainWindow->repaint();
}

void clear()
{
    if(!sCurrentFigure3d)
        return;

    auto& theScene = sCurrentFigure3d->mMainWindow->get3DSceneAndLock();
    theScene->clear();
    sCurrentFigure3d->mMainWindow->unlockAccess3DScene();
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
    win->setImageView_fast(cim);
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
    CPointCloudColouredPtr obj;
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
