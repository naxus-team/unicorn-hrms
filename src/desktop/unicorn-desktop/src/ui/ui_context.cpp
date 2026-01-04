#include "ui_context.h"
#include "ui_renderer.h"
#include "../core/input.h"
#include "../core/application.h"
#include "../core/window.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace Unicorn::UI {

    // Color definitions
    const glm::vec4 Color::White = { 1.0f, 1.0f, 1.0f, 1.0f };
    const glm::vec4 Color::Black = { 0.0f, 0.0f, 0.0f, 1.0f };
    const glm::vec4 Color::Primary = { 0.26f, 0.59f, 0.98f, 1.0f };
    const glm::vec4 Color::Secondary = { 0.6f, 0.2f, 0.8f, 1.0f };
    const glm::vec4 Color::Background = { 0.12f, 0.12f, 0.12f, 1.0f };
    const glm::vec4 Color::Panel = { 0.18f, 0.18f, 0.18f, 1.0f };
    const glm::vec4 Color::Border = { 0.3f, 0.3f, 0.3f, 1.0f };
    const glm::vec4 Color::Text = { 0.95f, 0.95f, 0.95f, 1.0f };
    const glm::vec4 Color::TextDisabled = { 0.5f, 0.5f, 0.5f, 1.0f };
    const glm::vec4 Color::ButtonNormal = { 0.26f, 0.59f, 0.98f, 0.4f };
    const glm::vec4 Color::ButtonHover = { 0.26f, 0.59f, 0.98f, 1.0f };
    const glm::vec4 Color::ButtonActive = { 0.06f, 0.53f, 0.98f, 1.0f };

    UIContext::UIContext() {
        m_LayoutStack.push_back(LayoutContext());
        m_Renderer = std::make_unique<UIRenderer>();
    }

    UIContext::~UIContext() {}

    void UIContext::Init() {
        std::cout << "[UI] Initializing UI Context..." << std::endl;

        // Get window dimensions from Application
        Application& app = Application::Get();
        Window& window = app.GetWindow();
        uint32_t width = window.GetWidth();
        uint32_t height = window.GetHeight();

        m_Renderer->Init(width, height);
        std::cout << "[UI] Initialized with viewport: " << width << "x" << height << std::endl;
    }

    void UIContext::Shutdown() {
        std::cout << "[UI] Shutting down UI Context..." << std::endl;
        if (m_Renderer) {
            m_Renderer->Shutdown();
        }
    }

    void UIContext::BeginFrame() {
        m_DrawCommands.clear();
        m_IDStack.clear();
        m_LastWidgetState = WidgetState();

        // Update input
        m_LastMousePos = m_MousePos;
        m_MousePos = Input::GetMousePosition();
        m_MouseButtons[0] = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        m_MouseButtons[1] = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
        m_MouseButtons[2] = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE);

        m_FrameCount++;

        if (m_Renderer) {
            m_Renderer->BeginFrame();
        }
    }

    void UIContext::EndFrame() {
        // Clear active state if mouse released
        if (!m_MouseButtons[0]) {
            m_ActiveID.clear();
        }

        if (m_Renderer) {
            m_Renderer->EndFrame();
        }
    }

    void UIContext::Render() {
        if (m_Renderer && !m_DrawCommands.empty()) {
            m_Renderer->RenderDrawCommands(m_DrawCommands);
        }
    }

    void UIContext::OnWindowResize(uint32_t width, uint32_t height) {
        if (m_Renderer) {
            m_Renderer->OnWindowResize(width, height);
        }
    }

    void UIContext::BeginWindow(const std::string& title, const glm::vec2& pos, const glm::vec2& size) {
        static WindowData window;
        window.title = title;
        window.pos = pos;
        window.size = size;
        m_CurrentWindow = &window;

        // Reset layout
        m_LayoutStack.clear();
        LayoutContext layout;
        layout.cursor = pos + glm::vec2(10, 30);
        m_LayoutStack.push_back(layout);

        // Draw window background
        DrawCommand cmd;
        cmd.type = DrawCommand::Type::RoundedRect;
        cmd.pos = pos;
        cmd.size = size;
        cmd.color = Color::Panel;
        cmd.rounding = 8.0f;
        AddDrawCommand(cmd);

        // Draw title bar
        cmd.pos = pos;
        cmd.size = { size.x, 25.0f };
        cmd.color = Color::Primary;
        cmd.rounding = 8.0f;
        AddDrawCommand(cmd);

        // Draw title text
        DrawCommand textCmd;
        textCmd.type = DrawCommand::Type::Text;
        textCmd.pos = pos + glm::vec2(10, 5);
        textCmd.color = Color::White;
        textCmd.text = title;
        AddDrawCommand(textCmd);
    }

    void UIContext::EndWindow() {
        m_CurrentWindow = nullptr;
        if (!m_LayoutStack.empty()) {
            m_LayoutStack.pop_back();
        }
    }

    void UIContext::BeginHorizontal() {
        LayoutContext layout = m_LayoutStack.back();
        layout.direction = LayoutContext::Direction::Horizontal;
        m_LayoutStack.push_back(layout);
    }

    void UIContext::EndHorizontal() {
        if (m_LayoutStack.size() > 1) {
            auto horizontal = m_LayoutStack.back();
            m_LayoutStack.pop_back();
            m_LayoutStack.back().Advance(horizontal.contentSize);
        }
    }

    void UIContext::Spacing(float pixels) {
        auto& layout = m_LayoutStack.back();
        if (pixels == 0.0f) pixels = layout.spacing;

        if (layout.direction == LayoutContext::Direction::Vertical) {
            layout.cursor.y += pixels;
        }
        else {
            layout.cursor.x += pixels;
        }
    }

    void UIContext::Separator() {
        auto& layout = m_LayoutStack.back();

        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Line;
        cmd.pos = layout.cursor;
        cmd.size = layout.direction == LayoutContext::Direction::Vertical
            ? glm::vec2(200, 0) : glm::vec2(0, 200);
        cmd.color = Color::Border;
        cmd.thickness = 1.0f;
        AddDrawCommand(cmd);

        Spacing(layout.spacing * 2);
    }

    void UIContext::NewLine() {
        auto& layout = m_LayoutStack.back();
        layout.cursor.x = 10.0f;
        layout.cursor.y += 30.0f;
    }

    bool UIContext::Button(const std::string& label, const glm::vec2& size) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 buttonSize = size;
        if (buttonSize.x == 0) buttonSize.x = 120;
        if (buttonSize.y == 0) buttonSize.y = 30;

        glm::vec2 pos = layout.cursor;
        std::string id = GenerateID(label);

        WidgetState state = ProcessWidget(pos, buttonSize);
        m_LastWidgetState = state;

        // Choose color based on state
        glm::vec4 color = Color::ButtonNormal;
        if (state.active) color = Color::ButtonActive;
        else if (state.hovered) color = Color::ButtonHover;

        // Draw button background
        DrawCommand bgCmd;
        bgCmd.type = DrawCommand::Type::RoundedRect;
        bgCmd.pos = pos;
        bgCmd.size = buttonSize;
        bgCmd.color = color;
        bgCmd.rounding = 4.0f;
        AddDrawCommand(bgCmd);

        // Draw button text
        glm::vec2 textSize = CalcTextSize(label);
        glm::vec2 textPos = pos + (buttonSize - textSize) * 0.5f;

        DrawCommand textCmd;
        textCmd.type = DrawCommand::Type::Text;
        textCmd.pos = textPos;
        textCmd.color = Color::White;
        textCmd.text = label;
        AddDrawCommand(textCmd);

        layout.Advance(buttonSize);

        return state.clicked;
    }

    void UIContext::Text(const std::string& text) {
        TextColored(Color::Text, text);
    }

    void UIContext::TextColored(const glm::vec4& color, const std::string& text) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 textSize = CalcTextSize(text);

        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Text;
        cmd.pos = layout.cursor;
        cmd.color = color;
        cmd.text = text;
        cmd.textDirection = 0; // Auto
        AddDrawCommand(cmd);

        layout.Advance(textSize);
    }

    void UIContext::TextLTR(const std::string& text) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 textSize = CalcTextSize(text);

        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Text;
        cmd.pos = layout.cursor;
        cmd.color = Color::Text;
        cmd.text = text;
        cmd.textDirection = 1; // LTR
        AddDrawCommand(cmd);

        layout.Advance(textSize);
    }

    void UIContext::TextRTL(const std::string& text) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 textSize = CalcTextSize(text);

        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Text;
        cmd.pos = layout.cursor;
        cmd.color = Color::Text;
        cmd.text = text;
        cmd.textDirection = 2; // RTL
        AddDrawCommand(cmd);

        layout.Advance(textSize);
    }

    bool UIContext::Checkbox(const std::string& label, bool* value) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 boxSize(20, 20);
        glm::vec2 pos = layout.cursor;

        std::string id = GenerateID(label);
        WidgetState state = ProcessWidget(pos, boxSize);
        m_LastWidgetState = state;

        if (state.clicked && value) {
            *value = !(*value);
        }

        DrawCommand boxCmd;
        boxCmd.type = DrawCommand::Type::RoundedRect;
        boxCmd.pos = pos;
        boxCmd.size = boxSize;
        boxCmd.color = state.hovered ? Color::ButtonHover : Color::ButtonNormal;
        boxCmd.rounding = 2.0f;
        AddDrawCommand(boxCmd);

        if (value && *value) {
            DrawCommand checkCmd;
            checkCmd.type = DrawCommand::Type::Rect;
            checkCmd.pos = pos + glm::vec2(5, 5);
            checkCmd.size = glm::vec2(10, 10);
            checkCmd.color = Color::White;
            AddDrawCommand(checkCmd);
        }

        DrawCommand textCmd;
        textCmd.type = DrawCommand::Type::Text;
        textCmd.pos = pos + glm::vec2(25, 2);
        textCmd.color = Color::Text;
        textCmd.text = label;
        AddDrawCommand(textCmd);

        glm::vec2 totalSize(boxSize.x + 25 + CalcTextSize(label).x, boxSize.y);
        layout.Advance(totalSize);

        return state.clicked;
    }

    bool UIContext::InputText(const std::string& label, std::string& buffer, size_t maxLength) {
        Text(label + ": [Input not implemented]");
        return false;
    }

    bool UIContext::InputFloat(const std::string& label, float* value, float step) {
        Text(label + ": [Input not implemented]");
        return false;
    }

    bool UIContext::SliderFloat(const std::string& label, float* value, float min, float max) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 sliderSize(200, 20);
        glm::vec2 pos = layout.cursor;

        Text(label);
        pos = layout.cursor;

        std::string id = GenerateID(label);
        WidgetState state = ProcessWidget(pos, sliderSize);
        m_LastWidgetState = state;

        if (state.active && value) {
            float normalizedX = (m_MousePos.x - pos.x) / sliderSize.x;
            normalizedX = glm::clamp(normalizedX, 0.0f, 1.0f);
            *value = min + normalizedX * (max - min);
        }

        DrawCommand bgCmd;
        bgCmd.type = DrawCommand::Type::RoundedRect;
        bgCmd.pos = pos;
        bgCmd.size = sliderSize;
        bgCmd.color = Color::ButtonNormal;
        bgCmd.rounding = 10.0f;
        AddDrawCommand(bgCmd);

        if (value) {
            float fillWidth = (((*value) - min) / (max - min)) * sliderSize.x;
            DrawCommand fillCmd;
            fillCmd.type = DrawCommand::Type::RoundedRect;
            fillCmd.pos = pos;
            fillCmd.size = glm::vec2(fillWidth, sliderSize.y);
            fillCmd.color = Color::Primary;
            fillCmd.rounding = 10.0f;
            AddDrawCommand(fillCmd);
        }

        if (value) {
            char valueText[32];
            snprintf(valueText, sizeof(valueText), "%.2f", *value);
            DrawCommand textCmd;
            textCmd.type = DrawCommand::Type::Text;
            textCmd.pos = pos + glm::vec2(sliderSize.x + 10, 2);
            textCmd.color = Color::Text;
            textCmd.text = valueText;
            AddDrawCommand(textCmd);
        }

        layout.Advance(sliderSize);

        return state.active;
    }

    void UIContext::Panel(const glm::vec2& size, const std::function<void()>& content) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 pos = layout.cursor;

        DrawCommand cmd;
        cmd.type = DrawCommand::Type::RoundedRect;
        cmd.pos = pos;
        cmd.size = size;
        cmd.color = Color::Panel;
        cmd.rounding = 6.0f;
        AddDrawCommand(cmd);

        LayoutContext panelLayout;
        panelLayout.cursor = pos + glm::vec2(10, 10);
        m_LayoutStack.push_back(panelLayout);

        if (content) content();

        m_LayoutStack.pop_back();
        layout.Advance(size);
    }

    std::string UIContext::GenerateID(const std::string& label) {
        std::string id;
        for (const auto& parent : m_IDStack) {
            id += parent + "/";
        }
        id += label;
        return id;
    }

    WidgetState UIContext::ProcessWidget(const glm::vec2& pos, const glm::vec2& size) {
        WidgetState state;

        state.hovered = m_MousePos.x >= pos.x && m_MousePos.x <= pos.x + size.x &&
            m_MousePos.y >= pos.y && m_MousePos.y <= pos.y + size.y;

        if (state.hovered && m_MouseButtons[0]) {
            state.active = true;
        }

        static bool wasPressed = false;
        if (m_MouseButtons[0]) {
            wasPressed = true;
        }
        if (state.hovered && !m_MouseButtons[0] && wasPressed) {
            state.clicked = true;
            wasPressed = false;
        }
        if (!m_MouseButtons[0]) {
            wasPressed = false;
        }

        return state;
    }

    void UIContext::AddDrawCommand(const DrawCommand& cmd) {
        m_DrawCommands.push_back(cmd);
    }

    glm::vec2 UIContext::CalcTextSize(const std::string& text) {
        // Use the renderer's font manager for accurate text size
        if (m_Renderer && m_Renderer->GetFontManager().GetTextShaper().GetDirection() != TextShaper::TextDirection::Auto) {
            auto& fontManager = m_Renderer->GetFontManager();
            return fontManager.GetTextShaper().CalculateTextSize(text);
        }

        // Fallback: use font manager's shaped text calculation
        if (m_Renderer) {
            auto& fontManager = m_Renderer->GetFontManager();
            auto shapedGlyphs = fontManager.ShapeText(text);

            float width = 0.0f;
            float height = 16.0f; // Default height

            for (const auto& glyph : shapedGlyphs) {
                width += glyph.advance.x;
            }

            return glm::vec2(width, height);
        }

        // Ultimate fallback
        return glm::vec2(text.length() * 8.0f, 16.0f);
    }

    bool UIContext::IsMouseButtonDown(int button) const {
        return button >= 0 && button < 3 ? m_MouseButtons[button] : false;
    }

    bool UIContext::IsKeyPressed(int key) const {
        return Input::IsKeyPressed(key);
    }

    void UIContext::BeginScrollablePanel(const std::string& id, const glm::vec2& size) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 pos = layout.cursor;

        // Get or create scroll region
        if (m_ScrollRegions.find(id) == m_ScrollRegions.end()) {
            ScrollableRegion region;
            region.id = id;
            region.pos = pos;
            region.size = size;
            region.scrollOffset = glm::vec2(0, 0);
            region.contentSize = glm::vec2(0, 0);
            m_ScrollRegions[id] = region;
        }

        auto& region = m_ScrollRegions[id];
        region.pos = pos;
        region.size = size;
        m_ActiveScrollRegionID = id;

        // Draw panel background
        DrawCommand bgCmd;
        bgCmd.type = DrawCommand::Type::RoundedRect;
        bgCmd.pos = pos;
        bgCmd.size = size;
        bgCmd.color = Color::Panel;
        bgCmd.rounding = 6.0f;
        AddDrawCommand(bgCmd);

        // ADD SCISSOR START COMMAND
        DrawCommand scissorCmd;
        scissorCmd.type = DrawCommand::Type::PushScissor;
        scissorCmd.pos = pos;
        scissorCmd.size = size;
        AddDrawCommand(scissorCmd);

        // Handle scrolling
        bool mouseInPanel = IsPointInRect(m_MousePos, pos, size);
        if (mouseInPanel && m_MouseButtons[0] && !region.isDragging) {
            region.isDragging = true;
            region.dragStartMouse = m_MousePos;
            region.dragStartScroll = region.scrollOffset;
        }

        if (region.isDragging) {
            glm::vec2 mouseDelta = m_MousePos - region.dragStartMouse;
            region.scrollOffset = region.dragStartScroll + mouseDelta;
            float maxScrollY = glm::max(0.0f, region.contentSize.y - size.y + 20.0f);
            region.scrollOffset.y = glm::clamp(region.scrollOffset.y, -maxScrollY, 0.0f);
            region.scrollOffset.x = 0.0f;
            if (!m_MouseButtons[0]) {
                region.isDragging = false;
            }
        }

        // Set up scrolled layout
        LayoutContext scrolledLayout;
        scrolledLayout.cursor = pos + glm::vec2(10, 10) + region.scrollOffset;
        scrolledLayout.direction = LayoutContext::Direction::Vertical;
        m_LayoutStack.push_back(scrolledLayout);
    }

    void UIContext::EndScrollablePanel() {
        if (m_ActiveScrollRegionID.empty()) return;

        auto& region = m_ScrollRegions[m_ActiveScrollRegionID];

        // Calculate content size
        if (!m_LayoutStack.empty()) {
            auto& scrolledLayout = m_LayoutStack.back();
            region.contentSize = scrolledLayout.contentSize;
            m_LayoutStack.pop_back();
        }

        // ADD SCISSOR END COMMAND
        DrawCommand scissorCmd;
        scissorCmd.type = DrawCommand::Type::PopScissor;
        AddDrawCommand(scissorCmd);

        // Draw scrollbar (outside scissor)
        float contentHeight = region.contentSize.y;
        float panelHeight = region.size.y - 20.0f;

        if (contentHeight > panelHeight) {
            float scrollbarHeight = region.size.y;
            float thumbHeight = glm::max(20.0f, (panelHeight / contentHeight) * scrollbarHeight);
            float maxScroll = contentHeight - panelHeight;
            float scrollRatio = -region.scrollOffset.y / maxScroll;
            float thumbY = region.pos.y + scrollRatio * (scrollbarHeight - thumbHeight);

            DrawCommand trackCmd;
            trackCmd.type = DrawCommand::Type::RoundedRect;
            trackCmd.pos = glm::vec2(region.pos.x + region.size.x - 12, region.pos.y);
            trackCmd.size = glm::vec2(8, scrollbarHeight);
            trackCmd.color = glm::vec4(0.2f, 0.2f, 0.2f, 0.5f);
            trackCmd.rounding = 4.0f;
            AddDrawCommand(trackCmd);

            DrawCommand thumbCmd;
            thumbCmd.type = DrawCommand::Type::RoundedRect;
            thumbCmd.pos = glm::vec2(region.pos.x + region.size.x - 12, thumbY);
            thumbCmd.size = glm::vec2(8, thumbHeight);
            thumbCmd.color = Color::Primary;
            thumbCmd.rounding = 4.0f;
            AddDrawCommand(thumbCmd);
        }

        if (!m_LayoutStack.empty()) {
            m_LayoutStack.back().Advance(region.size);
        }

        m_ActiveScrollRegionID.clear();
    }

    bool UIContext::IsPointInRect(const glm::vec2& point, const glm::vec2& rectPos,
        const glm::vec2& rectSize) {
        return point.x >= rectPos.x && point.x <= rectPos.x + rectSize.x &&
            point.y >= rectPos.y && point.y <= rectPos.y + rectSize.y;
    }

} // namespace Unicorn::UI