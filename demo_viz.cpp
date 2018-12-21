#define CLOSE_LOG
#include <mrpt/opengl/CAxis.h>
#include <vslam/viz/all.hpp>
#include <xgl/all.hpp>
#include <gui3d/gui.h>
#include <gui3d/base/log.h>
#include <gui3d/base/io.h>
#include <Eigen/Dense>
#include <thread>
#include <ml.h>


using namespace xgl;
#define READ_SCENE

using namespace gui3d;
using namespace vslam;

bool ReadData(const char* filename, PoseV& _vWindowKeyframePoses, LandMark3dV& _vRefMapPoints);
bool ReadTrajectory(const char* filename, PoseV& _vWindowKeyframePoses);
bool ReadGT(const char* filename, PoseV& _vWindowKeyframePoses);
std::vector<int > range(int N);

int main(int argc, char **argv)
{
    string name = Engine();
    auto win = viz::namedWindow("Viz Demo");
    win->setBackgroundColor(xgl::Color::mlab());

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

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

    {
        auto kf_container = new viz::Collection();
        for(int i = 0; i < _vWindowKeyframePoses.size(); i++)
        {
            Eigen::Matrix4f Pose = _vWindowKeyframePoses[i].transpose();
            xgl::Matrix4f T;
            std::memcpy(T.data, Pose.data(), 16* sizeof(float));
            auto obj = new viz::Frustum(T);
            obj->setColor(xgl::Color::red());
            obj->setShowName(true);
            obj->setName(cv::format("KF%d", i));
            obj->setScale(xgl::Vector3f(2, 2, 2));
            kf_container->addObject(obj);
        }
        win->showObject("KeyFrames", kf_container);
    }

    {
        auto obj = new viz::Points(5.f);
        obj->setColor(xgl::Color::yellow());
        for(int i = 0; i < _vRefMapPoints.size(); i++)
        {
            Eigen::Vector3f pt = _vRefMapPoints[i];
            obj->push_back(xgl::Vector3f(pt(0), pt(1), pt(2)));
        }
        win->showObject("MapPoints", obj);
    }

    viz::spin();

    return 0;
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
