#include <memory>
#include <gui3d/render/options.hpp>
#include <gui3d/utils/dependency.hpp>

namespace gui3d{

class GuiObserver;
class Figure;
using GuiObserverPtr = std::shared_ptr<GuiObserver>;
using FigurePtr = std::shared_ptr<Figure>;

class Figure
{
public:
    Figure(const std::string& name, int width = 640, int height = 480);

	// get Figure3d's hObject
    CFrustumPtr            hFrame(const Channel& name);
    CSetOfObjectsPtr       hRobot(const Channel& name);
    CSetOfObjectsPtr       hPoseList(const Channel& name);
    CPointCloudPtr         hMapPoint(const Channel& name);
    CPointCloudColouredPtr hPointCloud(const Channel& name);
    CSetOfLinesPtr         hLine(const Channel& name);
    CSetOfObjectsPtr       hModel3d(const Channel& name);

    void lock()   { mMainWindow->get3DSceneAndLock(); }
    void unlock() { mMainWindow->unlockAccess3DScene(); }

public:
    int mFigureID;
	string name;
	static int mNextID;

public:
    CDisplayWindow3DPtr mMainWindow;

    COpenGLScenePtr mScene;
    CAxisPtr mAxis3d;
    CGridPlaneXYPtr mGridXY;

	COpenGLViewportPtr mGLViewImage;
	COpenGLViewportPtr mGLSubView;

    std::map<Channel, CFrustumPtr>            mSysFrame;
    std::map<Channel, CSetOfObjectsPtr>       mSysRobot;
    std::map<Channel, CSetOfObjectsPtr>       mSysPoseList;
    std::map<Channel, CPointCloudPtr>         mSysMapPoint;
    std::map<Channel, CPointCloudColouredPtr> mSysPointCloud;
    std::map<Channel, CSetOfLinesPtr>         mSysLine;
    std::map<Channel, CSetOfObjectsPtr>       mSysModel3d;

    GuiObserverPtr mObserver;
    volatile Gui3dOption mOption;
};


}
