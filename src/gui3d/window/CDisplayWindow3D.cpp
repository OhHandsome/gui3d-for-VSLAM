#include <gui3d/window/CDisplayWindow3D.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw_gl2.h>
#include <gui3d/render/model_render.h>

using namespace mrpt;
using namespace mrpt::opengl;

namespace gui3d{

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
}

static void ShowHelpMarker(const char* desc) {
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered()){
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

struct GlfwContextScopeGuard {
  explicit GlfwContextScopeGuard(GLFWwindow* win){
    prev_win = glfwGetCurrentContext();
    glfwMakeContextCurrent(win);
  }

  ~GlfwContextScopeGuard(){
    glfwMakeContextCurrent(prev_win);
  }
  GLFWwindow* prev_win;
};

struct ImGuiContextScopeGuard {
  explicit ImGuiContextScopeGuard(ImGuiContext* ctx) {
    prev_ctx = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext(ctx);
  }

  ~ImGuiContextScopeGuard() {
    ImGui::SetCurrentContext(prev_ctx);
  }

  ImGuiContext* prev_ctx;
};

CDisplayWindow3DPtr
CDisplayWindow3D::Create(const std::string &windowCaption,
                         unsigned int initialWindowWidth,
                         unsigned int initialWindowHeight) {
  return CDisplayWindow3DPtr(new CDisplayWindow3D(windowCaption, initialWindowWidth, initialWindowHeight));
}

CDisplayWindow3D::CDisplayWindow3D(const std::string &windowCaption,
                                   unsigned int initialWindowWidth,
                                   unsigned int initialWindowHeight) {
  //glfwSetErrorCallback(glfw_error_callback);

  // glfw: initialize and configure
  //glfwInit();

  // glfw window creation
  m_Window = glfwCreateWindow(initialWindowWidth, initialWindowHeight, windowCaption.c_str(), nullptr, nullptr);
  if (m_Window == nullptr) {
      std::cerr << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      exit(1);
  }

  // Make the window's context current
  GlfwContextScopeGuard gl_ctx_guard(m_Window);
  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cerr << "Failed to initialize GLAD" << std::endl;
      exit(1);
  }
  //glfwMakeContextCurrent(m_Window);
  glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

  // Setup Dear ImGui binding
  IMGUI_CHECKVERSION();
  m_ImGuiContext = ImGui::CreateContext();
  ImGuiContextScopeGuard imgui_ctx_guard(m_ImGuiContext);
  ImGui_ImplGlfwGL2_Init(m_Window, true);

  // Setup style
  ImGui::StyleColorsClassic();

  ImGuiIO& io = ImGui::GetIO(); (void)io;
  m_lastWheelRotation = io.MouseWheel;

  m_3Dscene = mrpt::opengl::COpenGLScene::Create();
  m_GlCanvas = new CGlCanvas(m_3Dscene);
  InitScene();
  RequestToRefresh3DView = true;
  glfwSetKeyCallback(m_Window, key_callback);
  glfwSetWindowUserPointer(m_Window, this);

  // must be add, sometimes crash
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  m_renderLoopThread = std::thread(&CDisplayWindow3D::backThreadRun, this);
}

CDisplayWindow3D::~CDisplayWindow3D() {
  if (m_renderLoopThread.joinable())
    m_renderLoopThread.join();

  delete m_GlCanvas;
  m_3Dscene.clear_unique();
}

mrpt::opengl::COpenGLScenePtr& CDisplayWindow3D::get3DSceneAndLock() {
  m_access3Dscene.lock();
  return m_3Dscene;
}

void CDisplayWindow3D::unlockAccess3DScene() {
  m_access3Dscene.unlock();
}

bool CDisplayWindow3D::WindowClosed() const
{
  return glfwWindowShouldClose(m_Window);
}

void CDisplayWindow3D::forceRepaint() {
  RequestToRefresh3DView = true;
}

void CDisplayWindow3D::InitScene(){
  const int AXISLength  = 6;
  // Add Axis
  {
    auto theScene = get3DSceneAndLock();
    CAxisPtr Axis = CAxis::Create(-AXISLength, -AXISLength, -AXISLength,
                                  AXISLength, AXISLength /* / 2.0 + 1*/, AXISLength /* / 2.0 + 1*/, 4, 2, true);
    Axis->setTextScale(0.25f);
    Axis->setName("CAxis");
    Axis->enableTickMarks();
    Axis->setFrequency(3);
    theScene->insert(Axis);
    m_Axis3d = Axis;
    unlockAccess3DScene();
  }
  // Add Plane XY
  {
    auto theScene = get3DSceneAndLock();
    auto XY = CGridPlaneXY::Create(-AXISLength, AXISLength, -AXISLength, AXISLength);
    XY->setName("CXY");
    XY->setGridFrequency(3);
    theScene->insert(XY);
    m_ZeroPlane = XY;
    unlockAccess3DScene();
  }
}

