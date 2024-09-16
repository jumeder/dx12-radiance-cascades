#include "application.h"

Application::Application(uint32_t width, uint32_t height)
    : m_camera((float)M_PI / 2, (float)width / height, 0.1f)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(width, height, "dx12-radiance-casccades", nullptr, nullptr);
    m_renderer = std::make_unique<Renderer>(glfwGetWin32Window(m_window), width, height);

    m_camera.SetPosition(0, 0.1f, 500.f);
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
        HandleInput();

        m_renderer->Render(m_camera);
    }
}

void Application::HandleInput()
{
    // TODO time difference
    if(glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    {
        m_camera.MoveLocal(0, 0, -m_cameraMoveSpeed);
    }

    if(glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
    {
        m_camera.MoveLocal(0, 0, m_cameraMoveSpeed);
    }

    if(glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
    {
        m_camera.MoveLocal(-m_cameraMoveSpeed, 0, 0);
    }

    if(glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
    {
        m_camera.MoveLocal(m_cameraMoveSpeed, 0, 0);
    }

    double currMouseX, currMouseY;
    glfwGetCursorPos(m_window, &currMouseX, &currMouseY);
    if(glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
    {
        const auto xdiff = currMouseX - m_lastMouseX;
        const auto ydiff = currMouseY - m_lastMouseY;
        m_camera.Pitch(-ydiff * m_cameraRotSpeed);
        m_camera.Yaw(-xdiff * m_cameraRotSpeed);
    }
    m_lastMouseX = currMouseX;
    m_lastMouseY = currMouseY;
}