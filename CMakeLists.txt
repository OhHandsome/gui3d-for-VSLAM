project(gui3d)

add_definitions(-std=c++11)
# Set search directory for looking for our custom CMake scripts to
# look for SuiteSparse, QGLViewer, and Eigen3.
list(APPEND CMAKE_MODULE_PATH ${gui3d_SOURCE_DIR}/cmake_modules)
find_package(Eigen3)
find_package(OpenCV)
file(GLOB_RECURSE SRCS src/* inc/*.h inc/*.hpp)
add_library(${PROJECT_NAME} STATIC ${SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ./inc)
option(USE_SYSTEM_MRPT "Use System MRPT Lib" ON)
if(USE_SYSTEM_MRPT)
  find_package(MRPT REQUIRED gui;opengl)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${MRPT_LIBS} ${Eigen3_LIBRARIES} ${OpenCV_LIBRARIES} )
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
  target_link_libraries(${PROJECT_NAME} PUBLIC
    libmrpt-base.so libmrpt-opengl.so libmrpt-gui.so
    ${Eigen3_LIBRARIES} ${OpenCV_LIBRARIES} )
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS)

set(PROJECT_NAME gui3d_demo)
add_executable(${PROJECT_NAME} demo.cpp)
target_link_libraries(${PROJECT_NAME} gui3d)

set(PROJECT_NAME gui3d_test)
add_executable(${PROJECT_NAME} test.cpp)
target_link_libraries(${PROJECT_NAME} gui3d)