void CDisplayWindow3D::OnPreRender() {
  // Display panel
  static bool show_axis3d = true;
  if(ImGui::Begin("System", &show_axis3d,
                  ImGuiWindowFlags_NoScrollbar |
                  ImGuiWindowFlags_AlwaysAutoResize)) {

    // Visible about Axis3d and ZeroPlane
    get3DSceneAndLock();
    bool visible = m_Axis3d->isVisible();
    if (ImGui::Checkbox("Axis3d", &visible))
      m_Axis3d->setVisibility(visible);
    ImGui::SameLine();
    visible = m_ZeroPlane->isVisible();
    if (ImGui::Checkbox("ZeroPlane", &visible))
      m_ZeroPlane->setVisibility(visible);

    ImGui::Text("Label:");
    ImGui::SameLine();
    int freq = m_Axis3d->getFrequency();
    int v[2];
    v[0] = m_Axis3d->getFrequency();
    v[1] = m_ZeroPlane->getGridFrequency();
    ImGui::SliderInt2("FREQ", v, 1, 8); // Edit 1 Int using a slider from 1 to 8
    m_Axis3d->setFrequency(v[0]);
    m_ZeroPlane->setGridFrequency(v[1]);
    unlockAccess3DScene();
  }
  ImGui::End();

  // Display [Scene] panel
  static bool visible_all;
  static bool show_tool_panel = true;
  if (ImGui::Begin("Scene", &show_tool_panel,
                   ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoScrollbar)) {
    get3DSceneAndLock();
    COpenGLViewportPtr mainVP = m_3Dscene->getViewport();
    visible_all = true;
    for (COpenGLViewport::const_iterator itO = mainVP->begin();
         itO!= mainVP->end();
         ++itO) {
      if((*itO)->getName() == m_Axis3d->getName() ||
         (*itO)->getName() == m_ZeroPlane->getName()) // remove Axis and ZeroPlane
        continue;

      if (!(*itO)->isVisible()) {
        visible_all = false;
        break;
      }
    }

    if (ImGui::Checkbox("[ALL]", &visible_all)) {
      for (COpenGLViewport::iterator itO = mainVP->begin();
           itO!= mainVP->end();
           ++itO) {
        if((*itO)->getName() == m_Axis3d->getName() ||
           (*itO)->getName() == m_ZeroPlane->getName())
          continue;
        (*itO)->setVisibility(visible_all);
      }
    }
    ImGui::SameLine();
    ShowHelpMarker("Toggle visibility's of all objects");
    for (COpenGLViewport::iterator itO = mainVP->begin();
         itO!= mainVP->end();
         ++itO) {
      if((*itO)->getName() == m_Axis3d->getName() ||
         (*itO)->getName() == m_ZeroPlane->getName())
        continue;

      bool visible = (*itO)->isVisible();
      if (ImGui::Checkbox((*itO)->getName().c_str(), &visible))
        (*itO)->setVisibility(visible);
    }
    unlockAccess3DScene();
  }
  ImGui::End();
}

void CDisplayWindow3D::OnEyeShotRender()
{
  GlfwContextScopeGuard gl_ctx_guard(m_Window);
  ImGuiContextScopeGuard imgui_ctx_guard(m_ImGuiContext);

  ImGuiIO& io = ImGui::GetIO(); (void)io;
  if(!io.WantCaptureMouse || !io.WantCaptureKeyboard){
    MouseEvent event;
    event.setFromIO(io);

    if(io.MouseDown[0] || io.MouseDown[1]) {
      double distance = std::sqrt(io.MouseDelta[0] * io.MouseDelta[0] + io.MouseDelta[1] * io.MouseDelta[1]);
      if(distance > 1.0 && distance < 200.0) {
        m_GlCanvas->OnMouseMove(event);
        RequestToRefresh3DView = true;
      }
      m_GlCanvas->OnMouseDown(event);
    }
    else if(io.MouseReleased[0] || io.MouseReleased[1]) {
      m_GlCanvas->OnMouseUp(event);
    }

    if(event.GetWheelRotation() - m_lastWheelRotation > 0.5 ||
       event.GetWheelRotation() - m_lastWheelRotation < -0.5) {
      m_GlCanvas->OnMouseWheel(event);
      RequestToRefresh3DView = true;
    }
  }
}

