#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

class Application
{
public:
    Application(uint32_t width = 640, uint32_t height = 480);
    ~Application();

    void Run();

private:
    GLFWwindow* m_window;
};