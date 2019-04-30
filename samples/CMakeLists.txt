set(PROJECT_NAME mrpt_demo)
add_executable(${PROJECT_NAME} example_mrpt.cpp)
target_link_libraries(${PROJECT_NAME} gui3d)

set(PROJECT_NAME viz_demo)
add_executable(${PROJECT_NAME} example_viz.cpp)
target_link_libraries(${PROJECT_NAME} gui3d vslam_viz)

set(PROJECT_NAME mrpt_test)
add_executable(${PROJECT_NAME} mrpt_test.cpp)
target_link_libraries(${PROJECT_NAME} gui3d)

set(PROJECT_NAME scene_demo)
add_executable(${PROJECT_NAME} example_imgui_scene.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad imgui imgui_impl gui3d)

set(PROJECT_NAME example_glfw_opengl2)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
set(SRCS example_glfw_opengl2.cpp
        ../src/gui3d/imgui/imgui_impl_glfw.cpp
        ../src/gui3d/imgui/imgui_impl_opengl2.cpp
        ../src/gui3d/imgui/imgui.cpp
        ../src/gui3d/imgui/imgui_demo.cpp
        ../src/gui3d/imgui/imgui_draw.cpp
        ../src/gui3d/imgui/imgui_widgets.cpp)
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE glfw ${OPENGL_gl_LIBRARY})