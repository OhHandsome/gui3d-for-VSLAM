cmake_minimum_required(VERSION 3.14)
project(gui3d)

set(CMAKE_CXX_STANDARD 11)
find_package(Eigen3 REQUIRED 3.3)
find_package(MRPT REQUIRED gui;opengl)
find_package(OpenCV REQUIRED)

file(GLOB_RECURSE SRCS src/gui3d/*.cpp)
list(APPEND SRCS Third-party/imguifilesystem/imguifilesystem.cpp)
add_library(${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE ${EIGEN3_INCLUDE_DIR} . PUBLIC src)
target_link_libraries(${PROJECT_NAME} PRIVATE ${MRPT_LIBS} imgui ${OpenCV_LIBRARIES})

## Third-party
add_subdirectory(Third-party)