void CDisplayWindow3D::OnPostRender()
{
//  ImGui::Spacing();
//  if (ImGui::CollapsingHeader("Plots")) {
//      static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
//      ImGui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));
//  }
}

void CDisplayWindow3D::backThreadRun() {

  GlfwContextScopeGuard gl_ctx_guard(m_Window);
  ImGuiContextScopeGuard imgui_ctx_guard(m_ImGuiContext);
  ImVec4 clear_color = ImVec4(0.6f, 0.6f, 0.60f, 0.00f);

  // loop
  while (!glfwWindowShouldClose(m_Window)) {
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwPollEvents();
    ImGui_ImplGlfwGL2_NewFrame();

    // input
    OnPreRender();

    // 1. Zoom-pan-rotate mouse manipulation
    OnEyeShotRender();

    // 2. Render
    //if(RequestToRefresh3DView)
    {
      get3DSceneAndLock();
      m_GlCanvas->OnPaint();
      unlockAccess3DScene();
      RequestToRefresh3DView = false;
    }
    OnPostRender();

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    ImGui::Render();
    ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_Window);
  }

  // Cleanup
  // glfw: terminate, clearing all previously allocated GLFW resources.
  {
    GlfwContextScopeGuard gl_ctx_guard1(m_Window);
    ImGuiContextScopeGuard imgui_ctx_guard1(m_ImGuiContext);
    ImGui_ImplGlfwGL2_Shutdown();
  }
  ImGui::DestroyContext();
  glfwDestroyWindow(m_Window);
  //glfwTerminate();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  void* userPointer = glfwGetWindowUserPointer(window);
  CDisplayWindow3D* window3d = (CDisplayWindow3D*) userPointer;

  volatile FigureOption& fig_option = window3d->Options().figOpt;
  volatile ControlOption& con_option = window3d->Options().conOpt;
  volatile SceneOption& scene_option = window3d->Options().sceneOpt;
  auto& ReadNextFrame = con_option.ReadNextFrame;
  auto& ReadFrameGap = con_option.ReadFrameGap;
  auto& bCacheIm = con_option.bCacheIm;

  auto& bExit = fig_option.bExit;
  auto& bViewPort = fig_option.bViewPort;
  auto& bSave3DScene = fig_option.bSave3DScene;
  auto& RequestToRefresh3DView = fig_option.RequestToRefresh3DView;

  auto& bOpenOptimizerPlot = scene_option.bOpenOptimizerPlot;
  auto& bViewAprilTags = scene_option.bViewAprilTags;
  auto& bWaitKey = fig_option.bWaitKey;

  if (action != GLFW_PRESS)
    return;

  switch (key) {
    case GLFW_KEY_ESCAPE:
    case GLFW_KEY_Q:
      glfwSetWindowShouldClose(window, true);
      bExit = true;
      break;

    case GLFW_KEY_SPACE:
      ReadNextFrame ^= true;
      break;

    case GLFW_KEY_RIGHT:
      ReadFrameGap += FRAME_GAP_LENGTH;
      break;

    case GLFW_KEY_S:
    {
      COpenGLScenePtr theScene = window3d->get3DSceneAndLock();
      gui3d::SaveScene(theScene, dataRoute());
      window3d->unlockAccess3DScene();
    }
      break;

    case 'p':
    case 'P':
    {
      bViewAprilTags ^= true;
      RequestToRefresh3DView = true;
    }
      break;

    case 'i':
    case 'I':
      bCacheIm ^= true;
      break;

    default:
      bWaitKey ^= true;
      printf("Key pushed: %c\n", key);
  };
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

/**
 * @param io
 *     mouse manipulation
 */
void MouseEvent::setFromIO(const ImGuiIO &io){
    const ImVec2 mouse_pos = io.MousePos;
    m_x = mouse_pos.x;
    m_y = mouse_pos.y;
    m_leftDown = io.MouseDown[0];
    m_rightDown = io.MouseDown[1];
    m_controlDown = io.KeyCtrl;
    m_shiftDown = io.KeyShift;
    m_wheelRotation = io.MouseWheel;
    //printf("m_x: %f, m_y: %f, m_leftDown: %s\n", m_x, m_y, m_leftDown ? "True" : "False");
}

} // namespace gui3d
