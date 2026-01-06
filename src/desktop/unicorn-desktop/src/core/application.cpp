#include "application.h"
#include "window.h"
#include "../renderer/renderer.h"
#include "../ui/ui_context.h"
#include <iostream>
#include <GLFW/glfw3.h>

namespace Unicorn {

    Application* Application::s_Instance = nullptr;

    Application::Application(const ApplicationConfig& config)
        : m_Config(config)
    {
        s_Instance = this;

        WindowProps props;
        props.title = config.name;
        props.width = config.width;
        props.height = config.height;

        m_Window = std::unique_ptr<Window>(Window::Create(props));
        m_Renderer = std::make_unique<Renderer>();
        m_UIContext = std::make_unique<UI::UIContext>();
    }

    Application::~Application() {}

    void Application::Run() {
        std::cout << "Starting " << m_Config.name << "..." << std::endl;
        OnInit();
        m_Renderer->Init();
        m_UIContext->Init();

        const double targetFrameTime = 1.0 / 60.0; // 60 FPS
        double lastFrameTime = glfwGetTime();
        double accumulator = 0.0;

        bool needsRedraw = true;
        glm::vec2 lastMousePos(0, 0);

        while (m_Running && !m_Window->ShouldClose()) {
            double currentTime = glfwGetTime();
            double frameTime = currentTime - lastFrameTime;
            lastFrameTime = currentTime;

            accumulator += frameTime;

            m_Window->OnUpdate();

            glm::vec2 currentMousePos = m_UIContext->GetMousePos();
            if (currentMousePos != lastMousePos) {
                needsRedraw = true;
                lastMousePos = currentMousePos;
            }

            if (accumulator >= targetFrameTime) {
                accumulator -= targetFrameTime;

                float dt = static_cast<float>(frameTime);

                // Check for window resize
                if (m_Window->WasResized()) {
                    uint32_t width = m_Window->GetWidth();
                    uint32_t height = m_Window->GetHeight();
                    m_Renderer->OnWindowResize(width, height);
                    m_UIContext->OnWindowResize(width, height);
                    needsRedraw = true;
                }

                OnUpdate(dt);

                if (needsRedraw) {
                    m_Renderer->BeginFrame();
                    OnRender();

                    m_UIContext->BeginFrame();
                    OnUIRender();
                    m_UIContext->EndFrame();
                    m_UIContext->Render();

                    m_Renderer->EndFrame();
                    m_Window->SwapBuffers();
                }
                else {
                    glfwWaitEventsTimeout(0.016);
                }
            }
            else {
                double sleepTime = targetFrameTime - accumulator;
                if (sleepTime > 0.001) {
                    glfwWaitEventsTimeout(sleepTime);
                }
            }
        }

        OnShutdown();
        m_UIContext->Shutdown();
        m_Renderer->Shutdown();
    }

    void Application::Close() {
        m_Running = false;
    }

}