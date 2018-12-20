#include <gui3d/render/observer.h>
#include <gui3d/render/options.hpp>
#include <gui3d/render/figure.h>
#include <gui3d/render/model_render.h>
#include <gui3d/render/style.h>

const char* MSG_HELP_WINDOW =
    "These are the supported commands:          \n"
        " - h     : Toogle help view             \n"
        " - x.y/z : Switch view/hide <Ground PlaneXY/Axis3d>\n"
		" - k/w   : Switch view/hide <KeyFrames/WinKeyFrames>\n"
        " - g/r   : Switch view/hide <GlobalMapPoints/RefMapPoints>\n"
        " - n     : Switch view/hide KeyFrameNames\n"
        " - c     : Switch view/hide Connetions\n"
		" - d     : Switch view/hide Depth PointCloud\n"
        " - t     : Switch view/hide ViewPorts\n"
        " - p     : Render/Delete AprilTags\n"
        " - i     : Cache viewImage's Image\n"
        " - +/-   : Increase/reduce size of Camera\n"
		" - Space : Stop SLAM Program\n"
		" - Right : Step SLAM Program\n"
        " - s     : Save 3D scene to file\n"
        " - F5    : Refresh 3D scene\n"
        " - q|ESC : Quit\n"
        " - Alt+Enter: Toogle fullscreen\n"
;

