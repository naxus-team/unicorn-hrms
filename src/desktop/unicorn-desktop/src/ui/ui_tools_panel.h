#pragma once

#include "ui_context.h"
#include <string>
#include <vector>

namespace Unicorn::UI {

    struct UITheme {
        glm::vec4 primaryColor = Color::Primary;
        glm::vec4 secondaryColor = Color::Secondary;
        glm::vec4 backgroundColor = Color::Background;
        glm::vec4 panelColor = Color::Panel;
        glm::vec4 textColor = Color::Text;
        glm::vec4 borderColor = Color::Border;

        float defaultRounding = 6.0f;
        float spacing = 8.0f;
        float padding = 10.0f;
    };

    class UIToolsPanel {
    public:
        UIToolsPanel(UIContext& uiContext);

        void Render();
        void Toggle() { m_IsVisible = !m_IsVisible; }
        bool IsVisible() const { return m_IsVisible; }

        // Theme management
        UITheme& GetTheme() { return m_Theme; }
        void ApplyTheme();
        void SaveTheme(const std::string& name);
        void LoadTheme(const std::string& name);

    private:
        void RenderFontSettings();
        void RenderColorSettings();
        void RenderLayoutSettings();
        void RenderPreview();

    private:
        UIContext& m_UI;
        bool m_IsVisible = true;
        UITheme m_Theme;

        // Font settings
        float m_FontSize = 14.0f;
        float m_LetterSpacing = 0.0f;
        float m_LineHeight = 1.0f;
        bool m_UseKerning = true;
        int m_AntialiasMode = 1; // 0=None, 1=Grayscale, 2=LCD

        // Preview text
        std::string m_PreviewTextArabic = "مرحباً بك في Unicorn HRMS";
        std::string m_PreviewTextEnglish = "Hello World! Test 123";
        std::string m_PreviewTextMixed = "النظام: System v1.0 الإصدار";
    };

} // namespace Unicorn::UI

// ui_tools_panel.cpp
#include "ui_tools_panel.h"
#include <iostream>

namespace Unicorn::UI {

    UIToolsPanel::UIToolsPanel(UIContext& uiContext)
        : m_UI(uiContext)
    {
    }

    void UIToolsPanel::Render() {
        if (!m_IsVisible) return;

        m_UI.BeginWindow("أدوات التصميم - Design Tools", glm::vec2(50, 50), glm::vec2(400, 700));

        // Tabs or sections
        m_UI.TextColored(Color::Primary, "إعدادات الخطوط - Font Settings");
        m_UI.Separator();
        RenderFontSettings();

        m_UI.Spacing(15.0f);

        m_UI.TextColored(Color::Primary, "إعدادات الألوان - Color Settings");
        m_UI.Separator();
        RenderColorSettings();

        m_UI.Spacing(15.0f);

        m_UI.TextColored(Color::Primary, "إعدادات التخطيط - Layout Settings");
        m_UI.Separator();
        RenderLayoutSettings();

        m_UI.Spacing(15.0f);

        m_UI.TextColored(Color::Primary, "معاينة - Preview");
        m_UI.Separator();
        RenderPreview();

        m_UI.EndWindow();
    }

    void UIToolsPanel::RenderFontSettings() {
        // Get font manager from UI context
        auto& fontManager = m_UI.GetRenderer().GetFontManager();
        auto& renderOptions = fontManager.GetRenderOptions();

        // Letter Spacing
        m_UI.Text("المسافة بين الحروف:");
        if (m_UI.SliderFloat("Letter Spacing", &m_LetterSpacing, -2.0f, 10.0f)) {
            renderOptions.letterSpacing = m_LetterSpacing;
            std::cout << "Letter spacing: " << m_LetterSpacing << std::endl;
        }

        // Line Height
        m_UI.Text("ارتفاع السطر:");
        if (m_UI.SliderFloat("Line Height", &m_LineHeight, 0.8f, 2.0f)) {
            renderOptions.lineHeight = m_LineHeight;
            std::cout << "Line height: " << m_LineHeight << std::endl;
        }

        // Kerning
        bool kerningChanged = m_UI.Checkbox("تفعيل Kerning", &m_UseKerning);
        if (kerningChanged) {
            renderOptions.useKerning = m_UseKerning;
            std::cout << "Kerning: " << (m_UseKerning ? "ON" : "OFF") << std::endl;
        }

        // Antialiasing mode
        m_UI.Text("جودة الخط:");
        m_UI.BeginHorizontal();
        {
            if (m_UI.Button("بدون", glm::vec2(70, 25))) {
                m_AntialiasMode = 0;
                renderOptions.aaMode = FontRenderOptions::AntialiasMode::None;
                std::cout << "AA Mode: None" << std::endl;
            }

            if (m_UI.Button("عادي", glm::vec2(70, 25))) {
                m_AntialiasMode = 1;
                renderOptions.aaMode = FontRenderOptions::AntialiasMode::Grayscale;
                std::cout << "AA Mode: Grayscale" << std::endl;
            }

            if (m_UI.Button("عالي", glm::vec2(70, 25))) {
                m_AntialiasMode = 2;
                renderOptions.aaMode = FontRenderOptions::AntialiasMode::LCD;
                std::cout << "AA Mode: LCD" << std::endl;
            }
        }
        m_UI.EndHorizontal();

        // Current mode indicator
        std::string modeText = "الوضع الحالي: ";
        switch (m_AntialiasMode) {
        case 0: modeText += "بدون تنعيم"; break;
        case 1: modeText += "تنعيم عادي"; break;
        case 2: modeText += "تنعيم عالي"; break;
        }
        m_UI.TextColored(Color::Secondary, modeText);
    }

