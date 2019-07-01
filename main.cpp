#include <iostream>
#include <Eigen/Core>
#include <mrpt/opengl/COpenGLScene.h>
#include <gui3d/gui.h>

int main()
{
    gui3d::nFigure(gui3d::Engine(), 1080, 720);
    gui3d::renderFrame(sysChannel[CurCamera], I44);
    while (1);
    return 0;
}