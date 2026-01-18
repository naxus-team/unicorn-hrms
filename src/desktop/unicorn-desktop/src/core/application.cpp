#include "application.h"
#include "window.h"
#include "input.h"
#include "../renderer/renderer.h"
#include "../ui/ui_context.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <thread>

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

        double lastFrameTime = glfwGetTime();
        m_UIContext->MarkDirty();

        while (m_Running && !m_Window->ShouldClose()) {
            bool hasAnimations = m_UIContext->HasActiveAnimations();
            bool isDirty = m_UIContext->IsDirty();

            if (!isDirty && !hasAnimations) {
                glfwWaitEvents();
            }
            else {
                glfwPollEvents();
            }

            double frameStart = glfwGetTime();
            float dt = static_cast<float>(frameStart - lastFrameTime);
            lastFrameTime = frameStart;

            if (m_Window->WasResized()) {
                uint32_t width = m_Window->GetWidth();
                uint32_t height = m_Window->GetHeight();
                m_Renderer->OnWindowResize(width, height);
                m_UIContext->OnWindowResize(width, height);
                m_UIContext->MarkDirty();
            }

            if (hasAnimations) {
                m_UIContext->UpdateAnimations(dt);

                if (!m_UIContext->HasActiveAnimations()) {
                    m_UIContext->MarkDirty();
                }
            }

            OnUpdate(dt);

            if (m_UIContext->IsDirty() || hasAnimations) {
                m_UIContext->BeginFrame();
                OnUIRender();
                m_UIContext->EndFrame();

                m_Renderer->BeginFrame();
                OnRender();
                m_UIContext->Render();
                m_Renderer->EndFrame();

                m_Window->SwapBuffers();
                m_UIContext->ClearDirty();
            }

            Input::ResetMouseWheel();

            if (hasAnimations) {
                auto targetFrameTime = std::chrono::microseconds(16667);
                auto frameTime = std::chrono::microseconds(static_cast<long long>(dt * 1000000.0));

                if (frameTime < targetFrameTime) {
                    std::this_thread::sleep_for(targetFrameTime - frameTime);
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

    void Application::TriggerRender() {
        if (s_Instance && s_Instance->m_UIContext) {
            s_Instance->m_UIContext->MarkDirty();
        }

        glfwPostEmptyEvent();
    }

}