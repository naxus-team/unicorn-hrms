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

        while (m_Running && !m_Window->ShouldClose()) {
            float time = (float)glfwGetTime();
            float dt = time - m_LastTime;
            m_LastTime = time;

            // Check for window resize
            if (m_Window->WasResized()) {
                uint32_t width = m_Window->GetWidth();
                uint32_t height = m_Window->GetHeight();

                // Update renderer viewport
                m_Renderer->OnWindowResize(width, height);

                // Update UI renderer viewport
                m_UIContext->OnWindowResize(width, height);

                std::cout << "[Application] Window resized to: " << width << "x" << height << std::endl;
            }

            m_Window->OnUpdate();

            OnUpdate(dt);

            m_Renderer->BeginFrame();
            OnRender();

            m_UIContext->BeginFrame();
            OnUIRender();
            m_UIContext->EndFrame();
            m_UIContext->Render();

            m_Renderer->EndFrame();

            m_Window->SwapBuffers();
        }

        OnShutdown();
        m_UIContext->Shutdown();
        m_Renderer->Shutdown();
    }

    void Application::Close() {
        m_Running = false;
    }

}