#include <gui3d/utils/cast_utils.h>
#include <gui3d/render/model_render.h>
#include <gui3d/gui.h>
#include <zconf.h>
#include <sys/stat.h>

namespace gui3d {

bool renderCartesianCoordinate(GLScenePtr theScene, CAxis::Ptr& Axis, CGridPlaneXY::Ptr& horizon, const bool visible) {
  const int AXISLength = 6;
  // Add Axis
  {
    Axis = CAxis::Create(-AXISLength, -AXISLength, -AXISLength,
                         AXISLength, AXISLength, AXISLength,
                         4, 2, true);
    Axis->setTextScale(0.25f);
    Axis->setName("CAxis");
    Axis->enableTickMarks();
    Axis->setFrequency(3);
    Axis->setVisibility(visible);
    theScene->insert(Axis);
  }
  // Add Plane XY
  {
    horizon = CGridPlaneXY::Create(-AXISLength, AXISLength, -AXISLength, AXISLength);
    horizon->setName("CXY");
    horizon->setGridFrequency(3);
    horizon->setVisibility(visible);
    theScene->insert(horizon);
  }
}

CFrustum::Ptr ModelFrame(int width, int height, float fx) {
  #define FAR_DISTANCE 0.15
  #define NEAR_DISTANCE 0.03

  float RAD2DEG = 180.0f / (float) M_PI;
  float horz_FOV_degrees = 2.0f * std::atan(width / 2.f / fx) * RAD2DEG;
  float vert_FOV_degrees = 2.0f * std::atan(height / 2.f / fx) * RAD2DEG;
  CFrustum::Ptr cam = std::make_shared<CFrustum>(
            NEAR_DISTANCE, FAR_DISTANCE,
            horz_FOV_degrees, vert_FOV_degrees, 0.2, true, false);
  return cam;
}

void ModelFrames(CSetOfObjects* objs, const PoseV& vPose, const NameV& vLabel, const tOptions& options) {
  if (!objs) return;
  if (options.method == tOptions::HOLD_OFF) objs->clear();

  int index = 0;
  for (const auto& pose : vPose) {
    CFrustum::Ptr kf = ModelFrame(640, 480, 460);
    kf->setPose(castCamera(pose));
    kf->setColor(castColor(options.color));
    kf->setScale(options.scale);
    objs->insert(kf);

    if (vLabel.empty()) continue;

    const int id = vLabel[index];
    kf->setName(cv::format("KF%d", id));
    CText::Ptr kfName = CText::Create(cv::format("%d", id));
    kfName->setPose(castPose(vPose[index++]));
    kfName->setName(cv::format("KFText%d", id));
    kfName->setScale(options.scale);
    objs->insert(kfName);
  }
}

CSetOfObjects::Ptr ModelRGBAxis(const float length) {
  CSetOfObjects::Ptr obj = CSetOfObjects::Create();

  CSimpleLine::Ptr x = CSimpleLine::Create(0.0f, 0.0f, 0.0f,
                                           length, 0.0f, 0.0f, 1.0f);
  x->setColor(TColorf(1, 0, 0));
  obj->insert(x);

  CSimpleLine::Ptr y = CSimpleLine::Create(0.0f, 0.0f, 0.0f,
                                           0.0f, length * 1.0f, 0.0f, 1.0f);
  y->setColor(TColorf(0, 1, 0));
  obj->insert(y);

  CSimpleLine::Ptr z = CSimpleLine::Create(0.0f, 0.0f, 0.0f,
                                           0.0f, 0.0f, length * 1.0f, 1.0f);
  z->setColor(TColorf(0, 0, 1));
  obj->insert(z);
  return obj;
}

CSetOfObjects::Ptr ModelRobot() {
  CSetOfObjects::Ptr cleanrobot = CSetOfObjects::Create();

  // CSimpleLine
  CSimpleLine::Ptr obj1 = CSimpleLine::Create(0.0f, 0.0f, 0.01f, 0.0f, 1.0f, 0.01f, 6.0f);
  obj1->setColor(TColorf(1, 1, 0));
  cleanrobot->insert(obj1);

  // CDisk
  CDisk::Ptr disk = CDisk::Create(1.0f, 0.8f);
  disk->setColor(TColorf(0.2f, 0.7f, 0.2f));
  cleanrobot->insert(disk);

  // Wheels
  CSimpleLine::Ptr obj3 = CSimpleLine::Create(0.5f, 0.1f, 0, 0.5f, -0.1f, 0, 6);
  obj3->setColor(TColorf(0, 0, 0));
  cleanrobot->insert(obj3);

  CSimpleLine::Ptr obj4 = CSimpleLine::Create(-0.5f, 0.1f, 0.0f, -0.5f, -0.1f, 0, 6);
  obj4->setColor(TColorf(0, 0, 0));
  cleanrobot->insert(obj4);

  cleanrobot->setScale(0.3f);
  return cleanrobot;
}

void saveToPlyFile(const std::string& name, CPointCloud::Ptr landmark, bool have_color) {
  std::vector<TPoint3D> vertices(landmark->size());
  for (int i = 0; i < landmark->size(); ++i)
    vertices[i] = landmark->getPoint(i);
  TColor color = landmark->getColor_u8();

  FILE* fp = fopen(name.c_str(), "w");
  if (!fp) return;

  const std::vector<TPoint3D>& v = vertices;
  fprintf(fp, "ply\nformat ascii 1.0\ncomment file created by arc3d!\n");
  fprintf(fp, "element vertex %ld\n", v.size());
  fprintf(fp, "property float x\nproperty float y\nproperty float z\n");
  if (have_color)fprintf(fp, "property uchar red\nproperty uchar green\nproperty uchar blue\n");
  fprintf(fp, "end_header\n");
  fflush(fp);
  //v
  for (int i = 0; i < v.size(); ++i) {
    fprintf(fp, "%f %f %f", v[i][0], v[i][1], v[i][2]);
    if (have_color) fprintf(fp, " %d %d %d", color.R, color.G, color.B);
    fprintf(fp, "\n");
  }
  fclose(fp);
}

void saveToPlyFile(const std::string& name, CPointCloudColoured::Ptr landmark, bool have_color) {
  std::vector<CPointCloudColoured::TPointColour> vertices(landmark->size());
  for (int i = 0; i < landmark->size(); ++i)
    vertices[i] = landmark->getPoint(i);

  FILE* fp = fopen(name.c_str(), "w");
  if (!fp) return;

  const std::vector<CPointCloudColoured::TPointColour>& v = vertices;
  fprintf(fp, "ply\nformat ascii 1.0\ncomment file created by arc3d!\n");
  fprintf(fp, "element vertex %ld\n", v.size());
  fprintf(fp, "property float x\nproperty float y\nproperty float z\n");
  if (have_color)fprintf(fp, "property uchar red\nproperty uchar green\nproperty uchar blue\n");
  fprintf(fp, "end_header\n");
  fflush(fp);
  //v
  for (int i = 0; i < v.size(); ++i) {
    fprintf(fp, "%f %f %f", v[i].x, v[i].y, v[i].z);
    if (have_color) fprintf(fp, " %d %d %d", (int) (255 * v[i].R), (int) (255 * v[i].G), (int) (255 * v[i].B));
    fprintf(fp, "\n");
  }
  fclose(fp);
}

void SaveScene(COpenGLScene::Ptr theScene, const std::string& route) {
  time_t t = std::time(0);
  struct tm* now = std::localtime(&t);
  string file_name;
  //the name of file is better to be determined by the system time
  file_name = cv::format("%d_%d_%d_%d_%d_%d",
                         now->tm_year + 1900,
                         now->tm_mon + 1,
                         now->tm_mday,
                         now->tm_hour,
                         now->tm_min,
                         now->tm_sec);
  const std::string saveRoute = route.empty() ? "." : route;
  if (theScene->saveToFile(saveRoute + "/" + file_name + ".3Dscene"))
    std::cout << "save theScene To " << file_name + ".3Dscene" << std::endl;

#if 0
  std::string dst_dir = saveRoute + "/" + file_name;
  SaveSceneAsPLY(theScene, dst_dir);
#endif
}

void SaveSceneAsPLY(GLScenePtr theScene, const std::string& dst_dir) {
  if (access(dst_dir.c_str(), 0) == -1) {
    printf("%s is not existing, now make dir: ", dst_dir.c_str());
    int flag = mkdir(dst_dir.c_str(), 0777);
    printf(flag == 0 ? "make successfully\n" : "make errorly\n");
  }

  size_t id = 0;
  while (theScene->getByClass<CPointCloud>(id)) {
    CPointCloud::Ptr landmarkObj = theScene->getByClass<CPointCloud>(id);
    saveToPlyFile(dst_dir + "/" + landmarkObj->getName() + ".ply", landmarkObj, true);
    id++;
  }

  id = 0;
  while (theScene->getByClass<CPointCloudColoured>(id)) {
    CPointCloudColoured::Ptr landmarkObj = theScene->getByClass<CPointCloudColoured>(id);
    saveToPlyFile(dst_dir + "/" + landmarkObj->getName() + ".ply", landmarkObj, true);
    id++;
  }
}

// -------------------------------  Render All Component -----------------------------------//
bool renderFrame(GLScenePtr theScene, CFrustum::Ptr& obj, const Pose& Twc, const tOptions& options) {
  if (!obj) {
    obj = ModelFrame(640, 480, 460);
    obj->setPlaneColor(TColor(0.85f, 0.435f, 0.86f));
    theScene->insert(obj);
  }

  obj->setColor(castColor(options.color));
  obj->setScale(options.scale);
  obj->setPose(castCamera(Twc));
}

bool renderRobot(GLScenePtr theScene, CSetOfObjects::Ptr& obj, const Pose& Twb, const tOptions& options) {
  if (!obj) {
    obj = ModelRobot();
    theScene->insert(obj);
  }

  obj->setPose(castPose(Twb));
  return true;
}

bool renderRGBAxis(GLScenePtr theScene, CSetOfObjects::Ptr& obj, const Pose& Twb, const tOptions& options) {
  if (!obj) {
    obj = ModelRGBAxis(0.02);
    theScene->insert(obj);
  }

  obj->setPose(castPose(Twb));
  return true;
}

bool renderFrames(GLScenePtr theScene, CSetOfObjects::Ptr& obj, const PoseV& vPose, const NameV& vLabels,
                  const tOptions& options) {
  if (vPose.empty())
    return false;

  if (!obj) {
    obj = CSetOfObjects::Create();
    theScene->insert(obj);
  }
  ModelFrames(obj.get(), vPose, vLabels, options);
  return true;
}

bool renderLines(GLScenePtr theScene, CSetOfLines::Ptr& obj, const Position3dV& vvPoints, const tOptions& options) {
  assert(vvPoints.size() % 2 == 0);
  if (!vvPoints.empty() && vvPoints.size() >= 2) {
    std::vector<TSegment3D> vLines;
    for (int i = 0; i < vvPoints.size(); i = i + 2)
      vLines.emplace_back(TSegment3D(castPoint(vvPoints[i]), castPoint(vvPoints[i + 1])));

    if (!obj) {
      obj = CSetOfLines::Create(vLines);
      theScene->insert(obj);
    }
    if (options.method == tOptions::HOLD_OFF) obj->clear();

    obj->setColor(castColor(options.color));
    obj->setLineWidth(options.scale);
    obj->appendLines(vLines);
    return true;
  }
  return false;
}

bool renderMapPoints(GLScenePtr theScene, CPointCloud::Ptr& obj, const Pose& Twc, const LandMark3dV& vPoints,
                     const tOptions& options) {
  if (!obj) {
    if (vPoints.empty())
      return false;

    obj = CPointCloud::Create();
    obj->setPointSize(1.0);
    obj->enablePointSmooth();
    theScene->insert(obj);
  }
  if (options.method == tOptions::HOLD_OFF) obj->clear();
  if (vPoints.empty()) obj->clear();
  obj->setColor(castColor(options.color));
  obj->setPointSize(options.scale);
  obj->setPose(castPose(Twc));
  for (const auto& pt : vPoints)
    obj->insertPoint(pt(0), pt(1), pt(2));
  return true;
}

bool renderPointCloud(GLScenePtr theScene, CPointCloudColoured::Ptr& obj, const Pose& Twc, const PointCloud& vPoints,
                      const tOptions& options) {
  if (vPoints.empty())
    return false;

  if (!obj) {
    obj = CPointCloudColoured::Create();
    obj->setPointSize(1.0);
    obj->enablePointSmooth();
    theScene->insert(obj);
  }

  if (options.method == tOptions::HOLD_OFF) {
    obj->clear();
    obj->resize(vPoints.size());
    for (int i = 0; i < (int) vPoints.size(); ++i) {
      CPointCloudColoured::TPointColour pt(vPoints[i](0), vPoints[i](1), vPoints[i](2),
                                           vPoints[i](3), vPoints[i](4), vPoints[i](5));
      obj->setPoint(i, pt);
    }
  }
  else {
    obj->reserve(obj->size() + vPoints.size());
    for (int i = 0; i < (int) vPoints.size(); ++i)
      obj->push_back(vPoints[i](0), vPoints[i](1), vPoints[i](2),
                     vPoints[i](3), vPoints[i](4), vPoints[i](5));
  }
  obj->setScale(options.scale);
  obj->setPose(castPose(Twc));
  return true;
}

} // namespace gui3d

