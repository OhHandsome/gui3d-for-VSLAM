#include <gui3d/window/CDisplayWindow3D.h>
#include <gui3d/window/CImGui.h>
#include <gui3d/render/model_render.h>
#include <gui3d/gui.h>

#include <mrpt/io/CFileGZInputStream.h>
#include <mrpt/serialization/archiveFrom_std_streams.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <imgui_internal.h>
#include <Third-party/imguifilesystem/imguifilesystem.h>
#include <list>

#define USE_BACKEND_RENDER 0

namespace gui3d {

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
}

CDisplayWindow3DPtr
CDisplayWindow3D::Create(const std::string &windowCaption,
                         unsigned int initialWindowWidth,
                         unsigned int initialWindowHeight) {
  return CDisplayWindow3DPtr(new CDisplayWindow3D(windowCaption, initialWindowWidth, initialWindowHeight));
}

bool CDisplayWindow3D::WindowClosed() const
{
  return glfwWindowShouldClose(m_Window);
}

void CDisplayWindow3D::forceRepaint() {
  if (USE_BACKEND_RENDER)
    RequestToRefresh3DView = true;
  else
    RunOnce();
}

CDisplayWindow3D::CDisplayWindow3D(const std::string &windowCaption,
                                   unsigned int initialWindowWidth,
                                   unsigned int initialWindowHeight)
   : m_windowCaption(windowCaption)
   , m_initialWindowWidth(initialWindowWidth)
   , m_initialWindowHeight(initialWindowHeight) {

#if USE_BACKEND_RENDER
  m_renderLoopThread = std::thread(&CDisplayWindow3D::backThreadRun, this);
  // must be add, sometimes crash in cmake debug model
  while (!m_ReadyContext) {};
#else
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
      return ;

  m_Window = glfwCreateWindow(m_initialWindowWidth, m_initialWindowHeight, m_windowCaption.c_str(), NULL, NULL);
  if (m_Window == NULL)
      return ;

  GlfwContextScopeGuard gl_ctx_guard(m_Window);
  glfwSwapInterval(1); // Enable vsync
  glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  m_ImGuiContext = ImGui::CreateContext();
  ImGuiContextScopeGuard imgui_ctx_guard(m_ImGuiContext);
  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
  ImGui_ImplOpenGL2_Init();

  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
  m_lastWheelRotation = io.MouseWheel;

  // Setup Dear ImGui style
  ImGui::StyleColorsClassic();

  m_GlCanvas = new CGlCanvas(m_3Dscene);
  glfwSetKeyCallback(m_Window, key_callback);
  glfwSetWindowUserPointer(m_Window, this);
#endif
}

CDisplayWindow3D::~CDisplayWindow3D() {
  if (m_renderLoopThread.joinable())
    m_renderLoopThread.join();

  delete m_GlCanvas;
  m_3Dscene.reset();


  // glfw: terminate, clearing all previously allocated GLFW resources.
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(m_Window);
  glfwTerminate();
}

CDisplayImagesPtr
CDisplayWindow3D::createViewImage(const std::string &name) {
  m_subview_image = std::make_shared<CDisplayImages>(name);
  return m_subview_image;
}

void CDisplayWindow3D::OnPreRender() {

  // Menu
  bool& show_scene_property_editor = (bool&)m_control.b_edit3DSceneProperty;
  bool& openSceneFile = (bool&)m_control.b_openSceneFile;
  bool& save3DScene = (bool&)m_control.b_save3DScene;
  bool& save3DSceneAs = (bool&)m_control.b_save3DSceneAs;

  // Display [Scene] panel
  static bool show_tool_panel = true;
  if (ImGui::Begin("Scene", &show_tool_panel,
       ImGuiWindowFlags_AlwaysAutoResize |
       ImGuiWindowFlags_NoScrollbar |
       ImGuiWindowFlags_MenuBar)) {

    ImGui::Text("Arcsoft VSLAM Team: ");
    ImGui::NewLine();

    // menu list
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        ImGui::MenuItem("Open", "Ctrl+O",         &openSceneFile);
        if (ImGui::MenuItem("Save", "Ctrl+S",     &save3DScene)) {}
        if (ImGui::MenuItem("Save As..", nullptr, &save3DSceneAs)) {}
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("System", "Ctrl+T", &show_scene_property_editor)) {}
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    // visiable
    get3DSceneAndLock();
    SceneManager::render_visiable();
    unlockAccess3DScene();
  }
  ImGui::End();

  // Show Scene Property
  if (show_scene_property_editor)
  {
    // Display panel
    if(ImGui::Begin("System", &show_scene_property_editor,
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_AlwaysAutoResize)) {

      // Visible about Axis3d and ZeroPlane
      auto& theScene = get3DSceneAndLock();
      SceneManager::render_property();
      unlockAccess3DScene();
    }
    ImGui::End();
  }
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
  auto& openSceneFile = (bool&)m_control.b_openSceneFile;
  if (openSceneFile)
  {
    const char* startingFolder = ".";
    const char* optionalFileExtensionFilterString = ".3Dscene";//".jpg;.jpeg;.png;.tiff;.bmp;.gif;.txt";
    if (ImGui::Begin("FileSystem",
                     &openSceneFile,
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize)
       ) {
      ImGui::Text("Please choose a file:  ");
      ImGui::SameLine();
      const bool browseButtonPressed = ImGui::Button("...");
      static ImGuiFs::Dialog fsInstance;
      const char* chosenPath = fsInstance.chooseFileDialog(browseButtonPressed,
                                                           startingFolder,
                                                           optionalFileExtensionFilterString);
      if (strlen(chosenPath)>0) {
        // A path (chosenPath) has been chosen right now.
        // However we can retrieve it later using: fsInstance.getChosenPath()
        loadSceneFrom(fsInstance.getChosenPath());
        openSceneFile = false;
      }
    }
    ImGui::End();
  }

  auto& saveSceneAs = (bool&)m_control.b_save3DSceneAs;
  if (saveSceneAs)
  {
    if (ImGui::Begin("FileSystem",
                     &saveSceneAs,
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize)
      ) {
      ImGui::Text("Please pretend to save the dummy file 'final.3Dscene' to: ");
      ImGui::SameLine();
      const bool browseButtonPressed3 = ImGui::Button("...##3");
      static ImGuiFs::Dialog fsInstance3;
      const char* savePath = fsInstance3.saveFileDialog(
                                    browseButtonPressed3,
                                    ".", "final.3Dscene");

      if (strlen(savePath)>0) {
        // A path (chosenPath) has been chosen right now.
        // However we can retrieve it later using: fsInstance.getChosenPath()
        COpenGLScene::Ptr theScene = get3DSceneAndLock();
        if (theScene->saveToFile(fsInstance3.getChosenPath()))
          std::cout << "save theScene To " << fsInstance3.getChosenPath() << std::endl;
        unlockAccess3DScene();
        saveSceneAs = false;
      }
    }
    ImGui::End();
  }
}

