project(gui3d)
cmake_minimum_required(VERSION 2.9)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# Set search directory for looking for our custom CMake scripts to
list(APPEND CMAKE_MODULE_PATH ${gui3d_SOURCE_DIR}/cmake_modules)

## dependency
find_package(OpenCV)
find_package(MRPT REQUIRED gui;opengl)

file(GLOB_RECURSE SRCS src/gui3d/render/*.cpp src/gui3d/utils/*.cpp)
option(HAS_IMGUI "Use System MRPT Lib" OFF)
if(HAS_IMGUI)
  list(APPEND SRCS

       src/gui3d/window/CDisplayWindow3D.cpp
       src/gui3d/window/CGlCanvas.cpp)
endif()

add_library(${PROJECT_NAME} STATIC ${SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ./inc)
target_link_libraries(${PROJECT_NAME} PUBLIC
        libmrpt-base.so libmrpt-opengl.so libmrpt-gui.so
        ${OpenCV_LIBRARIES} )
if(HAS_IMGUI)
  target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad imgui imgui_impl)
  target_compile_definitions(${PROJECT_NAME} PRIVATE -DHAS_IMGUI=1)
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS)

add_subdirectory(samples)