#pragma once

#define NOMINMAX
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Renderer.h"
#include "Camera.h"

class Scene;
class Model;

class Application
{
public:
    Application(uint32_t width = 640, uint32_t height = 480);
    ~Application();

    void Run();

private:
    void HandleInput();

    GLFWwindow* m_window;
    std::unique_ptr<Renderer> m_renderer;
    Camera m_camera;

    std::unique_ptr<Model> m_bunny;
    std::unique_ptr<Model> m_cornell;
    std::unique_ptr<Model> m_sphere;
    std::unique_ptr<Scene> m_scene;

    uint32_t m_bunnyInstance;
    uint32_t m_sphereInstance;
    uint32_t m_sphereInstance2;

    bool m_mouseDown = false;
    bool m_rightKeyPressed = false;
    bool m_leftKeyPressed = false;
    double m_lastMouseX = 0.f;
    double m_lastMouseY = 0.f;
    float m_cameraMoveSpeed = 0.01f;
    float m_cameraRotSpeed = 0.01f;
    int m_currentDebugCascade = -1;
};