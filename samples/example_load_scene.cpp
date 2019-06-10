#define CLOSE_LOG
#include <gui3d/gui.h>
#include <gui3d/base/log.h>
#include <gui3d/base/io.h>
#include <Eigen/Dense>
#include <gui3d/render/parameter.h>
#include <thread>

#define READ_SCENE

using namespace gui3d;
bool ReadData(const char* filename, PoseV& _vWindowKeyframePoses, LandMark3dV& _vRefMapPoints);
bool ReadTrajectory(const char* filename, PoseV& _vWindowKeyframePoses);
bool ReadGT(const char* filename, PoseV& _vWindowKeyframePoses);
std::vector<int > range(int N);

int RunShowScene(int argc, char **argv);
int RunAppLoadScene(int argc, char **argv);

int main(int argc, char **argv)
{
    string name = Engine();
    nFigure(name, 1080, 720);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    moveFigure(820, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    volatile Gui3dOption& guiOpt = Option();
    auto &RequestToRefresh3DView = guiOpt.figOpt.RequestToRefresh3DView;
    auto &bExit = guiOpt.figOpt.bExit;

    RunShowScene(argc, argv);

    bool bNeedRefresh3DView = true;
    while (!bExit)
    {
        //if (RequestToRefresh3DView || bNeedRefresh3DView)
        {
            repaint();
            RequestToRefresh3DView = false;
            bNeedRefresh3DView = false;
        }
    }
    bExit = false;

    return 0;
}

int RunAppLoadScene(int argc, char **argv)
{
    gui3d::setDataRoute("/media/oyg5285/developer/gitRespo/data");
}

int RunShowScene(int argc, char **argv)
{
    volatile Gui3dOption& guiOpt = Option();
    auto &RequestToRefresh3DView = guiOpt.figOpt.RequestToRefresh3DView;
    auto &bExit = guiOpt.figOpt.bExit;
    auto &KFScale = guiOpt.sceneOpt.KFScale;

    std::string route = "./data/scene.txt";
    if(argc >= 2)
      route = argv[1];

    PoseV _vWindowKeyframePoses;
    LandMark3dV _vRefMapPoints;
  #ifdef READ_SCENE
    if(!ReadData(route.c_str(), _vWindowKeyframePoses, _vRefMapPoints))
    {
      printf("Usage: [exec] scene.txt\n");
      printf("please find the scene in drawer/vslam/data\n");
      return -1;
    }
  #else
    if (!ReadGT(route.c_str(), _vWindowKeyframePoses))
      {
          printf("Usage: [exec] trajectory.txt\n");
          return -1;
      }
  #endif

    KFScale *= 0.25;
    auto pos= route.find_last_of('/');
    std::string file(route.substr(pos+1));

    addTextMessage(TEXT_X, TEXT_LOCALMAP_Y, file, TextID::LOCALMAP);
    setDataRoute(route.substr(0, pos).c_str());
    renderFrames(sysChannel[LocalFrames], _vWindowKeyframePoses);
    //renderPath(sysChannel[Path], _vWindowKeyframePoses);
    renderMapPoints(sysChannel[RefMapPoints], _vRefMapPoints);
    auto Tp = _vWindowKeyframePoses[_vWindowKeyframePoses.size() - 2];
    renderFrame(sysChannel[CurCamera], Tp);
}

bool ReadData(const char* filename, PoseV& _vWindowKeyframePoses, LandMark3dV& _vRefMapPoints)
{
    using namespace io;
    char buf[CMV_MAX_BUF];
    std::fstream fp;
    fp.open(filename, std::fstream::in);
    if (fp.is_open())
    {
        // read Frames
        nop(fp);
        int winSize = readByte<int>(fp);
        for(int i = 0; i < winSize; ++i)
        {
            Pose Tp = readMxN<float,4,4>(fp);
            _vWindowKeyframePoses.push_back(Tp);
        }

        // read Features
        nop(fp);
        int n = readByte<int>(fp);
        for(int i = 0; i < n; i++)
        {
            LandMark3d pt3 = readMxN<float, 1, 3>(fp).transpose();
            _vRefMapPoints.push_back(pt3);
        }
        return true;
    }
    else
    {
        printf("File %s cannot be opened\n", filename);
        return false;
    }
}

bool ReadTrajectory(const char* filename, PoseV& _vWindowKeyframePoses)
{
    using namespace io;
    char buf[CMV_MAX_BUF];
    std::fstream fp;
    fp.open(filename, std::fstream::in);
    if (fp.is_open())
    {
        // read Frames
        nop(fp);
        while (!fp.eof())
        {
            Eigen::Matrix<float, 1, 8> data = readMxN<float, 1, 8>(fp);
            float time = data(0);
            Eigen::Vector3f p = data.block<1, 3>(0, 1);
            Eigen::Vector4f q = data.block<1, 4>(0, 4);

            Eigen::Quaternionf quat = Eigen::Quaternionf(q(3), q(0), q(1), q(2));
            Pose Tp = Pose::Identity();
            Tp.topLeftCorner(3, 3) = quat.toRotationMatrix();
            Tp.block<3, 1>(0, 3) = p;

            _vWindowKeyframePoses.push_back(Tp.transpose());
        }
        return true;
    }
    else
    {
        printf("File %s cannot be opened\n", filename);
        return false;
    }
}

bool ReadGT(const char* filename, PoseV& _vWindowKeyframePoses)
{
    using namespace io;
    char buf[CMV_MAX_BUF];
    FILE* fp = fopen(filename, "r");
    if (!fp)
    {
        printf("File %s cannot be opened\n", filename);
        return false;
    }

    // read Frames
    while(feof(fp)==0)
    {
        float time, x, y, z, qx, qy, qz, qw;
        int num = fscanf(fp, "%f,%f,%f,%f,%f,%f,%f,%f",&time, &x, &y, &z, &qx, &qy, &qz, &qw);
        Eigen::Vector3f p(x, y, z);
        Eigen::Quaternionf quat = Eigen::Quaternionf(qw, qx, qy, qz);
        Pose Tp = Pose::Identity();
        Tp.topLeftCorner(3, 3) = quat.toRotationMatrix();
        Tp.block<3, 1>(0, 3) = p;

        _vWindowKeyframePoses.push_back(Tp.transpose());
    }
    return true;
}


std::vector<int > range(int N)
{
    std::vector<int > n;
    for (int i = 0; i < N; i++)
    {
        n.push_back(i);
    }
    return n;
}
