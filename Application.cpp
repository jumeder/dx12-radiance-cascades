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

    m_cornell = std::make_unique<Model>("..\\..\\Models\\CornellBox-Original.obj", m_renderer->GetDevice());
    m_sphere = std::make_unique<Model>("..\\..\\Models\\Sphere.glb", m_renderer->GetDevice());
    m_bunny = std::make_unique<Model>("..\\..\\Models\\Bunny.obj", m_renderer->GetDevice());


    const auto bunnyTransform = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.3f, 0.3f, 0.3f), DirectX::XMMatrixTranslation(0.3f, 1.1f, 0.3f));
    const auto sphereTransform = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f), DirectX::XMMatrixTranslation(0.f, 1.f, 0));

    m_scene = std::make_unique<Scene>(m_renderer->GetDevice());
    m_scene->AddInstance(*m_cornell, DirectX::XMMatrixIdentity(), DirectX::XMVECTOR{0.8f, 0.8f, 0.8f, 1.f}, DirectX::XMVECTOR{0.f, 0.f, 0.f, 1.f});
    m_bunnyInstance = m_scene->AddInstance(*m_bunny, bunnyTransform, DirectX::XMVECTOR{0.f, 0.f, 0.f, 1.f}, DirectX::XMVECTOR{10.f, 5.f, 0.5f, 0.f});
    m_sphereInstance = m_scene->AddInstance(*m_sphere, sphereTransform, DirectX::XMVECTOR{0.f, 0.f, 0.f, 1.f}, DirectX::XMVECTOR{10.f, 10.f, 10.f, 1.f});
    //m_sphereInstance2 = m_scene->AddInstance(*m_sphere, sphereTransform, DirectX::XMVECTOR{0.f, 0.f, 0.f, 1.f}, DirectX::XMVECTOR{1.f, 2.f, 10.f, 0.f});
}

Application::~Application()
{
    m_renderer->Finish();
    glfwTerminate();
}

void Application::Run()
{
    while(!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        HandleInput();

        const auto angle = std::chrono::high_resolution_clock::now().time_since_epoch().count() * 1e-9f;
        const float xanim = sin(angle);
        const float zanim = cos(angle);

        const auto bunnyTransform = DirectX::XMMatrixMultiply(
            DirectX::XMMatrixRotationAxis({1.f, 1.f, -1.f, 0.f}, angle),
            DirectX::XMMatrixMultiply(
                DirectX::XMMatrixScaling(0.3f, 0.3f, 0.3f), 
                DirectX::XMMatrixTranslation(0.3f, 1.1f, 0.3f)
            )
        );
        const auto sphereTransform = DirectX::XMMatrixMultiply(
            DirectX::XMMatrixScaling(0.001f, 0.001f, 0.001f), 
            DirectX::XMMatrixTranslation(xanim * 0.9f, 0.8f, zanim * 0.9f)
        );

        m_scene->SetInstanceTransform(m_sphereInstance, sphereTransform);
        m_scene->SetInstanceTransform(m_bunnyInstance, bunnyTransform);

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

    if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        if (!m_rightKeyPressed)
        {
            m_currentDebugCascade = std::min(2, m_currentDebugCascade + 1);
            m_renderer->VisualizeCascade(m_currentDebugCascade);
            m_rightKeyPressed = true;
        }
    }
    else
        m_rightKeyPressed = false;

    if(glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        if (!m_leftKeyPressed)
        {
            m_currentDebugCascade = std::max(-1, m_currentDebugCascade - 1);
            m_renderer->VisualizeCascade(m_currentDebugCascade);
            m_leftKeyPressed = true;
        }
    }
    else
        m_leftKeyPressed = false;

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