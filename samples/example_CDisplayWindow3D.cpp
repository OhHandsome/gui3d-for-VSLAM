// dear imgui: standalone example application for GLFW + OpenGL2, using legacy fixed pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_glfw_opengl2/ folder**
// See imgui_impl_glfw.cpp for details.

#include "../src/gui3d/imgui/imgui.h"
#include "../src/gui3d/imgui/imgui_impl_glfw.h"
#include "../src/gui3d/imgui/imgui_impl_opengl2.h"
#include "../src/gui3d/imgui/imgui_internal.h"
#include "CGlCanvas.h"
#include <mrpt/opengl/COpenGLScene.h>
#include <mrpt/opengl/COpenGLViewport.h>
#include <mrpt/opengl/CAxis.h>
#include <mrpt/opengl/CGridPlaneXY.h>

#include <stdio.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>
#include <thread>
#include <mutex>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
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

using namespace mrpt;
using namespace mrpt::opengl;

class Scene
{
public:
    GLFWwindow* window;
    ImGuiContext* imGuiContext;
    std::mutex                             m_access3Dscene;
    std::thread m_renderLoopThread;

    mrpt::opengl::CAxisPtr                m_Axis3d;
    mrpt::opengl::CGridPlaneXYPtr         m_ZeroPlane;

    float                                  m_lastWheelRotation;
    mrpt::opengl::COpenGLScenePtr          m_3Dscene;   //!< Internal OpenGL object (see general discussion in about usage of this object)
    CGlCanvas*                             m_GlCanvas;  //!< Internal Mouse View object
    bool                                   RequestToRefresh3DView = true;

    Scene()
    {
        // must be add, sometimes crash
        std::this_thread::sleep_for(std::chrono::microseconds(30));
        m_renderLoopThread = std::thread(&Scene::backThreadRun, this);
    }

    mrpt::opengl::COpenGLScenePtr& get3DSceneAndLock() {
        m_access3Dscene.lock();
        return m_3Dscene;
    }

    void unlockAccess3DScene() {
        m_access3Dscene.unlock();
    }

    void InitScene() {
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

    void OnEyeShotRender()
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

    int backThreadRun()
    {
//        GlfwContextScopeGuard gl_ctx_guard(window);
//        ImGuiContextScopeGuard imgui_ctx_guard(imGuiContext);

        // Setup window
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return 1;
        window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL2 example", NULL, NULL);
        if (window == NULL)
            return 1;

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        imGuiContext = ImGui::CreateContext();
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

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL2_Init();


        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


        // Main loop
        while (!glfwWindowShouldClose(window))
        {
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
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

            {
                get3DSceneAndLock();
                m_GlCanvas->OnPaint();
                unlockAccess3DScene();
            }

            OnEyeShotRender();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);

            // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
            // you may need to backup/reset/restore current shader using the commented lines below.
            //GLint last_program;
            //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
            //glUseProgram(0);
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            //glUseProgram(last_program);

            glfwMakeContextCurrent(window);
            glfwSwapBuffers(window);
        }

        // Cleanup
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

std::shared_ptr<Scene> WindowPtr;
int main(int, char**)
{
    WindowPtr = std::make_shared<Scene>();
    while (1);
    return 0;
}
