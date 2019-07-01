cmake_minimum_required(VERSION 3.14)
project(gui3d)

set(CMAKE_CXX_STANDARD 11)
find_package(Eigen3 REQUIRED 3.3)
find_package(MRPT REQUIRED gui;opengl)
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)

file(GLOB_RECURSE SRCS src/gui3d/*.cpp)
list(APPEND SRCS Third-party/imguifilesystem/imguifilesystem.cpp)
add_library(${PROJECT_NAME} ${SRCS} src/gui3d/window/CImGui.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${EIGEN3_INCLUDE_DIR} . PUBLIC src)
target_link_libraries(${PROJECT_NAME} PUBLIC ${MRPT_LIBS} imgui ${OpenCV_LIBRARIES}
                                      PRIVATE ${OPENGL_gl_LIBRARY} ${GLUT_LIBRARY} pthread)

## Third-party
add_subdirectory(Third-party)

## instance
add_executable(example_gui3d_instance main.cpp)
target_link_libraries(example_gui3d_instance gui3d)
target_include_directories(example_gui3d_instance PRIVATE ${EIGEN3_INCLUDE_DIR})
