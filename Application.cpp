#include "Application.h"
#include "Model.h"
#include "Scene.h"

#include <chrono>

Application::Application(uint32_t width, uint32_t height)
    : m_camera((float)M_PI / 2, (float)width / height, 0.1f)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(width, height, "dx12-radiance-casccades", nullptr, nullptr);
    m_renderer = std::make_unique<Renderer>(glfwGetWin32Window(m_window), width, height);

    m_camera.SetPosition(0, 0.5f, 0.f);

    m_sponza = std::make_unique<Model>("d:\\Scenes\\Test\\Sponza.fbx", m_renderer->GetDevice());
    m_sphere = std::make_unique<Model>("d:\\Scenes\\Test\\Sphere.glb", m_renderer->GetDevice());

    const auto sphereTransform = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f), DirectX::XMMatrixTranslation(0.f, 6.f, 0));

    m_scene = std::make_unique<Scene>(m_renderer->GetDevice());
    m_scene->AddInstance(*m_sponza, DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f), DirectX::XMVECTOR{0.f, 0.5f, 0.5f, 1.f}, DirectX::XMVECTOR{0.f, 0.f, 0.f, 1.f});
    m_sphereInstance = m_scene->AddInstance(*m_sphere, sphereTransform, DirectX::XMVECTOR{0.f, 0.f, 0.f, 1.f}, DirectX::XMVECTOR{4.f, 4.f, 4.f, 1.f});
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

        const auto sphereTransform = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f), DirectX::XMMatrixTranslation(sin(std::chrono::high_resolution_clock::now().time_since_epoch().count() * 1e-9) * 5.f, 6.f, 0));
        m_scene->SetInstanceTransform(m_sphereInstance, sphereTransform);

        m_renderer->Render(m_camera, *m_scene);
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
        const auto xdiff = m_lastMouseX - currMouseX;
        const auto ydiff = m_lastMouseY - currMouseY;
        m_camera.Pitch((float)ydiff * m_cameraRotSpeed);
        m_camera.Yaw((float)xdiff * m_cameraRotSpeed);
    }
    m_lastMouseX = currMouseX;
    m_lastMouseY = currMouseY;
}