#include <gui3d/window/CDisplayWindow3D.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <imgui_internal.h>
#include <gui3d/render/model_render.h>
#include <gui3d/render/style.h>

using namespace mrpt;
using namespace mrpt::opengl;
static std::list<std::function<void(void)>> destoryOpenGLResourcesOnExit;

namespace gui3d {

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

CDisplayWindow3DPtr
CDisplayWindow3D::Create(const std::string &windowCaption,
                         unsigned int initialWindowWidth,
                         unsigned int initialWindowHeight) {
  return CDisplayWindow3DPtr(new CDisplayWindow3D(windowCaption, initialWindowWidth, initialWindowHeight));
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

CDisplayWindow3D::CDisplayWindow3D(const std::string &windowCaption,
                                   unsigned int initialWindowWidth,
                                   unsigned int initialWindowHeight)
   : m_windowCaption(windowCaption)
   , m_initialWindowWidth(initialWindowWidth)
   , m_initialWindowHeight(initialWindowHeight) {

  // must be add, sometimes crash
  m_renderLoopThread = std::thread(&CDisplayWindow3D::backThreadRun, this);

  while (!m_ReadyContext) {
    std::cout << "Wait Context Ready" << std::endl;
  };
}

CDisplayWindow3D::~CDisplayWindow3D() {
  if (m_renderLoopThread.joinable())
    m_renderLoopThread.join();

  delete m_GlCanvas;
  m_3Dscene.clear_unique();
}

CDisplayImagesPtr
CDisplayWindow3D::createViewImage(const std::string &name) {
  m_subview_image = std::make_shared<CDisplayImages>(name);
  return m_subview_image;
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
    Axis->setVisibility(true);
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
    XY->setVisibility(true);
    theScene->insert(XY);
    m_ZeroPlane = XY;
    unlockAccess3DScene();
  }
}

void CDisplayWindow3D::OnPreRender() {

  // Menu
  static bool show_scene_property_editor = false;
  // Show Scene Property
  if (show_scene_property_editor)
  {
    // Display panel
    if(ImGui::Begin("System", &show_scene_property_editor,
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_AlwaysAutoResize)) {

      // Visible about Axis3d and ZeroPlane
      auto& theScene = get3DSceneAndLock();
      bool visible = m_Axis3d->isVisible();
      if (ImGui::Checkbox("Axis3d", &visible))
        m_Axis3d->setVisibility(visible);
      ImGui::SameLine();
      visible = m_ZeroPlane->isVisible();
      if (ImGui::Checkbox("XY", &visible))
        m_ZeroPlane->setVisibility(visible);

      ImGui::Text("Label:");
      ImGui::SameLine();
      int v[2];
      v[0] = m_Axis3d->getFrequency();
      v[1] = m_ZeroPlane->getGridFrequency();
      ImGui::SliderInt2("FREQ", v, 1, 8); // Edit 1 Int using a slider from 1 to 8
      m_Axis3d->setFrequency(v[0]);
      m_ZeroPlane->setGridFrequency(v[1]);

      /*
      auto& im_visiable = m_Observer.figOpt.bViewPort;
      COpenGLViewportPtr vp = theScene->getViewport("Image");
      if (vp.get() != nullptr) {
        if (ImGui::Checkbox("im", (bool *)&im_visiable)) {
          vp->resetCloneView();
        }
      }*/
      unlockAccess3DScene();
    }
    ImGui::End();
  }

  // Display [Scene] panel
  static bool visible_all;
  static bool show_tool_panel = true;
  if (ImGui::Begin("Scene", &show_tool_panel,
       ImGuiWindowFlags_AlwaysAutoResize |
       ImGuiWindowFlags_NoScrollbar |
       ImGuiWindowFlags_MenuBar)) {

    ImGui::Text("Arcsoft VSLAM Team: ");
    ImGui::NewLine();
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open", "Ctrl+O")) {}
        if (ImGui::MenuItem("Save", "Ctrl+S")) {}
        if (ImGui::MenuItem("Save As..")) {}
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("System", "Ctrl+T", &show_scene_property_editor)) {}
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

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

void CDisplayWindow3D::OnImGuiRender() {

  if (!m_subview_image)
    return;

  const cv::Mat& frame = m_subview_image->m_image;
  const float x = m_subview_image->m_view_x;
  const float y = m_subview_image->m_view_y;
  const float width = m_subview_image->m_view_width;
  const float height = m_subview_image->m_view_height;

  const float BX = 16;
  const float BY = 36;

  if (!ImGui::Begin(m_subview_image->getName().c_str(),
                    nullptr,
                    ImVec2(width + BX, height + BY),
                    0.5f)) {
    ImGui::End();
    return;
  }

  static GLuint texId = 0;
  if (texId == 0) {
    glGenTextures(1, &texId);
    destoryOpenGLResourcesOnExit.emplace_back([&]() {
        glDeleteTextures(1, &texId);
    });
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glPixelStoref(GL_UNPACK_ALIGNMENT, 1); // default is 4
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, frame.data);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  else if (!frame.empty()) {
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.cols, frame.rows, GL_BGR_EXT, GL_UNSIGNED_BYTE, frame.data);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  ImVec2 showSize(ImGui::GetWindowWidth() - BX, ImGui::GetWindowHeight() - BY);
  ImGui::Image((void*) texId, showSize);

  if (ImGui::BeginPopupContextWindow()) {
    if (ImGui::Button("Reset Size")) {
      ImGui::SetWindowSize(m_subview_image->getName().c_str(),
              ImVec2(frame.cols * ZoomOfImage + BX,
                     frame.rows * ZoomOfImage + BY));
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  ImGui::End();
}

void CDisplayWindow3D::backThreadRun() {

  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
      return ;

  m_Window = glfwCreateWindow(m_initialWindowWidth, m_initialWindowHeight, m_windowCaption.c_str(), NULL, NULL);
  if (m_Window == NULL)
      return ;

  glfwMakeContextCurrent(m_Window);
  glfwSwapInterval(1); // Enable vsync

  glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  m_ImGuiContext = ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
  m_lastWheelRotation = io.MouseWheel;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();

  m_3Dscene = mrpt::opengl::COpenGLScene::Create();
  m_GlCanvas = new CGlCanvas(m_3Dscene);
  InitScene();
  glfwSetKeyCallback(m_Window, key_callback);
  glfwSetWindowUserPointer(m_Window, this);

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
  ImGui_ImplOpenGL2_Init();
  ImVec4 clear_color = ImVec4(0.6f, 0.6f, 0.60f, 1.00f);
  m_ReadyContext = true;

  // loop
  while (!glfwWindowShouldClose(m_Window)) {
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // input
    OnPreRender();

    // 1. Zoom-pan-rotate mouse manipulation
    OnEyeShotRender();

    // 2. Render
    //if(RequestToRefresh3DView)
    {
      get3DSceneAndLock();
      OnImGuiRender();
      m_GlCanvas->OnPaint();
      unlockAccess3DScene();
      RequestToRefresh3DView = false;
    }
    OnPostRender();

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glfwMakeContextCurrent(m_Window);
    glfwSwapBuffers(m_Window);

    //cv::waitKey(1);
  }

  // glfw: terminate, clearing all previously allocated GLFW resources.
  std::this_thread::sleep_for(std::chrono::microseconds(10));
  // Cleanup
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  for (auto& f : destoryOpenGLResourcesOnExit) f();

  glfwDestroyWindow(m_Window);
  glfwTerminate();
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
      bWaitKey = true;
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
