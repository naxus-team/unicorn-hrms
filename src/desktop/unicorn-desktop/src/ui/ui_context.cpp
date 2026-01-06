#include "ui_context.h"
#include "ui_renderer.h"
#include "../core/input.h"
#include "../core/application.h"
#include "../core/window.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>


namespace Unicorn::UI {

    // Color definitions
    const glm::vec4 Color::Transparent = { 1.0f, 1.0f, 1.0f, 0.0f };
    const glm::vec4 Color::White = { 1.0f, 1.0f, 1.0f, 1.0f };
    const glm::vec4 Color::Black = { 0.0f, 0.0f, 0.0f, 0.8f };
    const glm::vec4 Color::Primary = { 0.26f, 0.59f, 0.98f, 1.0f };
    const glm::vec4 Color::Secondary = { 0.6f, 0.2f, 0.8f, 1.0f };
    const glm::vec4 Color::Background = { 0.12f, 0.12f, 0.12f, 1.0f };
    const glm::vec4 Color::Panel = { 0.18f, 0.18f, 0.18f, 1.0f };
    const glm::vec4 Color::Border = { 0.90f, 0.90f, 0.90f, 1.0f };
    const glm::vec4 Color::Text = { 0.0f, 0.0f, 0.0f, 1.0f };
    const glm::vec4 Color::TextDisabled = { 0.5f, 0.5f, 0.5f, 1.0f };
    const glm::vec4 Color::ButtonNormal = { 1.0f, 1.0f, 1.0f, 1.0f };
    const glm::vec4 Color::ButtonHover = { 0.92f, 0.92f, 0.92f, 1.0f };
    const glm::vec4 Color::ButtonActive = { 0.9f, 0.9f, 0.9f, 1.0f };

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

        m_IconManager = std::make_unique<IconManager>();
        if (!m_IconManager->Init()) {
            std::cerr << "[UI] Failed to initialize Icon Manager!" << std::endl;
        }

        m_AnimController = std::make_unique<AnimationController>();

