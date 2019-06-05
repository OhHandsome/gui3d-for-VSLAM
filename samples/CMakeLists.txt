set(PROJECT_NAME example_load_scene)
add_executable(${PROJECT_NAME} example_load_scene.cpp)
target_link_libraries(${PROJECT_NAME} gui3d)

set(PROJECT_NAME example_viz_demo)
add_executable(${PROJECT_NAME} example_viz.cpp)
target_link_libraries(${PROJECT_NAME} gui3d vslam_viz)

set(PROJECT_NAME mrpt_test)
add_executable(${PROJECT_NAME} mrpt_test.cpp)
target_link_libraries(${PROJECT_NAME} gui3d)

set(PROJECT_NAME example_imgui_demo)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
set(SRCS example_imgui_demo.cpp
        ../src/gui3d/imgui/imgui_impl_glfw.cpp
        ../src/gui3d/imgui/imgui_impl_opengl2.cpp
        ../src/gui3d/imgui/imgui.cpp
        ../src/gui3d/imgui/imgui_demo.cpp
        ../src/gui3d/imgui/imgui_draw.cpp
        ../src/gui3d/imgui/imgui_widgets.cpp)
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE glfw ${OPENGL_gl_LIBRARY})

set(PROJECT_NAME example_CDisplayWindow3D)
find_package(GLUT REQUIRED)
set(SRCS example_CDisplayWindow3D.cpp
        CGlCanvas.cpp
        ../src/gui3d/imgui/imgui_impl_glfw.cpp
        ../src/gui3d/imgui/imgui_impl_opengl2.cpp
        ../src/gui3d/imgui/imgui.cpp
        ../src/gui3d/imgui/imgui_demo.cpp
        ../src/gui3d/imgui/imgui_draw.cpp
        ../src/gui3d/imgui/imgui_widgets.cpp)
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw ${OPENGL_gl_LIBRARY} ${GLUT_LIBRARY} pthread
        libmrpt-base.so libmrpt-opengl.so)