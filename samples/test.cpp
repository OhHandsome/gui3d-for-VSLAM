#include <mrpt/gui.h>
#include <mrpt/opengl.h>
#include <gui3d/plot.h>
#include <gui3d/gui.h>
#include <gui3d/base/system.h>
#include <thread>
using namespace gui3d;
using namespace gui3d::system;

#include <opencv/highgui.h>
#include <gui3d/utils/cast_utils.h>

void test_mrpt_opengl();
void test_plots();
void test_gui3d();

int main()
{
//    test_plots();
//    test_gui3d();
    test_mrpt_opengl();
    return 0;
}

void test_plots()
{
    std::vector<float > x = line<float>(0, 3.14f*2.0f, 0.01);
    std::vector<float > y(x.size()), z(x.size());
    for(int i = 0; i < x.size(); i++)
    {
        y[i] = std::sin(x[i]);
        z[i] = std::cos(x[i]);
    }

//    using namespace mrpt;
//    using namespace mrpt::gui;
//
//    CDisplayWindowPlotsPtr win = CDisplayWindowPlots::Create("main");
//    win->plot(x, y, "r-o", "y = sin(x)");
//    win->hold_on();
//    win->plot(x, z, "g-*", "y = sin(x)");

    {
        matlab::hObject fig = matlab::plot(x, y, "r-o");
        matlab::hold_off(fig);
        matlab::plot(fig, x, z, "g-*");
    }

    {
        matlab::hObject fig = matlab::plot(x, y, "r-o");
        matlab::hold_on(fig);
        matlab::plot(fig, x, z, "g-*");
    }
}

void test_gui3d()
{
    Pose Twc = Pose::Identity();
    string name = Engine();
    hObject fig1 = nFigure(name, 1080, 720);
    renderFrame(sysChannel[CurCamera], Twc);
    waitExit(fig1);
    destoryFigure(fig1);

    {
        hObject fig2 = nFigure("Test For Gui3d", 1080, 720);
        renderFrame(sysChannel[CurCamera], Twc);
        waitExit(fig2);
        destoryFigure(fig2);
    }

//    setOutputFigure(fig1);
//    LOGI("Move ");
//    Twc(3, 0) = 2;
//    renderFrame(sysChannel[CurCamera], Twc, CurCameraOptions);
//    waitKey();

//
//    LOGI("destoryFigure1 ");
//    destoryFigure(name);
//    LOGI("destoryFigure2 ");
//    destoryFigure("Test For Gui3d");
//    LOGI("destoryFigure3 ");
//
//    renderFrame("CurFrame", Twc, options);
//    waitKey();

}

void test_mrpt_opengl()
{
    using namespace mrpt;
    using namespace mrpt::gui;
    using namespace mrpt::opengl;
    using namespace mrpt::utils;
    using namespace mrpt::math;
    using namespace mrpt::poses;

    Pose Twc = Pose::Identity();

    CDisplayWindow3DPtr win = CDisplayWindow3D::Create("main");
    COpenGLScenePtr theScene = win->get3DSceneAndLock();

    // Axis
    {
        opengl::CAxisPtr obj = opengl::CAxis::Create(-6,-6,-6, 6,6,6, 2,2, true);
        obj->setLocation(0,0,0);
        theScene->insert( obj );

        opengl::CTextPtr gl_txt = opengl::CText::Create("CAxis");
        gl_txt->setLocation(0, 1 ,0);
        theScene->insert(gl_txt);
    }

    {
        CFrustumPtr obj = CFrustum::Create();
        obj->setPose(CPose3D(1, 2, 3));
        obj->setColor(TColorf(1, 0, 0));
        theScene->insert(obj);
    }

    {
        std::string file = "/media/oyg5285/developer/gitRespo/pratice/visp/tutorial/detection/tag/AprilTag.pgm";
        cv::Mat im = cv::imread(file);
        CImage cim = gui3d::castImage(im);
//        CTexturedPlanePtr obj = CTexturedPlane::Create();
//        obj->setPose(CPose3D(1, 2, 3));
//        obj->assignImage(gui3d::castImage(im));
//        theScene->insert(obj);

        COpenGLViewportPtr gl_view_Image = theScene->createViewport("Image");
        gl_view_Image->setCloneView("main");
        gl_view_Image->setViewportPosition(0, 0, 320, 240);
        gl_view_Image->setNormalMode();
        gl_view_Image->setCloneCamera(true); //(theScene->getViewport()->getCamera());
        gl_view_Image->setImageView_fast(cim);
    }

    win->unlockAccess3DScene();
    win->repaint();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

//    {
//        CDisplayWindow3DPtr win1 = CDisplayWindow3D::Create("sub_win");
//        COpenGLScenePtr theScene1 = win1->get3DSceneAndLock();
//        CFrustumPtr obj1 = CFrustum::Create();
//        obj1->setPose(CPose3D(1, 2, 3));
//        theScene1->insert(obj1);
//        win1->unlockAccess3DScene();
//        win1->repaint();
//        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//    }

    std::cout << "Close the window to end.\n";
    while (win->isOpen())
    {
        win->addTextMessage(5,5, format("%.02fFPS", win->getRenderingFPS()));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        win->repaint();
    }
}

