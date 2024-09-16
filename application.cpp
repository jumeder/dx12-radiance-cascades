#include "application.h"

Application::Application(uint32_t width, uint32_t height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(width, height, "dx12-radiance-casccades", nullptr, nullptr);
}

Application::~Application()
{
    glfwTerminate();
}

void Application::Run()
{
    while(!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
    }
}