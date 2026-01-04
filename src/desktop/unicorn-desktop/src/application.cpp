#include "core/application.h"
#include "core/window.h"        // Add this - full path to window.h
#include "ui/ui_context.h"
#include "ui/ui_renderer.h"
#include "ui/font_manager.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>         // Move GLFW include here, AFTER glad
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Unicorn {

    // Simple Input namespace for keyboard queries
    namespace Input {
        static bool IsKeyPressed(int key) {
            // Get the current GLFW window from Application
            auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
            if (!window) return false;
            return glfwGetKey(window, key) == GLFW_PRESS;
        }
    }

    class UnicornApp : public Application {
    public:
        UnicornApp(const ApplicationConfig& config)
            : Application(config)
        {
            m_Counter = 0;
            m_SliderValue = 0.5f;
            m_CheckboxValue = false;

            // الإضافات الجديدة فقط
            m_ShowFontTools = false;
            m_LetterSpacing = 0.0f;
            m_LineHeight = 1.2f;
            m_KerningEnabled = true;

#ifdef _WIN32
            SetConsoleOutputCP(CP_UTF8);
#endif
        }

        // في application.cpp - OnInit()

        void OnInit() override {
            std::cout << "==================================" << std::endl;
            std::cout << "Unicorn HRMS Starting..." << std::endl;
            std::cout << "مرحباً بك في نظام إدارة الموارد البشرية" << std::endl;
            std::cout << "==================================" << std::endl;

            auto& fontManager = GetUI().GetRenderer().GetFontManager();

            UI::FontRenderOptions options;
            options.useKerning = true;          // بدون kerning
            options.letterSpacing = 1.0f;        // مهم جداً: بدون مسافات إضافية
            options.lineHeight = 1.0f;
            options.aaMode = UI::FontRenderOptions::AntialiasMode::Grayscale;

            fontManager.LoadFontWithOptions("default", "./assets/fonts/cairo.ttf", 14, options);

            // تعيين القيم الافتراضية
            m_LetterSpacing = 1.0f;
            m_LineHeight = 1.0f;
            m_KerningEnabled = true;

            std::cout << "[App] Font loaded with natural spacing (0.0px)" << std::endl;
        }

        void OnUpdate(float dt) override {
            // Toggle font tools window بـ F key
            static bool fKeyWasPressed = false;
            bool fKeyPressed = Input::IsKeyPressed(GLFW_KEY_F);

            if (fKeyPressed && !fKeyWasPressed) {
                m_ShowFontTools = !m_ShowFontTools;
                std::cout << "[App] Font tools: " << (m_ShowFontTools ? "ON" : "OFF") << std::endl;
            }
            fKeyWasPressed = fKeyPressed;
        }

        void OnRender() override {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        void OnUIRender() override {
            auto& ui = GetUI();

            // 1. لوحة أدوات الخط (اختياري - اضغط F)
            if (m_ShowFontTools) {
                RenderFontToolsWindow(ui);
            }

            // 2. النافذة الرئيسية (نفس الكود القديم)
            RenderMainWindow(ui);

            // 3. نافذة الإحصائيات
            RenderStatsWindow(ui);

            // 4. نافذة التعليمات
            RenderInstructionsWindow(ui);
        }

        void OnShutdown() override {
            std::cout << "==================================" << std::endl;
            std::cout << "جاري الإغلاق..." << std::endl;
            std::cout << "Shutting down..." << std::endl;
            std::cout << "==================================" << std::endl;
        }

    private:
        // ========================================
        // النافذة الرئيسية (نفس الكود القديم)
        // ========================================
        void RenderMainWindow(UI::UIContext& ui) {
            ui.BeginWindow("لوحة التحكم الرئيسية", glm::vec2(50, 50), glm::vec2(450, 600));

            ui.TextColored(UI::Color::Primary, "مرحباً بك في نظام Unicorn HRMS");
            ui.Spacing();
            ui.Separator();
            ui.Spacing();

            ui.Text("النظام: Unicorn HRMS Desktop");
            ui.Text("الإصدار: 1.0.0 Enhanced");
            ui.Spacing();
            ui.Separator();
            ui.Spacing();

            // العداد
            ui.Text("مثال على العداد:");
            ui.BeginHorizontal();
            {
                if (ui.Button("زيادة", glm::vec2(100, 30))) {
                    m_Counter++;
                    std::cout << "العداد: " << m_Counter << std::endl;
                }

                ui.Spacing(10.0f);

                if (ui.Button("نقصان", glm::vec2(100, 30))) {
                    m_Counter--;
                    std::cout << "العداد: " << m_Counter << std::endl;
                }
            }
            ui.EndHorizontal();

            std::string counterText = "العدد: " + std::to_string(m_Counter);
            ui.Text(counterText);

            ui.Spacing();
            ui.Separator();
            ui.Spacing();

            // الشريط
            ui.Text("مثال على الشريط:");
            ui.SliderFloat("القيمة", &m_SliderValue, 0.0f, 1.0f);

            ui.Spacing();

            // Checkbox
            ui.Checkbox("تفعيل الخاصية", &m_CheckboxValue);

            ui.Spacing();
            ui.Separator();
            ui.Spacing();

            // قائمة قابلة للتمرير (الإضافة الجديدة)
            ui.Text("قائمة قابلة للتمرير:");
            ui.BeginScrollablePanel("employee_list", glm::vec2(430, 150));
            {
                for (int i = 0; i < 25; i++) {
                    std::string itemText = "موظف رقم " + std::to_string(i + 1);
                    if (ui.Button(itemText, glm::vec2(400, 30))) {
                        std::cout << "Selected: " << itemText << std::endl;
                    }
                    ui.Spacing(3.0f);
                }
            }
            ui.EndScrollablePanel();

            ui.Spacing();

            // أزرار الإجراءات
            ui.Panel(glm::vec2(430, 120), [&]() {
                ui.TextColored(UI::Color::Secondary, "الإجراءات السريعة");
                ui.Spacing();

                if (ui.Button("إضافة موظف", glm::vec2(150, 30))) {
                    std::cout << "إضافة موظف" << std::endl;
                }

                if (ui.Button("عرض التقارير", glm::vec2(150, 30))) {
                    std::cout << "عرض التقارير" << std::endl;
                }

                if (ui.Button("الإعدادات", glm::vec2(150, 30))) {
                    std::cout << "الإعدادات" << std::endl;
                }
                });

            ui.Spacing();

            if (ui.Button("إغلاق التطبيق", glm::vec2(180, 35))) {
                std::cout << "إغلاق التطبيق" << std::endl;
                Close();
            }

            ui.EndWindow();
        }

        // ========================================
        // نافذة الإحصائيات
        // ========================================
        void RenderStatsWindow(UI::UIContext& ui) {
            ui.BeginWindow("Statistics", glm::vec2(550, 50), glm::vec2(350, 280));

            ui.Text("Application Statistics");
            ui.Text("إحصائيات التطبيق");
            ui.Separator();

            ui.Text("FPS: ~60");

            std::string mousePos =
                std::to_string((int)ui.GetMousePos().x) + ", " +
                std::to_string((int)ui.GetMousePos().y);
            ui.Text(mousePos);

            if (ui.IsItemHovered()) {
                ui.TextColored(UI::Color::Secondary, "متحرك!");
            }

            ui.Spacing();
            ui.Separator();
            ui.Spacing();

            // معلومات الخط (الإضافة الجديدة)
            auto& fm = ui.GetRenderer().GetFontManager();
            std::string fontInfo = "Font: " + fm.GetActiveFontName();
            ui.Text(fontInfo);

            auto& opts = fm.GetRenderOptions();
            std::string kerningInfo = "Kerning: " + std::string(opts.useKerning ? "ON" : "OFF");
            ui.Text(kerningInfo);

            std::string spacingInfo = "Spacing: " + std::to_string(opts.letterSpacing) + "px";
            ui.Text(spacingInfo);

            ui.Spacing();
            ui.Separator();
            ui.Spacing();

            // اختبار Kerning
            ui.TextColored(UI::Color::Secondary, "Kerning Test:");
            ui.Text("AWAY - AV - Ta - To");
            ui.Text("Mixed: مرحباً Hello 123");

            ui.EndWindow();
        }

        // ========================================
        // نافذة التعليمات
        // ========================================
        void RenderInstructionsWindow(UI::UIContext& ui) {
            ui.BeginWindow("التعليمات", glm::vec2(950, 50), glm::vec2(300, 280));

            ui.TextColored(UI::Color::Primary, "كيفية الاستخدام:");
            ui.Spacing();

            ui.Text("١. اضغط على الأزرار");
            ui.Text("٢. حرك الشريط");
            ui.Text("٣. فعّل الخيارات");
            ui.Text("٤. مرر القائمة للأعلى/الأسفل");
            ui.Text("5. Press F for font tools");

            ui.Spacing();
            ui.Separator();
            ui.Spacing();

            ui.TextColored(UI::Color::Secondary, "Keyboard Shortcuts:");
            ui.Text("F - Toggle font tools");
            ui.Text("ESC - (Future feature)");

            ui.Spacing();
            ui.Separator();
            ui.Spacing();

            ui.TextColored(UI::Color::Secondary, "New Features:");
            ui.Text("✓ Kerning support");
            ui.Text("✓ Scrollable panels");
            ui.Text("✓ Letter spacing control");
            ui.Text("✓ Rounded rectangles");

            ui.EndWindow();
        }

        // ========================================
        // نافذة أدوات الخط (الإضافة الجديدة)
        // ========================================
        void RenderFontToolsWindow(UI::UIContext& ui) {
            ui.BeginWindow("⚙ أدوات الخط - Font Tools", glm::vec2(50, 670), glm::vec2(450, 300));

            auto& fontManager = ui.GetRenderer().GetFontManager();
            auto& options = fontManager.GetRenderOptions();

            ui.TextColored(UI::Color::Primary, "إعدادات الخط المتقدمة");
            ui.Spacing();
            ui.Separator();
            ui.Spacing();

            // Letter Spacing
            ui.Text("المسافة بين الحروف:");
            if (ui.SliderFloat("##spacing", &m_LetterSpacing, -3.0f, 10.0f)) {
                options.letterSpacing = m_LetterSpacing;
                std::cout << "[FontTools] Letter spacing: " << m_LetterSpacing << "px" << std::endl;
            }

            ui.Spacing();

            // Line Height
            ui.Text("ارتفاع السطر:");
            if (ui.SliderFloat("##lineheight", &m_LineHeight, 0.5f, 3.0f)) {
                options.lineHeight = m_LineHeight;
                std::cout << "[FontTools] Line height: " << m_LineHeight << "x" << std::endl;
            }

            ui.Spacing();

            // Kerning Toggle
            if (ui.Checkbox("تفعيل Kerning", &m_KerningEnabled)) {
                options.useKerning = m_KerningEnabled;
                fontManager.SetRenderOptions(options);
                std::cout << "[FontTools] Kerning: " << (m_KerningEnabled ? "ON" : "OFF") << std::endl;
            }

            ui.Spacing();
            ui.Separator();
            ui.Spacing();

            // Preview Panel
            ui.Panel(glm::vec2(430, 110), [&]() {
                ui.TextColored(UI::Color::Primary, "معاينة:");
                ui.Spacing();

                ui.Text("English: AWAY AV Ta To Yo");
                ui.Text("عربي: مرحباً بك في النظام");
                ui.Text("Mixed: النظام System 123");
                ui.Text("Numbers: 0123456789");
                });

            ui.EndWindow();
        }

    private:
        // المتغيرات الأساسية
        int m_Counter;
        float m_SliderValue;
        bool m_CheckboxValue;

        // المتغيرات الجديدة (Font Tools)
        bool m_ShowFontTools;
        float m_LetterSpacing;
        float m_LineHeight;
        bool m_KerningEnabled;
    };

    // ========================================
    // Factory Function
    // ========================================
    Application* CreateApplication() {
        ApplicationConfig config;
        config.name = "Unicorn HRMS Desktop - نظام إدارة الموارد البشرية";
        config.width = 1280;
        config.height = 720;
        config.vsync = true;

        return new UnicornApp(config);
    }

} // namespace Unicorn