void CDisplayWindow3D::OnImGuiRender() {

  if (!m_subview_image)
    return;

  m_subview_image->render();
}

void CDisplayWindow3D::loadSceneFrom(const char* fileName)
{
  std::cout << "load Scene: " << fileName << std::endl;
  mrpt::io::CFileGZInputStream f(fileName);

  get3DSceneAndLock();
  SceneManager::clear();
  mrpt::serialization::archiveFrom(f) >> *m_3Dscene;
  m_GlCanvas->m_openGLScene = m_3Dscene;
  m_Axis3d = m_3Dscene->getByClass<CAxis>();
  m_ZeroPlane = m_3Dscene->getByClass<CGridPlaneXY>();
  unlockAccess3DScene();
}

void CDisplayWindow3D::RunOnce()
{
  GlfwContextScopeGuard gl_ctx_guard(m_Window);
  ImGuiContextScopeGuard imgui_ctx_guard(m_ImGuiContext);
  ImVec4 clear_color = ImVec4(0.6f, 0.6f, 0.60f, 0.00f);

  glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    m_GlCanvas->OnPaint();
    OnImGuiRender();
    for (auto hook : m_hookFuncs)
        hook.run();
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
  //ImGui::StyleColorsDark();
  ImGui::StyleColorsClassic();
  m_GlCanvas = new CGlCanvas(m_3Dscene);
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
      for (auto hook : m_hookFuncs)
          hook.run();
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

  glfwDestroyWindow(m_Window);
  glfwTerminate();
}

void CDisplayWindow3D::pushRenderCallBack(gui3d::TCallbackRender userFunction, void *userParam)
{
    HookFunc hook;
    hook.userFunction = userFunction;
    hook.userParam = userParam;
    m_hookFuncs.push_back(hook);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  void* userPointer = glfwGetWindowUserPointer(window);
  CDisplayWindow3D* window3d = (CDisplayWindow3D*) userPointer;
  volatile ControlOptions& con_option = window3d->m_control;

  auto& ReadNextFrame = con_option.ReadNextFrame;
  auto& ReadFrameGap = con_option.ReadFrameGap;
  auto& bWaitKey = con_option.bWaitKey;
  auto& RequestToRefresh3DView = con_option.RequestToRefresh3DView;

  auto& b_openSceneFile = con_option.b_openSceneFile;
  auto& b_save3DScene = con_option.b_save3DScene;
  auto& b_edit3DSceneProperty = con_option.b_edit3DSceneProperty;
  auto& bViewAprilTags = con_option.bViewAprilTags;

  if (action != GLFW_PRESS)
    return;

  switch (key) {
    case GLFW_KEY_ESCAPE:
    case GLFW_KEY_Q:
      glfwSetWindowShouldClose(window, true);
      break;

    case GLFW_KEY_SPACE:
      ReadNextFrame ^= true;
      break;

    case GLFW_KEY_RIGHT:
      ReadFrameGap += FRAME_GAP_LENGTH;
      break;

    case GLFW_KEY_S: {
      if (mods & GLFW_MOD_CONTROL) {
        b_save3DScene = true;
      }
    }
      break;

    case GLFW_KEY_O: {
      if (mods & GLFW_MOD_CONTROL) {
        b_openSceneFile ^= true;
        std::cout << "openFile" << std::endl;
      }
    }
      break;

    case GLFW_KEY_T: {
      if (mods & GLFW_MOD_CONTROL) {
        b_edit3DSceneProperty ^= true;
      }
    }
      break;

    case 'p':
    case 'P': {
      bViewAprilTags ^= true;
      RequestToRefresh3DView = true;
    }
      break;

    default:
      bWaitKey = true;
      //printf("Key pushed: %c\n", key);
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
