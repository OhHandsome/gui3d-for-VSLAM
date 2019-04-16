set(PROJECT_NAME gui3d_demo)
add_executable(${PROJECT_NAME} demo.cpp)
target_link_libraries(${PROJECT_NAME} gui3d)

set(PROJECT_NAME viz_demo)
add_executable(${PROJECT_NAME} demo_viz.cpp)
target_link_libraries(${PROJECT_NAME} gui3d vslam_viz)

set(PROJECT_NAME gui3d_test)
add_executable(${PROJECT_NAME} test.cpp)
target_link_libraries(${PROJECT_NAME} gui3d)

set(PROJECT_NAME imgui_test)
add_executable(${PROJECT_NAME} imgui_test.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad imgui imgui_impl gui3d)
