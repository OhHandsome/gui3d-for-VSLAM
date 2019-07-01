cmake_minimum_required(VERSION 3.14)
project(imgui)
set(CMAKE_CXX_STANDARD 11)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

set(LIB_SRCS
        imgui.cpp
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_widgets.cpp
        imgui_impl_glfw.cpp
        imgui_impl_opengl2.cpp)

set(LIB_INC
        imgui.h
        imgui_internal.h
        imconfig.h
        imgui_impl_glfw.h
        imgui_impl_opengl2.h
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h)
add_library(${PROJECT_NAME} ${LIB_SRCS} ${LIB_INC})
target_include_directories(${PROJECT_NAME} PUBLIC ../imgui)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw ${OPENGL_gl_LIBRARY})

option(BUILD_IMGUI_DEMO "build example for imgui demo" OFF)
if (BUILD_IMGUI_DEMO)
    add_executable(example_imgui_demo example_imgui_demo.cpp)
    target_link_libraries(example_imgui_demo PRIVATE imgui)
endif()