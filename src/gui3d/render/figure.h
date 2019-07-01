#include <memory>
#include <gui3d/render/options.hpp>
#include <gui3d/utils/dependency.hpp>
#if HAS_IMGUI
#include <gui3d/window/CDisplayWindow3D.h>
#endif

namespace gui3d{

#if HAS_IMGUI == 0
class GuiObserver;
using GuiObserverPtr = std::shared_ptr<GuiObserver>;
#endif

class Figure;
using FigurePtr = std::shared_ptr<Figure>;

class Figure
{
public:
	explicit Figure(const std::string& name, int width = 640, int height = 480);

  // get Figure3d's hObject
	CFrustum::Ptr            hFrame(const Channel& name);
	CSetOfObjects::Ptr       hRobot(const Channel& name);
	CSetOfObjects::Ptr       hPoseList(const Channel& name);
	CPointCloud::Ptr         hMapPoint(const Channel& name);
	CPointCloudColoured::Ptr hPointCloud(const Channel& name);
	CSetOfLines::Ptr         hLine(const Channel& name);
	CSetOfObjects::Ptr       hModel3d(const Channel& name);
    CSetOfObjects::Ptr       hAxis3d(const Channel& name);

	void lock()   { mMainWindow->get3DSceneAndLock(); }
	void unlock() { mMainWindow->unlockAccess3DScene(); }
	void init();
    void clear();
	volatile Gui3dOption& Options();

public:
    int mFigureID;
	string name;
	static int mNextID;

public:
#if HAS_IMGUI
	gui3d::CDisplayWindow3DPtr mMainWindow;
#else
    mrpt::gui::CDisplayWindow3D::Ptr mMainWindow;

	GuiObserverPtr mObserver;
#endif
	COpenGLScene::Ptr mScene;
	CAxis::Ptr mAxis3d;
	CGridPlaneXY::Ptr mGridXY;

	COpenGLViewport::Ptr mGLViewImage;
	COpenGLViewport::Ptr mGLSubView;

	std::map<Channel, CFrustum::Ptr>            mSysFrame;
	std::map<Channel, CSetOfObjects::Ptr>       mSysRobot;
	std::map<Channel, CSetOfObjects::Ptr>       mSysPoseList;
	std::map<Channel, CPointCloud::Ptr>         mSysMapPoint;
	std::map<Channel, CPointCloudColoured::Ptr> mSysPointCloud;
	std::map<Channel, CSetOfLines::Ptr>         mSysLine;
	std::map<Channel, CSetOfObjects::Ptr>       mSysModel3d;

	std::map<Channel, CSetOfObjects::Ptr>       mSysAxis3d;

	volatile Gui3dOption mOption;
};


}