    void UIToolsPanel::RenderColorSettings() {
        m_UI.Text("اللون الأساسي:");

        // Primary color sliders
        m_UI.Text("R:");
        m_UI.SliderFloat("##PrimaryR", &m_Theme.primaryColor.r, 0.0f, 1.0f);
        m_UI.Text("G:");
        m_UI.SliderFloat("##PrimaryG", &m_Theme.primaryColor.g, 0.0f, 1.0f);
        m_UI.Text("B:");
        m_UI.SliderFloat("##PrimaryB", &m_Theme.primaryColor.b, 0.0f, 1.0f);

        // Color preview
        m_UI.Panel(glm::vec2(360, 30), [&]() {
            m_UI.TextColored(m_Theme.primaryColor, "▓▓▓▓▓▓▓▓▓▓ معاينة اللون");
            });

        m_UI.Spacing(10.0f);

        // Background color
        m_UI.Text("لون الخلفية:");
        m_UI.Text("Lightness:");
        float bgLightness = m_Theme.backgroundColor.r;
        if (m_UI.SliderFloat("##BgLight", &bgLightness, 0.0f, 1.0f)) {
            m_Theme.backgroundColor = glm::vec4(bgLightness, bgLightness, bgLightness, 1.0f);
        }

        // Apply button
        if (m_UI.Button("تطبيق الألوان - Apply Colors", glm::vec2(200, 30))) {
            ApplyTheme();
        }
    }

    void UIToolsPanel::RenderLayoutSettings() {
        m_UI.Text("زوايا مدورة:");
        m_UI.SliderFloat("Rounding", &m_Theme.defaultRounding, 0.0f, 20.0f);

        m_UI.Text("المسافات:");
        m_UI.SliderFloat("Spacing", &m_Theme.spacing, 0.0f, 20.0f);

        m_UI.Text("الحواف الداخلية:");
        m_UI.SliderFloat("Padding", &m_Theme.padding, 0.0f, 30.0f);

        // Preview rectangles with current rounding
        m_UI.Spacing(10.0f);
        m_UI.Text("معاينة الزوايا:");

        for (int i = 0; i < 3; i++) {
            m_UI.Panel(glm::vec2(150, 40), [&]() {
                std::string text = "مربع " + std::to_string(i + 1);
                m_UI.Text(text);
                });
            m_UI.Spacing(5.0f);
        }
    }

    void UIToolsPanel::RenderPreview() {
        m_UI.Panel(glm::vec2(380, 200), [&]() {
            m_UI.TextColored(Color::Primary, "نص عربي:");
            m_UI.Text(m_PreviewTextArabic);
            m_UI.Spacing();

            m_UI.TextColored(Color::Primary, "English Text:");
            m_UI.Text(m_PreviewTextEnglish);
            m_UI.Spacing();

            m_UI.TextColored(Color::Primary, "نص مختلط:");
            m_UI.Text(m_PreviewTextMixed);
            m_UI.Spacing();

            // Sample buttons
            m_UI.Button("زر تجريبي", glm::vec2(150, 30));

            // Sample checkbox
            static bool testCheck = true;
            m_UI.Checkbox("خيار تجريبي", &testCheck);
            });

        m_UI.Spacing(10.0f);

        // Export/Import buttons
        m_UI.BeginHorizontal();
        {
            if (m_UI.Button("حفظ الإعدادات", glm::vec2(180, 30))) {
                SaveTheme("default");
            }

            if (m_UI.Button("استعادة الإعدادات", glm::vec2(180, 30))) {
                LoadTheme("default");
            }
        }
        m_UI.EndHorizontal();
    }

    void UIToolsPanel::ApplyTheme() {
        // Apply theme colors to the global Color struct
        // Note: You might want to make Color modifiable instead of const
        std::cout << "[UITools] Theme applied!" << std::endl;
        std::cout << "Primary: " << m_Theme.primaryColor.r << ", "
            << m_Theme.primaryColor.g << ", "
            << m_Theme.primaryColor.b << std::endl;
    }

    void UIToolsPanel::SaveTheme(const std::string& name) {
        std::cout << "[UITools] Saving theme: " << name << std::endl;
        // TODO: Implement theme serialization to JSON/file
    }

    void UIToolsPanel::LoadTheme(const std::string& name) {
        std::cout << "[UITools] Loading theme: " << name << std::endl;
        // TODO: Implement theme deserialization
    }

} // namespace Unicorn::UI

// Usage in main application.cpp:

#include "ui/ui_tools_panel.h"

class UnicornApp : public Unicorn::Application {
private:
    std::unique_ptr<Unicorn::UI::UIToolsPanel> m_ToolsPanel;

public:
    UnicornApp(const Unicorn::ApplicationConfig& config)
        : Application(config)
    {
        // ... existing initialization ...
    }

    void OnInit() override {
        // ... existing code ...

        // Create tools panel
        m_ToolsPanel = std::make_unique<Unicorn::UI::UIToolsPanel>(GetUI());
    }

    void OnUIRender() override {
        auto& ui = GetUI();

        // Render tools panel (press T to toggle)
        if (Input::IsKeyPressed(GLFW_KEY_T)) {
            m_ToolsPanel->Toggle();
        }

        m_ToolsPanel->Render();

        // ... rest of your UI code ...
    }
};