        std::cout << "[UI] Initialized with viewport: " << width << "x" << height << std::endl;
        std::cout << "[UI] Icon Manager loaded with " << m_IconManager->GetIcon("add") << " icons" << std::endl;
    }

    void UIContext::Shutdown() {
        std::cout << "[UI] Shutting down UI Context..." << std::endl;

        if (m_IconManager) {
            m_IconManager->Shutdown();
            m_IconManager.reset();
        }

        if (m_AnimController) {
            m_AnimController.reset();
        }

        if (m_Renderer) {
            m_Renderer->Shutdown();
        }
    }

    void UIContext::BeginFrame() {
        m_DrawCommands.clear();
        m_IDStack.clear();
        m_LastWidgetState = WidgetState();

        // Update input

        glm::vec2 oldMousePos = m_MousePos;
        bool oldMouseButtons[3] = { m_MouseButtons[0], m_MouseButtons[1], m_MouseButtons[2] };


        m_LastMousePos = m_MousePos;
        m_MousePos = Input::GetMousePosition();
        m_MouseButtons[0] = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        m_MouseButtons[1] = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
        m_MouseButtons[2] = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE);

        if (oldMousePos != m_MousePos ||
            oldMouseButtons[0] != m_MouseButtons[0] ||
            oldMouseButtons[1] != m_MouseButtons[1] ||
            oldMouseButtons[2] != m_MouseButtons[2]) {
            m_IsDirty = true;
        }

        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        m_DeltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        if (m_AnimController) {
            m_AnimController->Update(m_DeltaTime);
            if (m_AnimController->HasActiveAnimations()) {
                m_IsDirty = true;
            }
        }

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

    void UIContext::BeginWindow(const std::string& title,
        const glm::vec2& pos,
        const glm::vec2& size,
        const WindowBorderStyle& borderStyle) {
        static WindowData window;
        window.title = title;
        window.pos = pos;
        window.size = size;
        m_CurrentWindow = &window;

        glm::vec4 bgColor = Color::Panel;
        if (title == "##sidebar") {
            bgColor = Color::Transparent;
        }

        glm::vec2 finalPos = pos;
        if (m_GlobalScroll.active) {
            finalPos.y += m_GlobalScroll.offset.y;
        }

        m_LayoutStack.clear();
        LayoutContext layout;
        layout.cursor = finalPos + glm::vec2(10, 10);
        m_LayoutStack.push_back(layout);

        DrawCommand cmd;
        cmd.type = DrawCommand::Type::RoundedRect;
        cmd.pos = pos;
        cmd.size = size;
        cmd.color = (title == "##sidebar") ? Color::Transparent : Color::Transparent;
        cmd.rounding = (title == "##sidebar") ? 0.0f : 8.0f;
        m_DrawCommands.push_back(cmd);

        if (borderStyle.enabled) {
            float thickness = borderStyle.thickness;
            glm::vec4 borderColor = borderStyle.color;

            if (borderStyle.top) {
                DrawCommand topBorder;
                topBorder.type = DrawCommand::Type::Rect;
                topBorder.pos = pos;
                topBorder.size = glm::vec2(size.x, thickness);
                topBorder.color = borderColor;
                AddDrawCommand(topBorder);
            }

            if (borderStyle.bottom) {
                DrawCommand bottomBorder;
                bottomBorder.type = DrawCommand::Type::Rect;
                bottomBorder.pos = pos + glm::vec2(0, size.y - thickness);
                bottomBorder.size = glm::vec2(size.x, thickness);
                bottomBorder.color = borderColor;
                AddDrawCommand(bottomBorder);
            }

            if (borderStyle.left) {
                DrawCommand leftBorder;
                leftBorder.type = DrawCommand::Type::Rect;
                leftBorder.pos = pos;
                leftBorder.size = glm::vec2(thickness, size.y);
                leftBorder.color = borderColor;
                AddDrawCommand(leftBorder);
            }

            if (borderStyle.right) {
                DrawCommand rightBorder;
                rightBorder.type = DrawCommand::Type::Rect;
                rightBorder.pos = pos + glm::vec2(size.x - thickness, 0);
                rightBorder.size = glm::vec2(thickness, size.y);
                rightBorder.color = borderColor;
                AddDrawCommand(rightBorder);
            }
        }
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

    void UIContext::Separator(float line, int weight) {
        auto& layout = m_LayoutStack.back();
		float defaultRoundedness = 2.0f;

        DrawCommand cmd;
        cmd.type = DrawCommand::Type::Line;
        cmd.pos = layout.cursor;
        cmd.rounding = defaultRoundedness;
        cmd.size = layout.direction == LayoutContext::Direction::Vertical
            ? glm::vec2(weight, 0) : glm::vec2(0, weight);
        cmd.color = Color::Border;
        cmd.thickness = line;
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

        std::string widgetID = GenerateID("widget_" +
            std::to_string((int)pos.x) + "_" + std::to_string((int)pos.y));

        state.hovered = m_MousePos.x >= pos.x && m_MousePos.x <= pos.x + size.x &&
            m_MousePos.y >= pos.y && m_MousePos.y <= pos.y + size.y;

        // Check active (pressed)
        if (state.hovered && m_MouseButtons[0]) {
            state.active = true;
            m_WidgetPressStates[widgetID] = true;
        }

        // Check clicked (released while hovering)
        if (state.hovered && !m_MouseButtons[0] && m_WidgetPressStates[widgetID]) {
            state.clicked = true;
            m_WidgetPressStates[widgetID] = false;
        }

        // Clear press state if mouse released anywhere
        if (!m_MouseButtons[0]) {
            m_WidgetPressStates[widgetID] = false;
        }

        return state;
    }

    void UIContext::AddDrawCommand(const DrawCommand& cmd) {
        DrawCommand modifiedCmd = cmd;

        // Apply global scroll offset to position-based commands
        if (m_GlobalScroll.active) {
            switch (cmd.type) {
            case DrawCommand::Type::Rect:
            case DrawCommand::Type::RoundedRect:
            case DrawCommand::Type::Text:
            case DrawCommand::Type::Line:
            case DrawCommand::Type::Icon:
                modifiedCmd.pos.y += m_GlobalScroll.offset.y;
                break;
            default:
                break;
            }
        }

        m_DrawCommands.push_back(modifiedCmd);
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

        bool mouseInPanel = IsPointInRect(m_MousePos, pos, size);

        // Handle scrolling
        if (mouseInPanel) {
            float wheelDelta = Input::GetMouseWheelDelta();
            if (wheelDelta != 0.0f) {
                // Scroll speed: 40 pixels per wheel notch
                region.scrollOffset.y += wheelDelta * 40.0f;

                // Clamp scroll
                float maxScrollY = glm::max(0.0f, region.contentSize.y - size.y + 20.0f);
                region.scrollOffset.y = glm::clamp(region.scrollOffset.y, -maxScrollY, 0.0f);
            }
        }

        // Mouse drag scrolling (fallback)
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

    void UIContext::BeginGlobalScroll(const glm::vec2& pos, const glm::vec2& size) {
        m_GlobalScroll.active = true;
        m_GlobalScroll.viewportPos = pos;
        m_GlobalScroll.viewportSize = size;

        bool mouseInViewport = IsPointInRect(m_MousePos, pos, size);

        if (mouseInViewport) {
            float wheelDelta = Input::GetMouseWheelDelta();
            if (wheelDelta != 0.0f) {
                m_GlobalScroll.offset.y += wheelDelta * 60.0f; // 60 pixels per notch

                // Clamp scroll offset
                m_GlobalScroll.offset.y = glm::clamp(
                    m_GlobalScroll.offset.y,
                    -m_GlobalScroll.maxScroll,
                    0.0f
                );
            }
        }

        // Handle scroll via right mouse button drag (fallback)
        static bool wasDragging = false;
        static glm::vec2 dragStartPos;
        static glm::vec2 scrollStartOffset;

        if (mouseInViewport && m_MouseButtons[1] && !wasDragging) {
            wasDragging = true;
            dragStartPos = m_MousePos;
            scrollStartOffset = m_GlobalScroll.offset;
        }

        if (wasDragging && m_MouseButtons[1]) {
            glm::vec2 mouseDelta = m_MousePos - dragStartPos;
            m_GlobalScroll.offset.y = scrollStartOffset.y + mouseDelta.y;

            // Clamp scroll offset
            m_GlobalScroll.offset.y = glm::clamp(
                m_GlobalScroll.offset.y,
                -m_GlobalScroll.maxScroll,
                0.0f
            );
        }
        else {
            wasDragging = false;
        }
    }

    void UIContext::EndGlobalScroll() {
        if (!m_GlobalScroll.active) return;

        // Calculate total content height from all draw commands
        m_GlobalScroll.contentHeight = 0.0f;

        for (const auto& cmd : m_DrawCommands) {
            if (cmd.type == DrawCommand::Type::Rect ||
                cmd.type == DrawCommand::Type::RoundedRect) {
                float bottom = cmd.pos.y + cmd.size.y;
                m_GlobalScroll.contentHeight = glm::max(m_GlobalScroll.contentHeight, bottom);
            }
        }

        // Calculate max scroll
        m_GlobalScroll.maxScroll = glm::max(
            0.0f,
            m_GlobalScroll.contentHeight - m_GlobalScroll.viewportSize.y + 50.0f
        );

        // Draw scrollbar if content exceeds viewport
        if (m_GlobalScroll.maxScroll > 0.0f) {
            float scrollbarX = m_GlobalScroll.viewportPos.x + m_GlobalScroll.viewportSize.x - 12.0f;
            float scrollbarHeight = m_GlobalScroll.viewportSize.y;

            // Calculate thumb size and position
            float visibleRatio = m_GlobalScroll.viewportSize.y / m_GlobalScroll.contentHeight;
            float thumbHeight = glm::max(30.0f, scrollbarHeight * visibleRatio);

            float scrollRatio = -m_GlobalScroll.offset.y / m_GlobalScroll.maxScroll;
            float thumbY = m_GlobalScroll.viewportPos.y + scrollRatio * (scrollbarHeight - thumbHeight);

            // Draw scrollbar track
            DrawCommand trackCmd;
            trackCmd.type = DrawCommand::Type::RoundedRect;
            trackCmd.pos = glm::vec2(scrollbarX, m_GlobalScroll.viewportPos.y);
            trackCmd.size = glm::vec2(8, scrollbarHeight);
            trackCmd.color = glm::vec4(0.2f, 0.2f, 0.2f, 0.3f);
            trackCmd.rounding = 4.0f;
            AddDrawCommand(trackCmd);

            // Draw scrollbar thumb
            DrawCommand thumbCmd;
            thumbCmd.type = DrawCommand::Type::RoundedRect;
            thumbCmd.pos = glm::vec2(scrollbarX, thumbY);
            thumbCmd.size = glm::vec2(8, thumbHeight);
            thumbCmd.color = glm::vec4(0.6f, 0.6f, 0.6f, 0.8f);
            thumbCmd.rounding = 4.0f;
            AddDrawCommand(thumbCmd);
        }

        m_GlobalScroll.active = false;
    }

    bool UIContext::IconButton(const std::string& iconName,
        const glm::vec2& size,
        Alignment align) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 buttonSize = size;
        glm::vec2 pos = layout.cursor;

        if (m_CurrentWindow) {
            float availableWidth = m_CurrentWindow->size.x - 22.0f;

            switch (align) {
            case Alignment::Center:
                pos.x = m_CurrentWindow->pos.x + 10.0f + (availableWidth - buttonSize.x) * 0.5f;
                break;
            case Alignment::Right:
                pos.x = m_CurrentWindow->pos.x + 10.0f + availableWidth - buttonSize.x;
                break;
            case Alignment::Left:
            default:
                break;
            }
        }

        std::string id = GenerateID(iconName);
        WidgetState state = ProcessWidget(pos, buttonSize);
        m_LastWidgetState = state;

        if (!m_AnimController) {
            DrawCommand bgCmd;
            bgCmd.type = DrawCommand::Type::RoundedRect;
            bgCmd.pos = pos;
            bgCmd.size = buttonSize;
            bgCmd.color = state.hovered ? Color::ButtonHover : Color::ButtonNormal;
            bgCmd.rounding = 6.0f;
            AddDrawCommand(bgCmd);

            if (align == Alignment::Left) {
                layout.Advance(buttonSize);
            }
            else {
                layout.cursor.y += buttonSize.y + layout.spacing;
            }

            return state.clicked;
        }

        auto& animState = m_AnimController->GetButtonState(id);
        float deltaTime = m_DeltaTime > 0 ? m_DeltaTime : 0.016f;

        const float hoverSpeed = 16.0f;
        const float activeSpeed = 16.0f;

        if (state.hovered) {
            animState.hoverProgress = glm::min(1.0f, animState.hoverProgress + deltaTime * hoverSpeed);
        }
        else {
            animState.hoverProgress = glm::max(0.0f, animState.hoverProgress - deltaTime * hoverSpeed);
        }

        if (state.active) {
            animState.activeProgress = glm::min(1.0f, animState.activeProgress + deltaTime * activeSpeed);
        }
        else {
            animState.activeProgress = glm::max(0.0f, animState.activeProgress - deltaTime * activeSpeed);
        }

        float smoothHover = animState.hoverProgress * animState.hoverProgress *
            (3.0f - 2.0f * animState.hoverProgress);
        float smoothActive = animState.activeProgress * animState.activeProgress *
            (3.0f - 2.0f * animState.activeProgress);

        float hoverScale = 1.0f + (smoothHover * 0.03f);
        float activeScale = 1.0f - (smoothActive * 0.04f);
        float finalScale = hoverScale * activeScale;

        glm::vec4 baseColor = Color::Transparent;
        glm::vec4 hoverColor = Color::ButtonHover;
        glm::vec4 activeColor = Color::ButtonActive;

        glm::vec4 currentColor = AnimationController::LerpColor(baseColor, hoverColor, smoothHover);
        currentColor = AnimationController::LerpColor(currentColor, activeColor, smoothActive);

        glm::vec2 scaledSize = buttonSize * finalScale;
        glm::vec2 centerOffset = (buttonSize - scaledSize) * 0.5f;
        glm::vec2 finalPos = pos + centerOffset;

        DrawCommand bgCmd;
        bgCmd.type = DrawCommand::Type::RoundedRect;
        bgCmd.pos = finalPos;
        bgCmd.size = scaledSize;
        bgCmd.color = currentColor;
        bgCmd.rounding = 6.0f;
        AddDrawCommand(bgCmd);

        if (m_IconManager) {
            const IconManager::Icon* icon = m_IconManager->GetIcon(iconName);
            if (icon) {
                float iconSize = 20.0f;

                glm::vec2 iconPos;
                iconPos = finalPos + glm::vec2(
                    (scaledSize.x - iconSize) * 0.5f,
                    (scaledSize.y - iconSize) * 0.5f
                );

                DrawCommand iconCmd;
                iconCmd.type = DrawCommand::Type::Icon;
                iconCmd.pos = iconPos;
                iconCmd.size = glm::vec2(iconSize, iconSize);
                iconCmd.textureID = icon->textureID;

                iconCmd.color = Color::Black;


                AddDrawCommand(iconCmd);
            }
        }

        if (align == Alignment::Left) {
            layout.Advance(buttonSize);
        }
        else {
            layout.cursor.y += buttonSize.y + layout.spacing;
        }

        return state.clicked;
    }

    bool UIContext::ButtonWithIcon(const std::string& iconName,
        const std::string& label,
        const glm::vec2& size,
        Alignment align) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 buttonSize = size;
        glm::vec2 pos = layout.cursor;

        if (m_CurrentWindow) {
            float availableWidth = m_CurrentWindow->size.x - 20.0f;

            switch (align) {
            case Alignment::Center:
                pos.x = m_CurrentWindow->pos.x + 10.0f + (availableWidth - buttonSize.x) * 0.5f;
                break;
            case Alignment::Right:
                pos.x = m_CurrentWindow->pos.x + 10.0f + availableWidth - buttonSize.x;
                break;
            case Alignment::Left:
            default:
                break;
            }
        }

        std::string id = GenerateID(label.empty() ? iconName : label);
        WidgetState state = ProcessWidget(pos, buttonSize);
        m_LastWidgetState = state;

        if (!m_AnimController) {
            DrawCommand bgCmd;
            bgCmd.type = DrawCommand::Type::RoundedRect;
            bgCmd.pos = pos;
            bgCmd.size = buttonSize;
            bgCmd.color = state.hovered ? Color::ButtonHover : Color::ButtonNormal;
            bgCmd.rounding = 6.0f;
            AddDrawCommand(bgCmd);

            if (!label.empty()) {
                glm::vec2 textSize = CalcTextSize(label);
                glm::vec2 textPos = pos + (buttonSize - textSize) * 0.5f;
                DrawCommand textCmd;
                textCmd.type = DrawCommand::Type::Text;
                textCmd.pos = textPos;
                textCmd.color = Color::Black;
                textCmd.text = label;
                AddDrawCommand(textCmd);
            }

            if (align == Alignment::Left) {
                layout.Advance(buttonSize);
            }
            else {
                layout.cursor.y += buttonSize.y + layout.spacing;
            }

            return state.clicked;
        }

        auto& animState = m_AnimController->GetButtonState(id);
        float deltaTime = m_DeltaTime > 0 ? m_DeltaTime : 0.016f;

        const float hoverSpeed = 16.0f;
        const float activeSpeed = 16.0f;

        if (state.hovered) {
            animState.hoverProgress = glm::min(1.0f, animState.hoverProgress + deltaTime * hoverSpeed);
        }
        else {
            animState.hoverProgress = glm::max(0.0f, animState.hoverProgress - deltaTime * hoverSpeed);
        }

        if (state.active) {
            animState.activeProgress = glm::min(1.0f, animState.activeProgress + deltaTime * activeSpeed);
        }
        else {
            animState.activeProgress = glm::max(0.0f, animState.activeProgress - deltaTime * activeSpeed);
        }

        float smoothHover = animState.hoverProgress * animState.hoverProgress *
            (3.0f - 2.0f * animState.hoverProgress);
        float smoothActive = animState.activeProgress * animState.activeProgress *
            (3.0f - 2.0f * animState.activeProgress);

        float hoverScale = 1.0f + (smoothHover * 0.03f);
        float activeScale = 1.0f - (smoothActive * 0.04f);
        float finalScale = hoverScale * activeScale;

        glm::vec4 baseColor = Color::Transparent;
        glm::vec4 hoverColor = Color::ButtonHover;
        glm::vec4 activeColor = Color::ButtonActive;

        glm::vec4 currentColor = AnimationController::LerpColor(baseColor, hoverColor, smoothHover);
        currentColor = AnimationController::LerpColor(currentColor, activeColor, smoothActive);

        glm::vec2 scaledSize = buttonSize * finalScale;
        glm::vec2 centerOffset = (buttonSize - scaledSize) * 0.5f;
        glm::vec2 finalPos = pos + centerOffset;

        DrawCommand bgCmd;
        bgCmd.type = DrawCommand::Type::RoundedRect;
        bgCmd.pos = finalPos;
        bgCmd.size = scaledSize;
        bgCmd.color = currentColor;
        bgCmd.rounding = 6.0f;
        AddDrawCommand(bgCmd);

        if (m_IconManager) {
            const IconManager::Icon* icon = m_IconManager->GetIcon(iconName);
            if (icon) {
                float iconSize = 20.0f;

                glm::vec2 iconPos;
                if (label.empty()) {
                    iconPos = finalPos + glm::vec2(
                        (scaledSize.x - iconSize) * 0.5f,
                        (scaledSize.y - iconSize) * 0.5f
                    );
                }
                else {
                    iconPos = finalPos + glm::vec2(8.0f, (scaledSize.y - iconSize) * 0.5f);
                }

                DrawCommand iconCmd;
                iconCmd.type = DrawCommand::Type::Icon;
                iconCmd.pos = iconPos;
                iconCmd.size = glm::vec2(iconSize, iconSize);
                iconCmd.textureID = icon->textureID;

                iconCmd.color = Color::Black;


                AddDrawCommand(iconCmd);
            }
        }

        if (!label.empty()) {
            glm::vec2 textSize = CalcTextSize(label);
            glm::vec2 textPos = finalPos + glm::vec2(36.0f, (scaledSize.y - textSize.y) * 0.5f);

            DrawCommand textCmd;
            textCmd.type = DrawCommand::Type::Text;
            textCmd.pos = textPos;
            textCmd.color = Color::Black;
            textCmd.text = label;
            AddDrawCommand(textCmd);
        }

        if (align == Alignment::Left) {
            layout.Advance(buttonSize);
        }
        else {
            layout.cursor.y += buttonSize.y + layout.spacing;
        }

        return state.clicked;
    }

    bool UIContext::IsPointInRect(const glm::vec2& point, const glm::vec2& rectPos,
        const glm::vec2& rectSize) {
        return point.x >= rectPos.x && point.x <= rectPos.x + rectSize.x &&
            point.y >= rectPos.y && point.y <= rectPos.y + rectSize.y;
    }

} // namespace Unicorn::UI