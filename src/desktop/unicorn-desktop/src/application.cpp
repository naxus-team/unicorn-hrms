#define NOMINMAX
#include "core/application.h"
#include "core/window.h"
#include "ui/ui_context.h"
#include "ui/ui_renderer.h"
#include "ui/font_manager.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Unicorn {

    namespace Input {
        static bool IsKeyPressed(int key) {
            auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
            if (!window) return false;
            return glfwGetKey(window, key) == GLFW_PRESS;
        }
    }

    class UnicornApp : public Application {
    public:
        UnicornApp(const ApplicationConfig& config)
            : Application(config),
            m_SelectedPage(0),
            fpsCounter(0)
        {

#ifdef _WIN32
            SetConsoleOutputCP(CP_UTF8);
#endif
        }

        void OnInit() override {
            std::cout << "==================================" << std::endl;
            std::cout << "Unicorn HRMS Starting..." << std::endl;
            std::cout << "==================================" << std::endl;

            auto& fontManager = GetUI().GetRenderer().GetFontManager();

            UI::FontRenderOptions options;
            options.useKerning = true;
            options.letterSpacing = 0.0f;
            options.lineHeight = 1.0f;
            options.aaMode = UI::FontRenderOptions::AntialiasMode::Grayscale;

            fontManager.LoadFontWithOptions("default", "./assets/fonts/cairo.ttf", 14, options);

            std::cout << "[App] Font loaded successfully" << std::endl;
        }

        void OnUpdate(float dt) override {
            // ESC to close
            static bool escWasPressed = false;
            bool escPressed = Input::IsKeyPressed(GLFW_KEY_ESCAPE);

            if (escPressed && !escWasPressed) {
                std::cout << "[App] ESC pressed - Closing..." << std::endl;
                Close();
            }
            escWasPressed = escPressed;
        }

        void OnRender() override {
            // Dark background
            glClearColor(0.97f, 0.97f, 0.97f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        void OnUIRender() override {
            auto& ui = GetUI();

            uint32_t windowWidth = GetWindow().GetWidth();
            uint32_t windowHeight = GetWindow().GetHeight();

            // Left sidebar navigation
            RenderSidebar(ui, 2.0f, windowHeight);

            // Main content area with scrolling
            RenderMainContent(ui, windowWidth, windowHeight);

            // Debug info
            glm::vec2 mousePos = ui.GetMousePos();
            bool mousePressed = ui.IsMouseButtonDown(0);

            // Update FPS counter
            static int frameCounter = 0;
            if (++frameCounter % 60 == 0) {
                fpsCounter = static_cast<int>(1.0f / ui.GetDeltaTime());
            }
        }

        void OnShutdown() override {
            std::cout << "[App] Shutting down..." << std::endl;
        }

    private:
        void RenderSidebar(UI::UIContext& ui, float contentX, uint32_t windowHeight) {
            const float sidebarWidth = 48.0f;


            UI::WindowBorderStyle borderStyle;
            borderStyle.enabled = true;
            borderStyle.thickness = 1.0f;
            borderStyle.color = UI::Color::Border;
            borderStyle.top = false;
            borderStyle.right = true;
            borderStyle.bottom = false;
            borderStyle.left = false;

            ui.BeginWindow("##sidebar",
                glm::vec2(contentX, 0),
                glm::vec2(sidebarWidth, windowHeight),
                borderStyle
            );

            const float buttonSize = 32.0f;

            // Dashboard
            if (ui.IconButton("settings", glm::vec2(buttonSize, buttonSize), UI::Alignment::Center)) {
                m_SelectedPage = 1;
                std::cout << "[Nav] Dashboard selected" << std::endl;
            }

            // Dashboard
            if (ui.IconButton("person", glm::vec2(buttonSize, buttonSize), UI::Alignment::Center)) {
                m_SelectedPage = 2;
                std::cout << "[Nav] Dashboard selected" << std::endl;
            }

            // Dashboard
            if (ui.IconButton("report", glm::vec2(buttonSize, buttonSize), UI::Alignment::Center)) {
                m_SelectedPage = 3;
                std::cout << "[Nav] Dashboard selected" << std::endl;
            }

            ui.Separator(1.0f, 26);

            // Dashboard
            if (ui.IconButton("close", glm::vec2(buttonSize, buttonSize), UI::Alignment::Center)) {
                m_SelectedPage = 4;
                std::cout << "[Nav] Dashboard selected" << std::endl;
            }


            ui.EndWindow();
        }

        // ========================================
        // Main Content Area with Scrolling
        // ========================================
        void RenderMainContent(UI::UIContext& ui, uint32_t windowWidth, uint32_t windowHeight) {
            const float sidebarWidth = 50.0f;
            const float margin = 20.0f;

            const float contentX = sidebarWidth + margin;
            const float contentWidth = windowWidth - sidebarWidth - (margin * 2);

            // Begin global scroll for main content
            ui.BeginGlobalScroll(
                glm::vec2(contentX, 0),
                glm::vec2(contentWidth, windowHeight)
            );
            // Render content based on selected page
            switch (m_SelectedPage) {
            case 0: RenderSettings(ui, contentX, contentWidth); break;
            case 1: RenderEmployees(ui, contentX, contentWidth); break;
            case 2: RenderReports(ui, contentX, contentWidth); break;
            case 3: RenderCodeTools(ui, contentX, contentWidth); break;
            case 4: RenderDashboard(ui, contentX, contentWidth); break;
            }

            ui.EndGlobalScroll();
        }

        // ========================================
        // Page: Dashboard
        // ========================================
        void RenderDashboard(UI::UIContext& ui, float contentX, float contentWidth) {

            float windowWidth = glm::min(contentWidth * 0.9f, 800.0f);

            ui.BeginWindow("Dashboard",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 500)
            );

            ui.TextColored(UI::Color::Primary, "📊 Dashboard - Overview");
            ui.Spacing();

            float buttonWidth = glm::min(windowWidth - 40.0f, 300.0f);

            if (ui.ButtonWithIcon("add", "إضافة عنصر",
                glm::vec2(buttonWidth, 40), UI::Alignment::Left)) {
                std::cout << "Add clicked" << std::endl;
            }

            ui.Spacing();

            if (ui.ButtonWithIcon("settings", "الإعدادات",
                glm::vec2(buttonWidth, 40), UI::Alignment::Center)) {
                std::cout << "Settings clicked" << std::endl;
            }

            ui.Spacing();

            if (ui.ButtonWithIcon("report", "التقارير",
                glm::vec2(buttonWidth, 40), UI::Alignment::Right)) {
                std::cout << "Reports clicked" << std::endl;
            }

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            // ⭐ Responsive panel
            ui.Panel(glm::vec2(windowWidth - 20.0f, 80), [&]() {
                ui.TextColored(UI::Color::White, "Terminal");
                ui.Text("cmd:");
                });

            ui.EndWindow();
        }

        // ========================================
        // Page: Employees
        // ========================================
        void RenderEmployees(UI::UIContext& ui, float contentX, float contentWidth) {
            float windowWidth = glm::min(contentWidth * 0.95f, 900.0f);


            ui.BeginWindow("Employees",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 600)
            );

            ui.TextColored(UI::Color::Primary, "👥 Employee Management");
            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            float buttonWidth = glm::min(windowWidth - 40.0f, 250.0f);
            if (ui.ButtonWithIcon("person", "Add New Employee", glm::vec2(buttonWidth, 42))) {
                std::cout << "Add employee clicked" << std::endl;
            }

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            // Employee list (scrollable)
            ui.Text("Employee List:");
            float panelWidth = windowWidth - 20.0f;
            ui.BeginScrollablePanel("employee_list", glm::vec2(panelWidth, 350));
            {
                for (int i = 0; i < 50; i++) {
                    ui.Panel(glm::vec2(panelWidth - 20.0f, 60), [&]() {
                        std::string name = "Employee #" + std::to_string(i + 1);
                        ui.TextColored(UI::Color::White, name);
                        ui.TextColored(UI::Color::White, "Position: Software Engineer");
                        });
                    ui.Spacing(5.0f);
                }
            }
            ui.EndScrollablePanel();

            ui.EndWindow();
        }

        // ========================================
        // Page: Reports
        // ========================================
        void RenderReports(UI::UIContext& ui, float contentX, float contentWidth) {
            float windowWidth = glm::min(contentWidth * 0.9f, 800.0f);

            ui.BeginWindow("Reports",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 550)
            );

            ui.TextColored(UI::Color::Primary, "📈 Reports & Analytics");
            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            float buttonWidth = glm::min(windowWidth - 40.0f, 250.0f);
            if (ui.ButtonWithIcon("report", "Generate Report", glm::vec2(buttonWidth, 42))) {
                std::cout << "Generate report clicked" << std::endl;
            }

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            // Sample report data
            ui.Text("Monthly Performance Report");
            ui.Spacing();

            float panelWidth = windowWidth - 20.0f;
            for (int i = 0; i < 10; i++) {
                ui.Panel(glm::vec2(panelWidth, 50), [&]() {
                    std::string month = "Month " + std::to_string(i + 1);
                    ui.Text(month);
                    ui.Text("Performance: 85%");
                    });
                ui.Spacing(5.0f);
            }

            ui.EndWindow();
        }

        // ========================================
        // Page: Settings
        // ========================================
        void RenderSettings(UI::UIContext& ui, float contentX, float contentWidth) {
            float windowWidth = glm::min(contentWidth * 0.85f, 700.0f);

            ui.BeginWindow("Settings",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 500)
            );

            ui.TextColored(UI::Color::Primary, "إعدادات التطبيق");
            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            static bool notifications = true;
            static bool darkMode = true;
            static float volume = 0.7f;

            ui.Text("الإعدادات العامة:");
            ui.Spacing();

            ui.Checkbox("Enable Notifications", &notifications);
            ui.Spacing();

            ui.Checkbox("Dark Mode", &darkMode);
            ui.Spacing();

            ui.Text("Volume:");
            ui.SliderFloat("##volume", &volume, 0.0f, 1.0f);

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            float buttonWidth = glm::min(windowWidth - 40.0f, 250.0f);
            if (ui.ButtonWithIcon("settings", "حفظ الإعدادات", glm::vec2(buttonWidth, 42))) {
                std::cout << "حفظ الإعدادات" << std::endl;
            }

            ui.EndWindow();
        }

        // ========================================
        // Page: Code/Tools
        // ========================================
        void RenderCodeTools(UI::UIContext& ui, float contentX, float contentWidth) {
            float windowWidth = glm::min(contentWidth * 0.95f, 900.0f);

            ui.BeginWindow("Code Tools",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 650)
            );

            ui.TextColored(UI::Color::Primary, "🛠️ Developer Tools");
            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            ui.Text("Debug Information:");
            ui.Spacing();

            std::string mousePos = "Mouse: " +
                std::to_string((int)ui.GetMousePos().x) + ", " +
                std::to_string((int)ui.GetMousePos().y);
            ui.Text(mousePos);

            ui.Spacing();

            ui.Text("Rendering Stats:");
			ui.Text("FPS: " + std::to_string(fpsCounter));
            ui.Text("Draw Calls: 15");

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            // Long content to test scrolling
            ui.Text("Scrollaboptions.letterSpacing = 1.0f;le Content Test:");
            ui.Spacing();

            float panelWidth = windowWidth - 20.0f;
            for (int i = 0; i < 20; i++) {
                ui.Panel(glm::vec2(panelWidth, 50), [&]() {
                    std::string text = "Debug Line #" + std::to_string(i + 1);
                    ui.Text(text);
                    });
                ui.Spacing(5.0f);
            }

            ui.EndWindow();
        }

    private:
        int m_SelectedPage;
        int fpsCounter;
    };

    Application* CreateApplication() {
        ApplicationConfig config;
        config.name = "Unicorn";
        config.width = 1280;
        config.height = 720;
        config.vsync = true;

        return new UnicornApp(config);
    }

} // namespace Unicorn