namespace gui3d {

void GuiObserver::Help()
{
    // Show small message in the corner
    gl_utils::renderMessageBox(
        0.06f,  0.94f,  // x,y (in screen "ratios")
        0.19f, 0.05f, // width, height (in screen "ratios")
        "Press 'h' for help",
        0.015f  // text size
    );

    // Also showing help
    if (showing_help || hiding_help)
    {
        static const double TRANSP_ANIMATION_TIME_SEC = 0.5;

        const double show_tim = tim_show_start.Tac();
        const double hide_tim = tim_show_end.Tac();

        const double transparency = hiding_help ?
                                    1.0-std::min(1.0,hide_tim/TRANSP_ANIMATION_TIME_SEC)
                                                :
                                    std::min(1.0,show_tim/TRANSP_ANIMATION_TIME_SEC);

        gl_utils::renderMessageBox(
            0.05f,  0.05f,  // x,y (in screen "ratios")
            0.50f, 0.50f, // width, height (in screen "ratios")
            MSG_HELP_WINDOW,
            0.015f,  // text size
            TColor(190,190,190, (uint8_t)(200*transparency)),   // background
            TColor(0,0,0, (uint8_t)(200*transparency)),  // border
            TColor(200,0,0, (uint8_t)(150*transparency)), // text
            5.0f, // border width
            "serif", // text font
            NICE // text style
        );

        if (hide_tim > TRANSP_ANIMATION_TIME_SEC && hiding_help)
            hiding_help = false;
    }
}

/**
 *
 * @param ev
 * @note
 *        Handle Event, for keyboard and mouse
 */
void GuiObserver::DoEvent(mrptEventWindowChar ev)
{
    volatile FigureOption  &fig_option   = m_figure->mOption.figOpt;
	volatile ControlOption &con_option   = m_figure->mOption.conOpt;
	volatile SceneOption   &scene_option = m_figure->mOption.sceneOpt;
    auto win = m_figure->mMainWindow;

    auto &bExit         = fig_option.bExit;
    auto &bAxis3d       = fig_option.bAxis3d;
    auto &AxisFrq       = fig_option.AxisFrq;
    auto &bPlaneXY      = fig_option.bPlaneXY;
    auto &bViewPort = fig_option.bViewPort;
    auto &bViewMapPoint = fig_option.bViewMapPoint;
    auto &bViewFrames   = fig_option.bViewFrames;
    auto &bSave3DScene  = fig_option.bSave3DScene;
    auto &bVideoCapture = fig_option.bVideoCapture;
    auto &RequestToRefresh3DView = fig_option.RequestToRefresh3DView;

	auto &bViewKeyframes = scene_option.bViewKeyframes;
    auto &bViewPoseNames = scene_option.bViewPoseNames;
    auto &bViewLocalFrames = scene_option.bViewLocalFrames;
    auto &bViewGlobalMapPoints = scene_option.bViewGlobalMapPoints;
    auto &bViewRefMapPoints = scene_option.bViewRefMapPoints;
    auto &bViewPointCloud = scene_option.bViewPointCloud;
    auto &bOpenOptimizerPlot = scene_option.bOpenOptimizerPlot;
    auto &bViewAprilTags = scene_option.bViewAprilTags;
	auto &KFScale = scene_option.KFScale;

	auto &bWaitKey = fig_option.bWaitKey;
	auto &ReadNextFrame = con_option.ReadNextFrame;
	auto &ReadFrameGap = con_option.ReadFrameGap;
	auto &bCacheIm = con_option.bCacheIm;

    bool rebuild_3d_obj = false;
    int eventType = 0;
    switch (ev.char_code)
    {
        case 'h':
        case 'H':
            if (!showing_help) {
                tim_show_start.Tic();
                showing_help = true;
            }
            else {
                tim_show_end.Tic();
                showing_help = false;
                hiding_help = true;
            }
            RequestToRefresh3DView = true;
            break;

        case 'q':
        case 'Q':
        case MRPTK_ESCAPE:
            bExit = true;
            break;

		// PlanXY
        case 'x':
        case 'X':
        case 'y':
        case 'Y':
        {
            bPlaneXY ^= true;
			m_figure->lock();
            VisiableModel(m_figure->mGridXY, bPlaneXY);
			m_figure->unlock();
            RequestToRefresh3DView = true;
        }
            break;

		// Axis3d
        case 'z':
        case 'Z':
        {
            bAxis3d ^= true;
			m_figure->lock();
            VisiableModel(m_figure->mAxis3d, bAxis3d);
			m_figure->unlock();
            RequestToRefresh3DView = true;
        }
            break;

        // Save
        case 's':
        case 'S':
		{
            bSave3DScene = true;
		    m_figure->lock();
            SaveScene(m_figure->mScene);
			m_figure->unlock();
		}
            break;

		// Zoom Axis3d
        case MRPTK_DOWN:
		{
			AxisFrq *= 0.5;
			m_figure->lock();
            ZoomAxis(m_figure->mScene, AxisFrq);
			m_figure->unlock();
            RequestToRefresh3DView = true;
		}
            break;

        case MRPTK_UP:
		{
			AxisFrq *= 2.0;
			m_figure->lock();
            ZoomAxis(m_figure->mScene, AxisFrq);
			m_figure->unlock();
            RequestToRefresh3DView = true;
		}
            break;

		// RefMapPoints
        case 'r':
        case 'R':
        {
            CPointCloudPtr obj = m_figure->hMapPoint(sysChannel[RefMapPoints]);
            bViewRefMapPoints ^= true;
            m_figure->lock();
            VisiableModel(obj, bViewRefMapPoints);
            m_figure->unlock();
            RequestToRefresh3DView = true;
        }
            break;

		// GlobalMapPoints
		case 'g':
		case 'G':
        {
            CPointCloudPtr obj = m_figure->hMapPoint(sysChannel[GlobalMapPoints]);
            bViewGlobalMapPoints ^= true;
            m_figure->lock();
            VisiableModel(obj, bViewGlobalMapPoints);
            m_figure->unlock();
            RequestToRefresh3DView = true;
        }
            break;

		// LocalFrames or WinFrames
        case 'w':
		case 'W':
        {
            CSetOfObjectsPtr obj = m_figure->hPoseList(sysChannel[LocalFrames]);
            bViewLocalFrames ^= true;
            m_figure->lock();
            VisiableModel(obj, bViewLocalFrames);
            m_figure->unlock();
            RequestToRefresh3DView = true;
        }
            break;

		// KeyFrames
		case 'k':
		case 'K':
        {
            CSetOfObjectsPtr obj = m_figure->hPoseList(sysChannel[KeyFrames]);
            bViewKeyframes ^= true;
            m_figure->lock();
            VisiableModel(obj, bViewKeyframes);
            m_figure->unlock();
            RequestToRefresh3DView = true;
        }

            break;

		// Depth PointCloud
		case 'd':
		case 'D':
        {
            CPointCloudColouredPtr obj = m_figure->hPointCloud(sysChannel[DepthPointCloud]);
            bViewPointCloud ^= true;
            m_figure->lock();
            VisiableModel(obj, bViewPointCloud);
            m_figure->unlock();
            RequestToRefresh3DView = true;
        }
            break;

        // Text
        case 'n':
        case 'N':
		{
		    bViewPoseNames ^= true;
            m_figure->lock();
            VisibleText(m_figure->mScene, bViewPoseNames);
            m_figure->unlock();
		}
            break;

		// Zoom KeyFrames Size
		case '=':  //'+'
        {
            KFScale *= 1.2;
            rebuild_3d_obj = true;
            eventType = eventType | EVENT_TYPE ::RESCALE_KF;
        }
            break;
        case '-': //'-'
        {
            KFScale *= 1.0/1.2;
            rebuild_3d_obj = true;
            eventType = eventType | EVENT_TYPE ::RESCALE_KF;
        }
            break;

        case 't':
        case 'T':
        {
            bViewPort ^= true;
            m_figure->mGLViewImage->setTransparent(bViewPort);
            m_figure->mGLSubView->setTransparent(bViewPort);
            RequestToRefresh3DView = true;
        }
            break;

        case 'p':
        case 'P':
        {
            bViewAprilTags ^= true;
            RequestToRefresh3DView = true;
        }
            break;

        case 'e':
        case 'E':
        {
            if(bVideoCapture)
                m_figure->mMainWindow->captureImagesStop();
            else
                m_figure->mMainWindow->captureImagesStart();
            bVideoCapture ^= true;
        }
            break;


        case 'i':
        case 'I':
            bCacheIm ^= true;
            break;

        case MRPTK_SPACE:
        {
            ReadNextFrame ^= true;
			win->addTextMessage(TEXT_RUN_STATE_X, TEXT_RUN_STATE_Y,
	                    ReadNextFrame ? "VSLAM Run" : "VSLAM Stop",
						ReadNextFrame ? TColorf(0, 0, 1) : TColorf(1, 0, 0),
                        TextID::RUN_STATE, MRPT_GLUT_BITMAP_HELVETICA_12);
            RequestToRefresh3DView = true;
        }
            break;

        case MRPTK_RIGHT:
        {
            ReadFrameGap += FRAME_GAP_LENGTH;
            win->addTextMessage(TEXT_RUN_STATE_X, TEXT_RUN_STATE_Y, "VSLAM Step", TColorf(0,0,1), TextID::RUN_STATE, MRPT_GLUT_BITMAP_HELVETICA_12);
            RequestToRefresh3DView = true;
        }
            break;

        case MRPTK_F5:
        {
            RequestToRefresh3DView = true;
        }
            break;

        default:
            bWaitKey ^= true;
            printf("Key pushed: %c\n", ev.char_code);
    };

    if (rebuild_3d_obj)
    {
		if(eventType | EVENT_TYPE::RESCALE_KF)
        {
			CSetOfObjectsPtr obj = m_figure->hPoseList(sysChannel[KeyFrames]);
			m_figure->lock();
            ZoomObjects(obj, KFScale);
            m_figure->unlock();
        }
        RequestToRefresh3DView = true;
    }
}

void GuiObserver::OnEvent(const mrptEvent &e)
{
    if (e.isOfType<mrptEventWindowChar>())
    {
        const mrptEventWindowChar * ev = e.getAs<mrptEventWindowChar>();
        DoEvent(*ev);
    }
    else if (e.isOfType<mrptEventGLPostRender>())
    {
        Help();
    }
    else if (e.isOfType<mrptEventWindowResize>())
    {
        const mrptEventWindowResize &ee = static_cast<const mrptEventWindowResize &>(e);
        m_figure->lock();
        if(m_figure->mGLViewImage)
            m_figure->mGLViewImage->setViewportPosition(ee.new_width - VideoWidth,
                                                        ee.new_height - VideoHeight,
                                                        VideoWidth,
                                                        VideoHeight);
        if(m_figure->mGLSubView)
            m_figure->mGLSubView->setViewportPosition(ee.new_width  -  VP1_Width,
                                                      ee.new_height -  VideoHeight - VideoHeight- WIN_SPACE,
                                                      VP1_Width, VP1_Height);
        m_figure->unlock();
    }
    else if (e.isOfType<mrptEventWindowClosed>())
    {
        const mrptEventWindowClosed &ee = static_cast<const mrptEventWindowClosed &>(e);
        auto &bExit         = m_figure->mOption.figOpt.bExit;
        auto &bVideoCapture = m_figure->mOption.figOpt.bVideoCapture;

        bExit = true;
        if(bVideoCapture)
            m_figure->mMainWindow->captureImagesStop();
    }
}

} // namespace gui3d

