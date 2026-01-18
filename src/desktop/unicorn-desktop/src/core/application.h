#pragma once
#include <memory>
#include <string>

namespace Unicorn {
    class Window;
    class Renderer;
    namespace UI { class UIContext; }

    struct ApplicationConfig {
        std::string name = "Unicorn";
        uint32_t width = 1920;
        uint32_t height = 1080;
        bool vsync = true;
    };

    class Application {
    public:
        Application(const ApplicationConfig& config);
        virtual ~Application();

        void Run();
        void Close();

        Window& GetWindow() { return *m_Window; }
        UI::UIContext& GetUI() { return *m_UIContext; }

        static Application& Get() { return *s_Instance; }
        static void TriggerRender();

    protected:
        virtual void OnInit() {}
        virtual void OnUpdate(float dt) {}
        virtual void OnRender() {}
        virtual void OnUIRender() {}
        virtual void OnShutdown() {}

    private:
        static Application* s_Instance;
        ApplicationConfig m_Config;
        std::unique_ptr<Window> m_Window;
        std::unique_ptr<Renderer> m_Renderer;
        std::unique_ptr<UI::UIContext> m_UIContext;


        bool m_Running = true;
        float m_LastTime = 0.0f;
    };

    Application* CreateApplication();
}