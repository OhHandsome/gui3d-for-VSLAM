#include <gui3d/base/type_def.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
namespace gui3d{
extern std::string mDataRoute;

void collectCloudFromDepth(const cv::Mat& depth, LandMark3dV& cloud)
{
    for(int y = 0; y < depth.rows; y++)
    {
        auto pdepth = depth.ptr<cv::Vec3f>(y);
        for(int x = 0; x < depth.cols; x++)
        {
            cv::Vec3f pt = pdepth[x];
            if(pt(2) == 0.0f)
                continue;

            LandMark3d info;
            info << pt(0), pt(1), pt(2);
            cloud.push_back(info);
        }
    }
}

void collectCloudFromRGBD(const cv::Mat& im, const cv::Mat& depth, PointCloud& cloud)
{
    cv::Mat color;
    if (im.channels() == 1)
        cv::cvtColor(im, color, cv::COLOR_GRAY2BGR);
    else
        color = im;

    cloud.clear();
    for(int y = 0; y < depth.rows; y++)
    {
        auto pdepth = depth.ptr<cv::Vec3f>(y);
        auto pimage = color.ptr<cv::Vec3b>(y);
        for(int x = 0; x < depth.cols; x++)
        {
            cv::Vec3f pt = pdepth[x];
            if(pt(2) == 0.0f)
                continue;

            Eigen::Matrix<float, 7, 1> info;
            info << pt(0), pt(1), pt(2),
                    pimage[x](2)/255.0f, pimage[x](1)/255.0f, pimage[x](0)/255.0f,  1.0f; // BGR
            cloud.push_back(info);
        }
    }
}

void collectLinesFromRGBDNormal(const cv::Mat& depth, const cv::Mat& normal, Position3dV& vecN, const float l)
{
    int step = 2;
    for(int y = 0; y < depth.rows; y=y+step)
    {
        auto pdepth = depth.ptr<cv::Vec3f>(y);
        auto pnormal = normal.ptr<cv::Vec3f>(y);
        for(int x = 0; x < depth.cols; x=x+step)
        {
            cv::Vec3f pt = pdepth[x];
            if(pt(2) == 0.0f)
                continue;

            cv::Vec3f n = pnormal[x];
            if (cv::norm(n) == 0.0f)
                continue;

            LandMark3d info;
            info << pt(0), pt(1), pt(2);
            vecN.push_back(info);

            pt += l * n;
            info << pt(0), pt(1), pt(2);
            vecN.push_back(info);
        }
    }
}

void saveAsPLY(const std::string& name, const Pose& Twc, const PointCloud& cloud)
{
    LandMark3dV vertices(cloud.size());
    for(int i = 0; i < cloud.size(); ++i)
    {
        vertices[i] = Twc.topLeftCorner(3, 3) * cloud[i].topRows(3) + Twc.topRightCorner(3, 1);
    }

    FILE *fp = fopen((mDataRoute + "/" + name).c_str(), "w");
    if (!fp) return;

	const LandMark3dV &v = vertices;
    bool have_color = true;
    fprintf(fp, "ply\nformat ascii 1.0\ncomment file created by arc3d!\n");
    fprintf(fp, "element vertex %ld\n", v.size());
    fprintf(fp, "property float x\nproperty float y\nproperty float z\n");
    if (have_color)fprintf(fp, "property uchar red\nproperty uchar green\nproperty uchar blue\n");
    fprintf(fp, "end_header\n");
    fflush(fp);
    //v
    for (int i = 0; i < v.size(); ++i) {
        fprintf(fp, "%f %f %f", v[i][0], v[i][1], v[i][2]);
        if (have_color)fprintf(fp, " %d %d %d", (int)(255*cloud[i][3]), (int)(255*cloud[i][4]), (int)(255*cloud[i][5]));
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void saveAsPLY(const std::string& name, const Pose& Twc, const cv::Mat& im, const cv::Mat& depth)
{
    PointCloud cloud;
    collectCloudFromRGBD(im, depth, cloud);
    saveAsPLY(name, Twc, cloud);
}




}
