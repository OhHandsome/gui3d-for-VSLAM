cmake_minimum_required(VERSION 3.14)
project(blueberry)

set(CMAKE_CXX_STANDARD 11)
find_package(Eigen3 REQUIRED 3.3)
find_package(MRPT REQUIRED gui;opengl)

add_executable(blueberry main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${EIGEN3_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${MRPT_LIBS})

## Third-party
add_subdirectory(Third-party)