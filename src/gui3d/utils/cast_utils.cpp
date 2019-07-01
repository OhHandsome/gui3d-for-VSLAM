#include <gui3d/utils/dependency.hpp>
#include <gui3d/base/type_def.h>
#include <gui3d/utils/cast_utils.h>

#define DRAW_USE_EIGEN 1
#define OPENGL_MATRIX  1
namespace gui3d {
// memory stored by rows or cols
Vector3f pickTranslation(const Pose &Twc)
{
#if OPENGL_MATRIX
    Vector3f t = Twc.block<3,1>(0, 3);
#else
    Vector3f t = Twc.transpose().block<3,1>(0, 3);
#endif
    return t;
}

Matrix3f pickRotation(const Pose& Twc)
{
#if OPENGL_MATRIX
    Matrix3f R = Twc.block<3,3>(0, 0);
#else
    Matrix3f R = Twc.transpose().block<3,3>(0, 0);
#endif
    return R;
}

Matrix4f pickPose(const Pose& Twc)
{
#if OPENGL_MATRIX
    Matrix4f T = Twc;
#else
    Matrix4f T = Twc.transpose();
#endif
    return T;
}

/*double m[16] = { 0,-1,0,0,
0,0,-1,0,
1,0,0,0,
0,0,0,1 };
CMatrixDouble44 bias = CMatrixDouble44(m);*/
Pose toPose44(const State& pose31)
{
    Pose T = Pose::Identity();
    T(0,0) = static_cast<float >(cos(pose31[2]));
    T(0,1) = static_cast<float >(-sin(pose31[2]));
    T(1,0) = static_cast<float >(sin(pose31[2]));
    T(1,1) = static_cast<float >(cos(pose31[2]));
    T(0,3) = static_cast<float >(pose31[0]);
    T(1,3) = static_cast<float >(pose31[1]);
    return T;
}

CImage castImage(const cv::Mat& srcIm)
{
    IplImage im = (IplImage)(srcIm);

    CImage o_img(srcIm.cols, srcIm.rows);
    o_img.loadFromIplImage(&im);
    return o_img;
}

CImage castImage_clone(const cv::Mat& srcIm)
{
    srcIm.clone();
    IplImage im = (IplImage)(srcIm);

    mrpt::img::CImage CIm;
    CIm.resize(srcIm.cols, srcIm.rows,
            srcIm.channels() == 3 ? TImageChannels::CH_RGB : TImageChannels::CH_GRAY);
    CIm.loadFromIplImage(&im);
    return CIm;
}

CPose3D castPose(const State& state)
{
    CMatrixDouble44 m44 = CMatrixDouble44::Identity();
    auto pose = toPose44(state);
    for(int i=0; i < 4; i++)
        for(int j=0; j<4; j++)
            m44(i,j) = pose(i, j);
    return CPose3D(m44);
}

/*
*    because of mrpt's model, adjust CFristum direction for keyFrame display
*/
CPose3D castCamera(const Pose& Twc)
{
    const double data[] = { 0., -1., 0., 0.,
            0, 0, -1, 0,
            1, 0, 0, 0,
            0, 0, 0, 1};
    CMatrixDouble44 bias(data);
    CMatrixDouble44 m44 = CMatrixDouble44::Identity();
    auto pdata = Twc.data();
#if OPENGL_MATRIX
    m44(0,0) = pdata[0]; m44(0,1) = pdata[4];m44(0,2) = pdata[8];m44(0,3) = pdata[12];
    m44(1,0) = pdata[1]; m44(1,1) = pdata[5];m44(1,2) = pdata[9];m44(1,3) = pdata[13];
    m44(2,0) = pdata[2]; m44(2,1) = pdata[6];m44(2,2) = pdata[10];m44(2,3) = pdata[14];
    m44(3,0) = pdata[3]; m44(3,1) = pdata[7];m44(3,2) = pdata[11];m44(3,3) = pdata[15];
#else
    m44(0,0) = pdata[0]; m44(0,1) = pdata[1];m44(0,2) = pdata[2];m44(0,3) = pdata[3];
    m44(1,0) = pdata[4]; m44(1,1) = pdata[5];m44(1,2) = pdata[6];m44(1,3) = pdata[7];
    m44(2,0) = pdata[8]; m44(2,1) = pdata[9];m44(2,2) = pdata[10];m44(2,3) = pdata[11];
    m44(3,0) = pdata[12]; m44(3,1) = pdata[13];m44(3,2) = pdata[14];m44(3,3) = pdata[15];
#endif
    CMatrixDouble44 m = m44 * bias;
    return CPose3D(m);
}

/**
*   cast Eigen::Matrix4f to CPose3D
*/
CPose3D castPose(const Pose& Twc_)
{
    CMatrixDouble44 m44 = CMatrixDouble44::Identity();
    auto pdata = Twc_.data();
#if OPENGL_MATRIX
    m44(0,0) = pdata[0]; m44(0,1) = pdata[4];m44(0,2) = pdata[8];m44(0,3) = pdata[12];
    m44(1,0) = pdata[1]; m44(1,1) = pdata[5];m44(1,2) = pdata[9];m44(1,3) = pdata[13];
    m44(2,0) = pdata[2]; m44(2,1) = pdata[6];m44(2,2) = pdata[10];m44(2,3) = pdata[14];
    m44(3,0) = pdata[3]; m44(3,1) = pdata[7];m44(3,2) = pdata[11];m44(3,3) = pdata[15];
#else
    m44(0,0) = pdata[0]; m44(0,1) = pdata[1];m44(0,2) = pdata[2];m44(0,3) = pdata[3];
    m44(1,0) = pdata[4]; m44(1,1) = pdata[5];m44(1,2) = pdata[6];m44(1,3) = pdata[7];
    m44(2,0) = pdata[8]; m44(2,1) = pdata[9];m44(2,2) = pdata[10];m44(2,3) = pdata[11];
    m44(3,0) = pdata[12]; m44(3,1) = pdata[13];m44(3,2) = pdata[14];m44(3,3) = pdata[15];
#endif
    return CPose3D(m44);
}

TPoint3D castPoint(const LandMark3d& point)
{
    return math::TPoint3D(point.x(), point.y(), point.z());
}

void castLines(const LandMark3dV& vvPoints, vector<TSegment3D>& vLines)
{
    vLines.clear();
    for(unsigned int i = 0; i < vvPoints.size(); i=i+2)
    {
        LandMark3d Point1 = vvPoints[i];
        LandMark3d Point2 = vvPoints[i+1];
        vLines.emplace_back(TSegment3D(castPoint(Point1), castPoint(Point2)));
    }
}

TColorf castColor(const Color& color)
{
    TColorf info(
            color(0) *(1.f/255),
            color(1) *(1.f/255),
            color(2) *(1.f/255),
            color(3) *(1.f/255));
    return info;
}

} // namespace gui




