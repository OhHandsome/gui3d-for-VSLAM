#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>

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

struct ControlOptions {
  #define FRAME_GAP_LENGTH 1

  bool bExit = false;
  bool ReadNextFrame = true;
  int ReadFrameGap = FRAME_GAP_LENGTH;

  bool b_openSceneFile = false;
  bool b_save3DScene = false;
  bool b_save3DSceneAs = false;
  bool b_edit3DSceneProperty = false;
  bool bWaitKey = false;
  bool RequestToRefresh3DView = false;
  bool bViewAprilTags = false;
};

