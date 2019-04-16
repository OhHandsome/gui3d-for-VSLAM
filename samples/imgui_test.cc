#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <mrpt/opengl/COpenGLScene.h>
#include <mrpt/opengl/CAxis.h>
#include <mrpt/opengl.h>
#include <imgui.h>
#include <imgui_impl_glfw_gl2.h>
#include <chrono>
#include <thread>
#include <gui3d/window/CGlCanvas.h>
#include <gui3d/window/CDisplayWindow3D.h>

const int AXISLength  = 6;

using namespace mrpt::opengl;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int single_thread_test()
{
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();

  // glfw window creation
  // --------------------
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == nullptr)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // Setup Dear ImGui binding
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  ImGui_ImplGlfwGL2_Init(window, true);

  // Setup style
  ImGui::StyleColorsClassic();

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  COpenGLScenePtr	m_openGLScene = COpenGLScene::Create();
  CAxisPtr Axis = CAxis::Create(-AXISLength, -AXISLength, -AXISLength,
                                AXISLength, AXISLength /* / 2.0 + 1*/, AXISLength /* / 2.0 + 1*/, 4, 2, true);
  Axis->setLocation(0, 0, 0);
  Axis->setTextScale(1.1f);
  Axis->setName("CXYZ");
  Axis->enableTickMarks();
  m_openGLScene->insert(Axis);

  gui3d::CGlCanvas glCanvas(m_openGLScene);

  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  bool is_clicked = false;
  // render loop
  // -----------
  while (!glfwWindowShouldClose(window))
  {
    // Rendering
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwPollEvents();
    ImGui_ImplGlfwGL2_NewFrame();

    // input
    // -----
    processInput(window);

    // 1. Show a simple window.
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
    {
      static float f = 0.0f;
      static int counter = 0;
      ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

      ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

      if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

    // Zoom-pan-rotate mouse manipulation
    if(!io.WantCaptureMouse || !io.WantCaptureKeyboard){
      gui3d::MouseEvent event;
      event.setFromIO(io);

      if(io.MouseDown[0] || io.MouseDown[1]){
        double distance = std::sqrt(io.MouseDelta[0] * io.MouseDelta[0] + io.MouseDelta[1] * io.MouseDelta[1]);
        if(distance > 1.0 && distance < 200.0) {
          glCanvas.OnMouseMove(event);
        }
        glCanvas.OnMouseDown(event);
      }
      else
      if(io.MouseReleased[0] || io.MouseReleased[1]){
        glCanvas.OnMouseUp(event);
      }
      glCanvas.OnMouseWheel(event);
    }

//    if(!io.WantCaptureMouse || !io.WantCaptureKeyboard){
//      MouseEvent event;
//      event.setFromIO(io);
//
//      if(io.MouseDown[0] || io.MouseDown[2]){
//        double distance = std::sqrt(io.MouseDelta[0] * io.MouseDelta[0] + io.MouseDelta[1] * io.MouseDelta[1]);
//        if(!is_clicked){
//          glCanvas.OnMouseDown(event);
//          is_clicked = true;
//        }
//        else if(distance > 1.0) {
//          glCanvas.OnMouseMove(event);
//        } else {
//          glCanvas.OnMouseDown(event);
//          is_clicked = true;
//        }
//      }
//      else
//      if(io.MouseReleased[0] || io.MouseReleased[2]){
//        glCanvas.OnMouseUp(event);
//        is_clicked = false;
//      }
//      glCanvas.OnMouseWheel(event);
//    }

    // render
    // ------
    glCanvas.OnPaint();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    ImGui::Render();
    ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);

  }

  // Cleanup
  ImGui_ImplGlfwGL2_Shutdown();
  ImGui::DestroyContext();

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

int multi_thread_test()
{
  gui3d::CDisplayWindow3DPtr win(new gui3d::CDisplayWindow3D("1", 1080, 720));
  volatile gui3d::Gui3dOption& guiOpt = win->Options();
  while (!guiOpt.figOpt.bExit)
  {
      std::cout << (guiOpt.figOpt.bExit ? "True" : "False") << std::endl;
  };
  printf("Exit");
}

int main()
{
  //single_thread_test();
  multi_thread_test();
}