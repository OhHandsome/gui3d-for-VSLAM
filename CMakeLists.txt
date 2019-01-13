project(gui3d)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# Set search directory for looking for our custom CMake scripts to
list(APPEND CMAKE_MODULE_PATH ${gui3d_SOURCE_DIR}/cmake_modules)

find_package(OpenCV)
option(USE_SYSTEM_MRPT "Use System MRPT Lib" ON)
if(USE_SYSTEM_MRPT)
    find_package(MRPT REQUIRED gui;opengl)
else()
    set(ARC_MRPT_SRCS_PATH "/home/oyg5285/project/mrpt-1.5.6")
    set(ARC_MRPT_LIBS      "/home/oyg5285/project/mrpt-1.5.6/lib")
    include_directories(
            ${ARC_MRPT_SRCS_PATH}/libs/base/include
            ${ARC_MRPT_SRCS_PATH}/libs/opengl/include
            ${ARC_MRPT_SRCS_PATH}/libs/gui/include
            ${ARC_MRPT_SRCS_PATH}/include/mrpt-config/unix
    )
    link_directories(${ARC_MRPT_LIBS})
endif()

file(GLOB_RECURSE SRCS src/gui3d/render/*.cpp src/gui3d/utils/*.cpp)

option(HAS_IMGUI "Use System MRPT Lib" OFF)
if(HAS_IMGUI)
  list(APPEND
       SRCS
       src/gui3d/window/CDisplayWindow3D.cpp
       src/gui3d/window/CGlCanvas.cpp)
endif()

add_library(${PROJECT_NAME} STATIC ${SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ./inc)
target_link_libraries(${PROJECT_NAME} PUBLIC libmrpt-base.so libmrpt-opengl.so libmrpt-gui.so ${OpenCV_LIBRARIES} )
if(HAS_IMGUI)
  target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad imgui imgui_impl)
  target_compile_definitions(${PROJECT_NAME} PRIVATE -DHAS_IMGUI=1)
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS)

set(PROJECT_NAME gui3d_demo)
add_executable(${PROJECT_NAME} demo.cpp)
target_link_libraries(${PROJECT_NAME} gui3d)

set(PROJECT_NAME viz_demo)
add_executable(${PROJECT_NAME} demo_viz.cpp)
target_link_libraries(${PROJECT_NAME} gui3d vslam_viz)

set(PROJECT_NAME gui3d_test)
add_executable(${PROJECT_NAME} test.cpp)
target_link_libraries(${PROJECT_NAME} gui3d)