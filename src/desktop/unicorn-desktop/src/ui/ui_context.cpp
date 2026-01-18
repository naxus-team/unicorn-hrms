#include "ui_context.h"
#include "ui_renderer.h"
#include "../core/input.h"
#include "../core/application.h"
#include "../core/window.h"
#include "helpers/colors.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <algorithm>

namespace Unicorn::UI {

    UIContext::UIContext() {
        m_LayoutStack.push_back(LayoutContext());
        m_Renderer = std::make_unique<UIRenderer>();
    }

    UIContext::~UIContext() {}

    void UIContext::Init() {
        std::cout << "[UI] Initializing UI Context..." << std::endl;

        Application& app = Application::Get();
        Window& window = app.GetWindow();
        uint32_t width = window.GetWidth();
        uint32_t height = window.GetHeight();

        // ⭐ Initialize renderer with MSAA4x mode and 14px fonts
        m_Renderer->Init(width, height, UI::MSAAMode::MSAA4x);

        m_IconManager = std::make_unique<IconManager>();
        if (!m_IconManager->Init()) {
            std::cerr << "[UI] Failed to initialize Icon Manager!" << std::endl;
        }

        m_AnimController = std::make_unique<AnimationController>();

        std::cout << "[UI] Initialized with viewport: " << width << "x" << height << std::endl;
        std::cout << "[UI] MSAA Mode: 4x (Balanced quality)" << std::endl;
        std::cout << "[UI] Font Size: 14px" << std::endl;
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

        m_LastMousePos = m_MousePos;
        m_MousePos = Input::GetMousePosition();
        m_MouseButtons[0] = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        m_MouseButtons[1] = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
        m_MouseButtons[2] = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE);
        m_MouseWheelDelta = Input::GetMouseWheelDelta();

        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        m_DeltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        UpdatePhysicsScroll(m_DeltaTime);

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

        // Update cursor based on what's being hovered
        int newCursor = 0; // Default arrow

        // Check if hovering any widget
        if (!m_LastHoveredWidgets.empty()) {
            // If hovering input field, show I-beam
            if (!m_ActiveInputID.empty() && m_ActiveInputID.find("##") != std::string::npos) {
                newCursor = 2; // I-beam cursor
            }
            else {
                newCursor = 1; // Hand cursor for buttons
            }
        }

        // Only update if cursor changed
        if (newCursor != m_CurrentCursor) {
            m_CurrentCursor = newCursor;
            Application::Get().GetWindow().SetCursor(newCursor);
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

        glm::vec4 bgColor = Unicorn::UI::Color::Panel;
        if (title == "##sidebar") {
            bgColor = Unicorn::UI::Color::Transparent;
        }

        glm::vec2 finalPos = pos;
        if (m_GlobalScroll.active) {
            finalPos.y += m_GlobalScroll.physics.offset.y;
        }

        m_LayoutStack.clear();
        LayoutContext layout;
        layout.cursor = finalPos + glm::vec2(10, 10);
        m_LayoutStack.push_back(layout);

        DrawCommand cmd;
        cmd.type = DrawCommand::Type::RoundedRect;
        cmd.pos = pos;
        cmd.size = size;
        cmd.color = (title == "##sidebar") ? Unicorn::UI::Color::Transparent : Unicorn::UI::Color::Transparent;
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
        // Track the bottom of the current window for content height calculation
        if (m_CurrentWindow && m_GlobalScroll.active) {
            float windowBottom = m_CurrentWindow->pos.y + m_CurrentWindow->size.y;
            m_GlobalScroll.lastWindowBottom = glm::max(
                m_GlobalScroll.lastWindowBottom,
                windowBottom
            );
        }

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
        cmd.color = Unicorn::UI::Color::Border;
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
        glm::vec4 color = Unicorn::UI::Color::ButtonNormal;
        if (state.active) color = Unicorn::UI::Color::ButtonActive;
        else if (state.hovered) color = Unicorn::UI::Color::ButtonHover;

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
        textCmd.color = Unicorn::UI::Color::Black;
        textCmd.text = label;
        AddDrawCommand(textCmd);

        layout.Advance(buttonSize);

        return state.clicked;
    }

    void UIContext::Text(const std::string& text) {
        TextColored(Unicorn::UI::Color::Text, text);
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
        cmd.color = Unicorn::UI::Color::Text;
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
        cmd.color = Unicorn::UI::Color::Text;
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
        boxCmd.color = state.hovered ? Unicorn::UI::Color::ButtonHover : Unicorn::UI::Color::ButtonNormal;
        boxCmd.rounding = 2.0f;
        AddDrawCommand(boxCmd);

        if (value && *value) {
            DrawCommand checkCmd;
            checkCmd.type = DrawCommand::Type::Rect;
            checkCmd.pos = pos + glm::vec2(5, 5);
            checkCmd.size = glm::vec2(10, 10);
            checkCmd.color = Unicorn::UI::Color::White;
            AddDrawCommand(checkCmd);
        }

        DrawCommand textCmd;
        textCmd.type = DrawCommand::Type::Text;
        textCmd.pos = pos + glm::vec2(25, 2);
        textCmd.color = Unicorn::UI::Color::Text;
        textCmd.text = label;
        AddDrawCommand(textCmd);

        glm::vec2 totalSize(boxSize.x + 25 + CalcTextSize(label).x, boxSize.y);
        layout.Advance(totalSize);

        return state.clicked;
    }

    std::string UIContext::GetScrollRegionUnderMouse() const {
        for (const auto& [id, region] : m_ScrollRegions) {
            if (IsPointInRect(m_MousePos, region.pos, region.size)) {
                return id;
            }
        }
        return "";
    }

    bool UIContext::InputText(const std::string& label, std::string& buffer, size_t maxLength) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 inputSize(300, 30);
        glm::vec2 pos = layout.cursor;

        // Show label if not hidden
        if (!label.empty() && label[0] != '#') {
            Text(label);
            pos = layout.cursor;
        }

        std::string id = GenerateID(label);
        WidgetState state = ProcessWidget(pos, inputSize);

        // Initialize or update text input state
        bool isActive = (m_TextInput.id == id);

        if (state.clicked) {
            m_TextInput.id = id;
            m_TextInput.buffer = &buffer;

            float clickX = m_MousePos.x - (pos.x + 10.0f);
            m_TextInput.cursorPos = GetCursorPositionFromX(buffer, clickX);
            m_TextInput.ClearSelection();
            m_TextInput.isDragging = false;
            isActive = true;
            m_IsDirty = true;
        }

        // Deactivate if clicked outside
        if (m_MouseButtons[0] && !state.hovered && isActive) {
            m_TextInput.id.clear();
            m_TextInput.buffer = nullptr;
            m_TextInput.isDragging = false;
            isActive = false;
            m_IsDirty = true;
        }

        if (isActive) {
            if (state.hovered && m_MouseButtons[0]) {
                if (!m_TextInput.isDragging) {
                    // Start dragging
                    m_TextInput.isDragging = true;
                    float clickX = m_MousePos.x - (pos.x + 10.0f);
                    m_TextInput.selectionStart = GetCursorPositionFromX(buffer, clickX);
                    m_TextInput.cursorPos = m_TextInput.selectionStart;
                    m_TextInput.hasSelection = false;
                    m_IsDirty = true;
                }
                else {
                    // Update selection during drag
                    float currentX = m_MousePos.x - (pos.x + 10.0f);
                    size_t newCursorPos = GetCursorPositionFromX(buffer, currentX);

                    if (newCursorPos != m_TextInput.cursorPos) {
                        m_TextInput.cursorPos = newCursorPos;
                        m_TextInput.selectionEnd = newCursorPos;
                        m_TextInput.hasSelection = (m_TextInput.selectionStart != m_TextInput.selectionEnd);
                        m_IsDirty = true;
                    }
                }
            }

            // End dragging when mouse released
            if (!m_MouseButtons[0] && m_TextInput.isDragging) {
                m_TextInput.isDragging = false;
                m_IsDirty = true;
            }
        }

        // Handle keyboard input when active
        if (isActive) {
            bool shift = Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT) ||
                Input::IsKeyPressed(GLFW_KEY_RIGHT_SHIFT);
            bool ctrl = Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) ||
                Input::IsKeyPressed(GLFW_KEY_RIGHT_CONTROL);

            // === IMPROVED WORD BOUNDARY DETECTION (All Languages) ===
            auto IsWordBoundary = [&](size_t pos, bool forward) -> bool {
                if (forward) {
                    if (pos >= buffer.length()) return true;

                    unsigned char c = buffer[pos];

                    // Space or punctuation
                    if (c == ' ' || c == '.' || c == ',' || c == '!' || c == '?' || c == '=' || c == '"' || c == '-') return true;

                    // Check for script changes (UTF-8 multi-byte)
                    if (c >= 0x80) {
                        // At boundary if next char is different script or space
                        if (pos + 1 < buffer.length()) {
                            unsigned char next = buffer[pos + 1];
                            if (next < 0x80 || next == ' ') return true;

                            // Arabic to Latin boundary
                            if ((c >= 0xD8 || c == 0xD9) && next < 0xC0) return true;
                        }
                    }

                    return false;
                }
                else {
                    if (pos == 0) return true;

                    unsigned char c = buffer[pos - 1];

                    if (c == ' ' || c == '.' || c == ',' || c == '!' || c == '?' || c == '=' || c == '"' || c == '-') return true;

                    if (c >= 0x80) {
                        if (pos > 1) {
                            unsigned char prev = buffer[pos - 2];
                            if (prev < 0x80 || prev == ' ') return true;
                        }
                    }

                    return false;
                }
                };

            // === CURSOR MOVEMENT ===

            // Left arrow
            if (IsKeyPressedWithRepeat(GLFW_KEY_LEFT)) {
                if (m_TextInput.hasSelection && !shift && !ctrl) {
                    m_TextInput.cursorPos = glm::min(m_TextInput.selectionStart, m_TextInput.selectionEnd);
                    m_TextInput.ClearSelection();
                }
                else if (m_TextInput.cursorPos > 0) {
                    size_t oldPos = m_TextInput.cursorPos;

                    if (ctrl) {
                        // Word jump - handle UTF-8
                        do {
                            m_TextInput.cursorPos--;
                            // Skip continuation bytes
                            while (m_TextInput.cursorPos > 0 &&
                                (buffer[m_TextInput.cursorPos] & 0xC0) == 0x80) {
                                m_TextInput.cursorPos--;
                            }
                        } while (m_TextInput.cursorPos > 0 &&
                            buffer[m_TextInput.cursorPos - 1] == ' ');

                        while (m_TextInput.cursorPos > 0 &&
                            !IsWordBoundary(m_TextInput.cursorPos, false)) {
                            m_TextInput.cursorPos--;
                            while (m_TextInput.cursorPos > 0 &&
                                (buffer[m_TextInput.cursorPos] & 0xC0) == 0x80) {
                                m_TextInput.cursorPos--;
                            }
                        }
                    }
                    else {
                        // Move one character (handle UTF-8)
                        m_TextInput.cursorPos--;
                        while (m_TextInput.cursorPos > 0 &&
                            (buffer[m_TextInput.cursorPos] & 0xC0) == 0x80) {
                            m_TextInput.cursorPos--;
                        }
                    }

                    if (shift) {
                        if (!m_TextInput.hasSelection) {
                            m_TextInput.selectionStart = oldPos;
                            m_TextInput.hasSelection = true;
                        }
                        m_TextInput.selectionEnd = m_TextInput.cursorPos;
                    }
                    else {
                        m_TextInput.ClearSelection();
                    }
                }
                m_IsDirty = true;
            }

            // Right arrow
            if (IsKeyPressedWithRepeat(GLFW_KEY_RIGHT)) {
                if (m_TextInput.hasSelection && !shift && !ctrl) {
                    m_TextInput.cursorPos = glm::max(m_TextInput.selectionStart, m_TextInput.selectionEnd);
                    m_TextInput.ClearSelection();
                }
                else if (m_TextInput.cursorPos < buffer.length()) {
                    size_t oldPos = m_TextInput.cursorPos;

                    if (ctrl) {
                        // Word jump - handle UTF-8
                        do {
                            m_TextInput.cursorPos++;
                            while (m_TextInput.cursorPos < buffer.length() &&
                                (buffer[m_TextInput.cursorPos] & 0xC0) == 0x80) {
                                m_TextInput.cursorPos++;
                            }
                        } while (m_TextInput.cursorPos < buffer.length() &&
                            buffer[m_TextInput.cursorPos - 1] == ' ');

                        while (m_TextInput.cursorPos < buffer.length() &&
                            !IsWordBoundary(m_TextInput.cursorPos, true)) {
                            m_TextInput.cursorPos++;
                            while (m_TextInput.cursorPos < buffer.length() &&
                                (buffer[m_TextInput.cursorPos] & 0xC0) == 0x80) {
                                m_TextInput.cursorPos++;
                            }
                        }
                    }
                    else {
                        // Move one character (handle UTF-8)
                        m_TextInput.cursorPos++;
                        while (m_TextInput.cursorPos < buffer.length() &&
                            (buffer[m_TextInput.cursorPos] & 0xC0) == 0x80) {
                            m_TextInput.cursorPos++;
                        }
                    }

                    if (shift) {
                        if (!m_TextInput.hasSelection) {
                            m_TextInput.selectionStart = oldPos;
                            m_TextInput.hasSelection = true;
                        }
                        m_TextInput.selectionEnd = m_TextInput.cursorPos;
                    }
                    else {
                        m_TextInput.ClearSelection();
                    }
                }
                m_IsDirty = true;
            }

            // Home key
            if (IsKeyPressedWithRepeat(GLFW_KEY_HOME)) {
                size_t oldPos = m_TextInput.cursorPos;
                m_TextInput.cursorPos = 0;

                if (shift) {
                    if (!m_TextInput.hasSelection) {
                        m_TextInput.selectionStart = oldPos;
                        m_TextInput.hasSelection = true;
                    }
                    m_TextInput.selectionEnd = 0;
                }
                else {
                    m_TextInput.ClearSelection();
                }
                m_IsDirty = true;
            }

            // End key
            if (IsKeyPressedWithRepeat(GLFW_KEY_END)) {
                size_t oldPos = m_TextInput.cursorPos;
                m_TextInput.cursorPos = buffer.length();

                if (shift) {
                    if (!m_TextInput.hasSelection) {
                        m_TextInput.selectionStart = oldPos;
                        m_TextInput.hasSelection = true;
                    }
                    m_TextInput.selectionEnd = buffer.length();
                }
                else {
                    m_TextInput.ClearSelection();
                }
                m_IsDirty = true;
            }

            // === DELETION ===

            // Backspace (UTF-8 aware)
            if (IsKeyPressedWithRepeat(GLFW_KEY_BACKSPACE)) {
                if (m_TextInput.hasSelection) {
                    m_TextInput.DeleteSelection();
                }
                else if (m_TextInput.cursorPos > 0) {
                    if (ctrl) {
                        // Delete word
                        size_t deleteStart = m_TextInput.cursorPos;
                        do {
                            deleteStart--;
                            while (deleteStart > 0 && (buffer[deleteStart] & 0xC0) == 0x80) {
                                deleteStart--;
                            }
                        } while (deleteStart > 0 && buffer[deleteStart - 1] == ' ');

                        while (deleteStart > 0 && !IsWordBoundary(deleteStart, false)) {
                            deleteStart--;
                            while (deleteStart > 0 && (buffer[deleteStart] & 0xC0) == 0x80) {
                                deleteStart--;
                            }
                        }

                        buffer.erase(deleteStart, m_TextInput.cursorPos - deleteStart);
                        m_TextInput.cursorPos = deleteStart;
                    }
                    else {
                        // Delete one character (UTF-8 aware)
                        size_t deletePos = m_TextInput.cursorPos - 1;
                        while (deletePos > 0 && (buffer[deletePos] & 0xC0) == 0x80) {
                            deletePos--;
                        }
                        size_t charLen = m_TextInput.cursorPos - deletePos;
                        buffer.erase(deletePos, charLen);
                        m_TextInput.cursorPos = deletePos;
                    }
                }
                m_IsDirty = true;
            }

            // Delete key (UTF-8 aware)
            if (IsKeyPressedWithRepeat(GLFW_KEY_DELETE)) {
                if (m_TextInput.hasSelection) {
                    m_TextInput.DeleteSelection();
                }
                else if (m_TextInput.cursorPos < buffer.length()) {
                    if (ctrl) {
                        // Delete word forward
                        size_t deleteEnd = m_TextInput.cursorPos;
                        do {
                            deleteEnd++;
                            while (deleteEnd < buffer.length() && (buffer[deleteEnd] & 0xC0) == 0x80) {
                                deleteEnd++;
                            }
                        } while (deleteEnd < buffer.length() && buffer[deleteEnd - 1] == ' ');

                        while (deleteEnd < buffer.length() && !IsWordBoundary(deleteEnd, true)) {
                            deleteEnd++;
                            while (deleteEnd < buffer.length() && (buffer[deleteEnd] & 0xC0) == 0x80) {
                                deleteEnd++;
                            }
                        }

                        buffer.erase(m_TextInput.cursorPos, deleteEnd - m_TextInput.cursorPos);
                    }
                    else {
                        // Delete one character forward (UTF-8 aware)
                        size_t deleteEnd = m_TextInput.cursorPos + 1;
                        while (deleteEnd < buffer.length() && (buffer[deleteEnd] & 0xC0) == 0x80) {
                            deleteEnd++;
                        }
                        buffer.erase(m_TextInput.cursorPos, deleteEnd - m_TextInput.cursorPos);
                    }
                }
                m_IsDirty = true;
            }

            // === CLIPBOARD OPERATIONS ===

            // Ctrl+A: Select all
            if (ctrl && IsKeyPressedWithRepeat(GLFW_KEY_A)) {
                m_TextInput.selectionStart = 0;
                m_TextInput.selectionEnd = buffer.length();
                m_TextInput.hasSelection = buffer.length() > 0;
                m_TextInput.cursorPos = buffer.length();
                m_IsDirty = true;
            }

            // Ctrl+C: Copy
            if (ctrl && IsKeyPressedWithRepeat(GLFW_KEY_C)) {
                if (m_TextInput.hasSelection) {
                    std::string selectedText = m_TextInput.GetSelectedText();
                    glfwSetClipboardString(nullptr, selectedText.c_str());
                }
            }

            // Ctrl+X: Cut
            if (ctrl && IsKeyPressedWithRepeat(GLFW_KEY_X)) {
                if (m_TextInput.hasSelection) {
                    std::string selectedText = m_TextInput.GetSelectedText();
                    glfwSetClipboardString(nullptr, selectedText.c_str());
                    m_TextInput.DeleteSelection();
                    m_IsDirty = true;
                }
            }

            // Ctrl+V: Paste
            if (ctrl && IsKeyPressedWithRepeat(GLFW_KEY_V)) {
                const char* clipboardText = glfwGetClipboardString(nullptr);
                if (clipboardText) {
                    if (m_TextInput.hasSelection) {
                        m_TextInput.DeleteSelection();
                    }

                    std::string pasteText(clipboardText);
                    pasteText.erase(std::remove(pasteText.begin(), pasteText.end(), '\n'), pasteText.end());
                    pasteText.erase(std::remove(pasteText.begin(), pasteText.end(), '\r'), pasteText.end());

                    size_t availableSpace = maxLength - buffer.length();
                    if (pasteText.length() > availableSpace) {
                        pasteText = pasteText.substr(0, availableSpace);
                    }

                    buffer.insert(m_TextInput.cursorPos, pasteText);
                    m_TextInput.cursorPos += pasteText.length();
                    m_IsDirty = true;
                }
            }

            // === TEXT INPUT (Full UTF-8 Support) ===
            unsigned int charInput = Input::GetLastChar();
            if (charInput > 0) {
                if ((charInput >= 32 && charInput < 127) || charInput >= 0x80) {
                    if (buffer.length() < maxLength) {
                        if (m_TextInput.hasSelection) {
                            m_TextInput.DeleteSelection();
                        }

                        // Encode UTF-8
                        if (charInput < 0x80) {
                            buffer.insert(m_TextInput.cursorPos, 1, static_cast<char>(charInput));
                            m_TextInput.cursorPos++;
                        }
                        else if (charInput < 0x800) {
                            char utf8[2];
                            utf8[0] = 0xC0 | (charInput >> 6);
                            utf8[1] = 0x80 | (charInput & 0x3F);
                            buffer.insert(m_TextInput.cursorPos, utf8, 2);
                            m_TextInput.cursorPos += 2;
                        }
                        else if (charInput < 0x10000) {
                            char utf8[3];
                            utf8[0] = 0xE0 | (charInput >> 12);
                            utf8[1] = 0x80 | ((charInput >> 6) & 0x3F);
                            utf8[2] = 0x80 | (charInput & 0x3F);
                            buffer.insert(m_TextInput.cursorPos, utf8, 3);
                            m_TextInput.cursorPos += 3;
                        }
                        else {
                            char utf8[4];
                            utf8[0] = 0xF0 | (charInput >> 18);
                            utf8[1] = 0x80 | ((charInput >> 12) & 0x3F);
                            utf8[2] = 0x80 | ((charInput >> 6) & 0x3F);
                            utf8[3] = 0x80 | (charInput & 0x3F);
                            buffer.insert(m_TextInput.cursorPos, utf8, 4);
                            m_TextInput.cursorPos += 4;
                        }

                        m_IsDirty = true;
                    }
                }
            }

            // === ESCAPE / ENTER ===
            static bool enterHandled = false;
            bool enterPressed = Input::IsKeyPressed(GLFW_KEY_ENTER) ||
                Input::IsKeyPressed(GLFW_KEY_KP_ENTER);

            if (enterPressed && !enterHandled) {
                m_TextInput.id.clear();
                m_TextInput.buffer = nullptr;
                isActive = false;
                m_IsDirty = true;
                enterHandled = true;
            }
            if (!enterPressed) {
                enterHandled = false;
            }

            static bool escapeHandled = false;
            bool escapePressed = Input::IsKeyPressed(GLFW_KEY_ESCAPE);

            if (escapePressed && !escapeHandled) {
                m_TextInput.id.clear();
                m_TextInput.buffer = nullptr;
                isActive = false;
                m_IsDirty = true;
                escapeHandled = true;
            }
            if (!escapePressed) {
                escapeHandled = false;
            }
        }

        // === RENDERING ===
        float borderWidth = 2.0f;
        float rounding = 6.0f;

        glm::vec4 borderColor;
        if (isActive) {
            borderColor = Unicorn::UI::Color::Primary;
        }
        else if (state.hovered) {
            borderColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        }
        else {
            borderColor = Unicorn::UI::Color::Border;
        }

        DrawCommand borderCmd;
        borderCmd.type = DrawCommand::Type::RoundedRect;
        borderCmd.pos = pos;
        borderCmd.size = inputSize;
        borderCmd.color = borderColor;
        borderCmd.rounding = rounding;
        AddDrawCommand(borderCmd);

        DrawCommand bgCmd;
        bgCmd.type = DrawCommand::Type::RoundedRect;
        bgCmd.pos = pos + glm::vec2(borderWidth, borderWidth);
        bgCmd.size = inputSize - glm::vec2(borderWidth * 2, borderWidth * 2);
        bgCmd.color = Unicorn::UI::Color::White;
        bgCmd.rounding = rounding - 1.0f;
        AddDrawCommand(bgCmd);

        float padding = 10.0f;
        glm::vec2 textPos = pos + glm::vec2(padding, 7);

        // Draw selection highlight (BiDi aware)
        if (isActive && m_TextInput.hasSelection) {
            size_t selStart = glm::min(m_TextInput.selectionStart, m_TextInput.selectionEnd);
            size_t selEnd = glm::max(m_TextInput.selectionStart, m_TextInput.selectionEnd);

            // Use shaped text for accurate positioning
            auto shapedGlyphs = m_Renderer->GetFontManager().ShapeText(buffer);

            float beforeWidth = 0.0f;
            float selectionWidth = 0.0f;

            size_t bytePos = 0;
            for (const auto& glyph : shapedGlyphs) {
                if (bytePos < selStart) {
                    beforeWidth += glyph.advance.x;
                }
                else if (bytePos < selEnd) {
                    selectionWidth += glyph.advance.x;
                }

                // Advance byte position (estimate - HarfBuzz handles this)
                bytePos++;
            }

            DrawCommand selectionCmd;
            selectionCmd.type = DrawCommand::Type::RoundedRect;
            selectionCmd.pos = textPos + glm::vec2(beforeWidth, -2);
            selectionCmd.size = glm::vec2(selectionWidth, 20);
            selectionCmd.color = glm::vec4(0.4f, 0.6f, 1.0f, 0.3f);
            selectionCmd.rounding = 3.0f;
            AddDrawCommand(selectionCmd);
        }

        // Draw text
        DrawCommand textCmd;
        textCmd.type = DrawCommand::Type::Text;
        textCmd.pos = textPos;
        textCmd.color = buffer.empty() ? Unicorn::UI::Color::TextDisabled : Unicorn::UI::Color::Text;
        textCmd.text = buffer.empty() ? "Type here..." : buffer;
        AddDrawCommand(textCmd);

        // Draw cursor (BiDi aware)
        if (isActive) {
            static auto lastBlink = std::chrono::high_resolution_clock::now();
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBlink);

            bool showCursor = (elapsed.count() / 530) % 2 == 0;

            if (showCursor && !buffer.empty()) {
                // Use shaped text for cursor position
                std::string textBeforeCursor = buffer.substr(0, m_TextInput.cursorPos);
                auto shapedGlyphs = m_Renderer->GetFontManager().ShapeText(textBeforeCursor);

                float cursorX = 0.0f;
                for (const auto& glyph : shapedGlyphs) {
                    cursorX += glyph.advance.x;
                }

                DrawCommand cursorCmd;
                cursorCmd.type = DrawCommand::Type::Rect;
                cursorCmd.pos = textPos + glm::vec2(cursorX, -1);
                cursorCmd.size = glm::vec2(2, 18);
                cursorCmd.color = Unicorn::UI::Color::Primary;
                AddDrawCommand(cursorCmd);
            }
            else if (showCursor && buffer.empty()) {
                // Cursor at start when empty
                DrawCommand cursorCmd;
                cursorCmd.type = DrawCommand::Type::Rect;
                cursorCmd.pos = textPos + glm::vec2(0, -1);
                cursorCmd.size = glm::vec2(2, 18);
                cursorCmd.color = Unicorn::UI::Color::Primary;
                AddDrawCommand(cursorCmd);
            }

            m_IsDirty = true;
        }

        DrawCommand scissorCmd;
        scissorCmd.type = DrawCommand::Type::PushScissor;
        scissorCmd.pos = pos + glm::vec2(borderWidth, borderWidth);
        scissorCmd.size = inputSize - glm::vec2(borderWidth * 2, borderWidth * 2);
        AddDrawCommand(scissorCmd);

        layout.Advance(inputSize);
        return state.clicked || isActive;
    }

    size_t UIContext::GetCursorPositionFromX(const std::string& text, float targetX) {
        if (text.empty() || targetX <= 0.0f) {
            return 0;
        }

        // Use the renderer's font manager (m_Renderer exists in UIContext)
        if (!m_Renderer) {
            return 0;
        }

        // Use shaped text for accurate positioning
        auto shapedGlyphs = m_Renderer->GetFontManager().ShapeText(text);

        float currentX = 0.0f;
        size_t bytePos = 0;

        for (size_t i = 0; i < shapedGlyphs.size(); i++) {
            float glyphWidth = shapedGlyphs[i].advance.x;
            float glyphMidpoint = currentX + (glyphWidth * 0.5f);

            // Click is before this glyph's midpoint
            if (targetX < glyphMidpoint) {
                return bytePos;
            }

            currentX += glyphWidth;

            // Advance byte position (UTF-8 aware)
            const char* str = text.c_str() + bytePos;
            unsigned char c = *str;
            if (c < 0x80) bytePos += 1;
            else if ((c & 0xE0) == 0xC0) bytePos += 2;
            else if ((c & 0xF0) == 0xE0) bytePos += 3;
            else if ((c & 0xF8) == 0xF0) bytePos += 4;
            else bytePos += 1;
        }

        return text.length();
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
        bgCmd.color = Unicorn::UI::Color::ButtonNormal;
        bgCmd.rounding = 10.0f;
        AddDrawCommand(bgCmd);

        if (value) {
            float fillWidth = (((*value) - min) / (max - min)) * sliderSize.x;
            DrawCommand fillCmd;
            fillCmd.type = DrawCommand::Type::RoundedRect;
            fillCmd.pos = pos;
            fillCmd.size = glm::vec2(fillWidth, sliderSize.y);
            fillCmd.color = Unicorn::UI::Color::Primary;
            fillCmd.rounding = 10.0f;
            AddDrawCommand(fillCmd);
        }

        if (value) {
            char valueText[32];
            snprintf(valueText, sizeof(valueText), "%.2f", *value);
            DrawCommand textCmd;
            textCmd.type = DrawCommand::Type::Text;
            textCmd.pos = pos + glm::vec2(sliderSize.x + 10, 2);
            textCmd.color = Unicorn::UI::Color::Text;
            textCmd.text = valueText;
            AddDrawCommand(textCmd);
        }

        layout.Advance(sliderSize);

        return state.active;
    }

    void UIContext::EndGlobalScroll() {
        if (!m_GlobalScroll.active) return;

        m_GlobalScroll.contentHeight = m_GlobalScroll.lastWindowBottom + 50.0f;

        m_GlobalScroll.maxScroll = glm::max(
            0.0f,
            m_GlobalScroll.contentHeight - m_GlobalScroll.viewportSize.y
        );

        m_GlobalScroll.physics.target.y = glm::clamp(
            m_GlobalScroll.physics.target.y,
            -m_GlobalScroll.maxScroll,
            0.0f
        );

        m_GlobalScroll.physics.offset.y = glm::clamp(
            m_GlobalScroll.physics.offset.y,
            -m_GlobalScroll.maxScroll,
            0.0f
        );

        m_PageScrollOffsets[m_GlobalScroll.pageId] = m_GlobalScroll.physics.offset;

        bool mouseInViewport = IsPointInRect(m_MousePos, m_GlobalScroll.viewportPos, m_GlobalScroll.viewportSize);

        bool mouseInAnyPanel = false;
        for (const auto& [id, region] : m_ScrollRegions) {
            if (IsPointInRect(m_MousePos, region.pos, region.size)) {
                mouseInAnyPanel = true;
                break;
            }
        }

        bool mouseOverScrollbar = false;
        if (m_GlobalScroll.maxScroll > 1.0f) {
            float scrollbarX = m_GlobalScroll.viewportPos.x + m_GlobalScroll.viewportSize.x - 12.0f;
            glm::vec2 scrollbarPos = glm::vec2(scrollbarX, m_GlobalScroll.viewportPos.y);
            glm::vec2 scrollbarSize = glm::vec2(12, m_GlobalScroll.viewportSize.y);
            mouseOverScrollbar = IsPointInRect(m_MousePos, scrollbarPos, scrollbarSize);
        }

        if (mouseInViewport && !mouseInAnyPanel && !mouseOverScrollbar && m_MouseWheelDelta != 0.0f) {
            float scrollAmount = m_MouseWheelDelta * 200.0f;
            m_GlobalScroll.physics.target.y += scrollAmount;

            m_GlobalScroll.physics.target.y = glm::clamp(
                m_GlobalScroll.physics.target.y,
                -m_GlobalScroll.maxScroll,
                0.0f
            );

            m_GlobalScroll.physics.velocity.y += scrollAmount * 4.5f;
            m_GlobalScroll.physics.hasInertia = true;

            m_IsDirty = true;
        }

        if (m_GlobalScroll.maxScroll > 1.0f) {
            float scrollbarX = m_GlobalScroll.viewportPos.x + m_GlobalScroll.viewportSize.x - 12.0f;
            float scrollbarHeight = m_GlobalScroll.viewportSize.y;
            float visibleRatio = glm::clamp(
                m_GlobalScroll.viewportSize.y / m_GlobalScroll.contentHeight,
                0.0f, 1.0f
            );
            float thumbHeight = glm::max(30.0f, scrollbarHeight * visibleRatio);

            float scrollRatio = 0.0f;
            if (m_GlobalScroll.maxScroll > 0.0f) {
                scrollRatio = glm::clamp(
                    -m_GlobalScroll.physics.offset.y / m_GlobalScroll.maxScroll,
                    0.0f, 1.0f
                );
            }

            float thumbY = m_GlobalScroll.viewportPos.y + scrollRatio * (scrollbarHeight - thumbHeight);

            glm::vec2 thumbPos = glm::vec2(scrollbarX, thumbY);
            glm::vec2 thumbSize = glm::vec2(8, thumbHeight);

            bool mouseOverThumb = IsPointInRect(m_MousePos, thumbPos, thumbSize);

            static bool isDraggingGlobalScrollbar = false;
            static float dragStartY = 0.0f;
            static float dragStartScrollRatio = 0.0f;

            if (mouseOverThumb && m_MouseButtons[0] && !isDraggingGlobalScrollbar) {
                isDraggingGlobalScrollbar = true;
                dragStartY = m_MousePos.y;
                dragStartScrollRatio = scrollRatio;
                m_IsDirty = true;
            }

            if (isDraggingGlobalScrollbar && m_MouseButtons[0]) {
                float deltaY = m_MousePos.y - dragStartY;
                float deltaRatio = deltaY / (scrollbarHeight - thumbHeight);
                float newScrollRatio = glm::clamp(dragStartScrollRatio + deltaRatio, 0.0f, 1.0f);

                m_GlobalScroll.physics.target.y = -newScrollRatio * m_GlobalScroll.maxScroll;
                m_GlobalScroll.physics.offset.y = m_GlobalScroll.physics.target.y;
                m_GlobalScroll.physics.velocity = glm::vec2(0);
                m_GlobalScroll.physics.hasInertia = false;

                m_IsDirty = true;
            }

            if (!m_MouseButtons[0] && isDraggingGlobalScrollbar) {
                isDraggingGlobalScrollbar = false;
                m_IsDirty = true;
            }

            glm::vec2 trackPos = glm::vec2(scrollbarX, m_GlobalScroll.viewportPos.y);
            glm::vec2 trackSize = glm::vec2(8, scrollbarHeight);
            bool mouseOverTrack = IsPointInRect(m_MousePos, trackPos, trackSize);

            if (mouseOverTrack && !mouseOverThumb && m_MouseButtons[0] && !isDraggingGlobalScrollbar) {
                float clickY = m_MousePos.y - m_GlobalScroll.viewportPos.y;
                float targetRatio = glm::clamp(clickY / scrollbarHeight, 0.0f, 1.0f);

                m_GlobalScroll.physics.target.y = -targetRatio * m_GlobalScroll.maxScroll;

                float distance = m_GlobalScroll.physics.target.y - m_GlobalScroll.physics.offset.y;
                m_GlobalScroll.physics.velocity.y = distance * 8.0f;
                m_GlobalScroll.physics.hasInertia = true;

                m_IsDirty = true;
            }

            DrawCommand trackCmd;
            trackCmd.type = DrawCommand::Type::RoundedRect;
            trackCmd.pos = glm::vec2(scrollbarX, m_GlobalScroll.viewportPos.y);
            trackCmd.size = glm::vec2(8, scrollbarHeight);
            trackCmd.color = glm::vec4(0.2f, 0.2f, 0.2f, 0.3f);
            trackCmd.rounding = 4.0f;
            AddDrawCommand(trackCmd);

            glm::vec4 thumbColor = glm::vec4(0.6f, 0.6f, 0.6f, 0.8f);
            if (mouseOverThumb || isDraggingGlobalScrollbar) {
                thumbColor = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
            }

            DrawCommand thumbCmd;
            thumbCmd.type = DrawCommand::Type::RoundedRect;
            thumbCmd.pos = glm::vec2(scrollbarX, thumbY);
            thumbCmd.size = glm::vec2(8, thumbHeight);
            thumbCmd.color = thumbColor;
            thumbCmd.rounding = 4.0f;
            AddDrawCommand(thumbCmd);
        }

        m_GlobalScroll.active = false;
    }


    void UIContext::Panel(const glm::vec2& size, const std::function<void()>& content) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 pos = layout.cursor;

        DrawCommand cmd;
        cmd.type = DrawCommand::Type::RoundedRect;
        cmd.pos = pos;
        cmd.size = size;
        cmd.color = Unicorn::UI::Color::Panel;
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

        // === CALCULATE EFFECTIVE MOUSE POSITION ===
        glm::vec2 effectiveMousePos = m_MousePos;

        // First: Apply panel scroll offset if inside a scrollable panel
        if (!m_ActiveScrollRegionID.empty()) {
            auto it = m_ScrollRegions.find(m_ActiveScrollRegionID);
            if (it != m_ScrollRegions.end()) {
                effectiveMousePos = m_MousePos - it->second.physics.offset;
            }
        }
        // Note: Global scroll is handled by offsetting widget positions, 
        // not mouse position, so we don't subtract it here

        // === CHECK IF MOUSE IS OVER WIDGET ===
        state.hovered = effectiveMousePos.x >= pos.x && effectiveMousePos.x <= pos.x + size.x &&
            effectiveMousePos.y >= pos.y && effectiveMousePos.y <= pos.y + size.y;

        // === VALIDATE HOVER WITHIN PANEL BOUNDS ===
        if (!m_ActiveScrollRegionID.empty()) {
            auto it = m_ScrollRegions.find(m_ActiveScrollRegionID);
            if (it != m_ScrollRegions.end()) {
                const auto& region = it->second;
                bool mouseInPanel = IsPointInRect(m_MousePos, region.pos, region.size);
                if (!mouseInPanel) {
                    state.hovered = false;
                }
            }
        }

        // === TRACK HOVER STATE CHANGES ===
        bool wasHovered = m_LastHoveredWidgets.find(widgetID) != m_LastHoveredWidgets.end();

        if (state.hovered) {
            m_LastHoveredWidgets.insert(widgetID);
        }
        else {
            m_LastHoveredWidgets.erase(widgetID);
        }

        if (state.hovered != wasHovered) {
            m_IsDirty = true;
        }

        // === HANDLE CLICK/ACTIVE STATES ===
        if (state.hovered && m_MouseButtons[0]) {
            state.active = true;
            m_WidgetPressStates[widgetID] = true;
            m_IsDirty = true;
        }

        if (state.hovered && !m_MouseButtons[0] && m_WidgetPressStates[widgetID]) {
            state.clicked = true;
            m_WidgetPressStates[widgetID] = false;
            m_IsDirty = true;
        }

        if (!m_MouseButtons[0]) {
            m_WidgetPressStates[widgetID] = false;
        }

        return state;
    }

    bool UIContext::HasActiveAnimations() const {
        if (m_AnimController && m_AnimController->HasActiveAnimations()) {
            return true;
        }

        // Check panels
        for (const auto& [id, region] : m_ScrollRegions) {
            float velLength = glm::length(region.physics.velocity);
            float dist = glm::distance(region.physics.offset, region.physics.target);

            if (velLength > 0.1f || dist > 0.1f) {
                return true;
            }
        }

        // Check global
        if (m_GlobalScroll.active) {
            float velLength = glm::length(m_GlobalScroll.physics.velocity);
            float dist = glm::distance(m_GlobalScroll.physics.offset, m_GlobalScroll.physics.target);

            if (velLength > 0.1f || dist > 0.1f) {
                return true;
            }
        }

        return false;
    }

    void UIContext::AddDrawCommand(const DrawCommand& cmd) {
        DrawCommand modifiedCmd = cmd;

        if (m_GlobalScroll.active) {
            switch (cmd.type) {
            case DrawCommand::Type::Rect:
            case DrawCommand::Type::RoundedRect:
            case DrawCommand::Type::Text:
            case DrawCommand::Type::Line:
            case DrawCommand::Type::Icon:
                modifiedCmd.pos.y += m_GlobalScroll.physics.offset.y;
                break;
            default:
                break;
            }
        }

        m_DrawCommands.push_back(modifiedCmd);
    }

    bool UIContext::IsKeyPressedWithRepeat(int key) {
        bool isPressed = Input::IsKeyPressed(key);
        double currentTime = glfwGetTime();

        auto& keyState = m_KeyStates[key];

        if (isPressed) {
            if (keyState.key != key) {
                // First press
                keyState.key = key;
                keyState.lastPressTime = currentTime;
                keyState.lastRepeatTime = currentTime;
                keyState.isRepeating = false;
                return true;
            }

            // Check for repeat
            if (!keyState.isRepeating) {
                if (currentTime - keyState.lastPressTime >= keyState.initialDelay) {
                    keyState.isRepeating = true;
                    keyState.lastRepeatTime = currentTime;
                    return true;
                }
            }
            else {
                if (currentTime - keyState.lastRepeatTime >= keyState.repeatInterval) {
                    keyState.lastRepeatTime = currentTime;
                    return true;
                }
            }

            return false;
        }
        else {
            // Key released
            keyState.key = -1;
            keyState.isRepeating = false;
            return false;
        }
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


    void UIContext::UpdatePhysicsScroll(float dt) {
        dt = glm::clamp(dt, 0.001f, 1.00f); // Cap at 30 FPS for stability

        for (auto& [id, region] : m_ScrollRegions) {
            UpdateScrollPhysics(region.physics, dt);
        }

        if (m_GlobalScroll.active) {
            UpdateScrollPhysics(m_GlobalScroll.physics, dt);
        }
    }

    void UIContext::UpdateScrollPhysics(ScrollPhysics& physics, float dt) {
        // ========================================
        // ULTRA SMOOTH SCROLL - Brave Browser Style
        // ========================================

        glm::vec2 displacement = physics.target - physics.offset;
        float distance = glm::length(displacement);

        // Stop threshold - very tight for instant stop
        if (distance < 0.1f && glm::length(physics.velocity) < 0.1f) {
            physics.offset = physics.target;
            physics.velocity = glm::vec2(0);
            physics.hasInertia = false;
            return;
        }

        // Spring physics with high responsiveness
        glm::vec2 springForce = displacement * physics.springStiffness;
        glm::vec2 dampingForce = -physics.velocity * physics.springDamping;
        glm::vec2 totalForce = springForce + dampingForce;

        physics.velocity += totalForce * dt;

        // Friction only during inertia
        if (physics.hasInertia) {
            physics.velocity *= physics.friction;

            // Stop inertia when too slow
            if (glm::length(physics.velocity) < 1.0f) {
                physics.hasInertia = false;
            }
        }

        physics.offset += physics.velocity * dt;

        m_IsDirty = true;
    }

    void UIContext::BeginScrollablePanel(const std::string& id, const glm::vec2& size,
        BorderStyle borderStyle) {
        auto& layout = m_LayoutStack.back();
        glm::vec2 pos = layout.cursor;

        // Initialize scroll region if needed
        if (m_ScrollRegions.find(id) == m_ScrollRegions.end()) {
            ScrollableRegion region;
            region.id = id;
            region.pos = pos;
            region.size = size;
            region.contentSize = glm::vec2(0, 0);

            // Panel-specific physics
            region.physics.friction = 0.90f;
            region.physics.springStiffness = 400.0f;
            region.physics.springDamping = 28.0f;
            region.physics.minVelocity = 0.1f;

            m_ScrollRegions[id] = region;
        }

        auto& region = m_ScrollRegions[id];
        region.pos = pos;
        region.size = size;
        m_ActiveScrollRegionID = id;

        float borderWidth = 1.0f;
        float rounding = 12.0f;

        // Draw border
        DrawBorder(m_DrawCommands, pos, size, borderStyle, borderWidth, Unicorn::UI::Color::Border, rounding);

        // Draw background
        DrawCommand bgCmd;
        bgCmd.type = DrawCommand::Type::RoundedRect;
        bgCmd.pos = pos + glm::vec2(borderWidth, borderWidth);
        bgCmd.size = size - glm::vec2(borderWidth * 2, borderWidth * 2);
        bgCmd.color = Unicorn::UI::Color::White;
        bgCmd.rounding = rounding - borderWidth;
        AddDrawCommand(bgCmd);

        // Push scissor for clipping
        DrawCommand scissorCmd;
        scissorCmd.type = DrawCommand::Type::PushScissor;
        scissorCmd.pos = pos + glm::vec2(borderWidth, borderWidth);
        scissorCmd.size = size - glm::vec2(borderWidth * 2, borderWidth * 2);
        AddDrawCommand(scissorCmd);

        // Check if mouse is inside panel
        bool mouseInPanel = IsPointInRect(m_MousePos, pos, size);

        // Handle mouse wheel scrolling
        if (mouseInPanel && m_MouseWheelDelta != 0.0f) {
            float scrollAmount = m_MouseWheelDelta * 180.0f;
            region.physics.target.y += scrollAmount;

            float maxScrollY = glm::max(0.0f, region.contentSize.y - size.y + 20.0f);
            region.physics.target.y = glm::clamp(region.physics.target.y, -maxScrollY, 0.0f);

            // Add velocity
            region.physics.velocity.y += scrollAmount * 4.0f;
            region.physics.hasInertia = true;

            m_IsDirty = true;
        }

        // Create scrolled layout context
        LayoutContext scrolledLayout;
        scrolledLayout.cursor = pos + glm::vec2(10, 10) + region.physics.offset;
        scrolledLayout.direction = LayoutContext::Direction::Vertical;
        m_LayoutStack.push_back(scrolledLayout);
    }

    void UIContext::BeginGlobalScroll(const glm::vec2& pos, const glm::vec2& size) {
        m_GlobalScroll.active = true;
        m_GlobalScroll.viewportPos = pos;
        m_GlobalScroll.viewportSize = size;
        m_GlobalScroll.lastWindowBottom = 0.0f;

        m_GlobalScroll.pageId = "page_" + std::to_string((int)pos.x) + "_" + std::to_string((int)pos.y);

        if (m_PageScrollOffsets.find(m_GlobalScroll.pageId) != m_PageScrollOffsets.end()) {
            m_GlobalScroll.physics.offset = m_PageScrollOffsets[m_GlobalScroll.pageId];
            m_GlobalScroll.physics.target = m_PageScrollOffsets[m_GlobalScroll.pageId];
        }
        else {
            m_GlobalScroll.physics.offset = glm::vec2(0, 0);
            m_GlobalScroll.physics.target = glm::vec2(0, 0);
        }

        m_GlobalScroll.physics.friction = 0.88f;
        m_GlobalScroll.physics.springStiffness = 320.0f;
        m_GlobalScroll.physics.springDamping = 24.0f;
        m_GlobalScroll.physics.minVelocity = 0.1f;
        m_GlobalScroll.physics.velocity = glm::vec2(0);
        m_GlobalScroll.physics.hasInertia = false;
    }


    void UIContext::EndScrollablePanel() {
        if (m_ActiveScrollRegionID.empty()) return;

        auto& region = m_ScrollRegions[m_ActiveScrollRegionID];

        // Calculate content size
        if (!m_LayoutStack.empty()) {
            auto& scrolledLayout = m_LayoutStack.back();
            region.contentSize = scrolledLayout.contentSize;

            if (region.contentSize.y < 10.0f) {
                glm::vec2 startPos = region.pos + glm::vec2(10, 10);
                glm::vec2 endPos = scrolledLayout.cursor;
                region.contentSize.y = endPos.y - startPos.y + 20.0f;
            }

            m_LayoutStack.pop_back();
        }

        // Pop scissor
        DrawCommand scissorCmd;
        scissorCmd.type = DrawCommand::Type::PopScissor;
        AddDrawCommand(scissorCmd);

        float contentHeight = region.contentSize.y;
        float panelHeight = region.size.y - 20.0f;

        // Draw scrollbar if needed
        if (contentHeight > panelHeight) {
            float scrollbarHeight = region.size.y;
            float thumbHeight = glm::max(30.0f, (panelHeight / contentHeight) * scrollbarHeight);
            float maxScroll = contentHeight - panelHeight;

            // Clamp scroll offset
            if (region.physics.target.y < -maxScroll) {
                region.physics.target.y = -maxScroll;
            }
            if (region.physics.target.y > 0.0f) {
                region.physics.target.y = 0.0f;
            }

            float scrollRatio = (maxScroll > 0) ? glm::clamp(-region.physics.offset.y / maxScroll, 0.0f, 1.0f) : 0.0f;
            float thumbY = region.pos.y + scrollRatio * (scrollbarHeight - thumbHeight);

            // Track
            DrawCommand trackCmd;
            trackCmd.type = DrawCommand::Type::RoundedRect;
            trackCmd.pos = glm::vec2(region.pos.x + region.size.x - 12, region.pos.y);
            trackCmd.size = glm::vec2(8, scrollbarHeight);
            trackCmd.color = glm::vec4(0.2f, 0.2f, 0.2f, 0.3f);
            trackCmd.rounding = 4.0f;
            AddDrawCommand(trackCmd);

            // Thumb
            DrawCommand thumbCmd;
            thumbCmd.type = DrawCommand::Type::RoundedRect;
            thumbCmd.pos = glm::vec2(region.pos.x + region.size.x - 12, thumbY);
            thumbCmd.size = glm::vec2(8, thumbHeight);
            thumbCmd.color = Unicorn::UI::Color::Primary;
            thumbCmd.rounding = 4.0f;
            AddDrawCommand(thumbCmd);
        }

        // Advance parent layout
        if (!m_LayoutStack.empty()) {
            m_LayoutStack.back().Advance(region.size);
        }

        m_ActiveScrollRegionID.clear();
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
            bgCmd.color = state.hovered ? Unicorn::UI::Color::ButtonHover : Unicorn::UI::Color::ButtonNormal;
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

        glm::vec4 baseColor = Unicorn::UI::Color::Transparent;
        glm::vec4 hoverColor = Unicorn::UI::Color::ButtonHover;
        glm::vec4 activeColor = Unicorn::UI::Color::ButtonActive;

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

                iconCmd.color = Unicorn::UI::Color::Black;


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
            bgCmd.color = state.hovered ? Unicorn::UI::Color::ButtonHover : Unicorn::UI::Color::ButtonNormal;
            bgCmd.rounding = 6.0f;
            AddDrawCommand(bgCmd);

            if (!label.empty()) {
                glm::vec2 textSize = CalcTextSize(label);
                glm::vec2 textPos = pos + (buttonSize - textSize) * 0.5f;
                DrawCommand textCmd;
                textCmd.type = DrawCommand::Type::Text;
                textCmd.pos = textPos;
                textCmd.color = Unicorn::UI::Color::Black;
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

        glm::vec4 baseColor = Unicorn::UI::Color::Transparent;
        glm::vec4 hoverColor = Unicorn::UI::Color::ButtonHover;
        glm::vec4 activeColor = Unicorn::UI::Color::ButtonActive;

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

                iconCmd.color = Unicorn::UI::Color::Black;


                AddDrawCommand(iconCmd);
            }
        }

        if (!label.empty()) {
            glm::vec2 textSize = CalcTextSize(label);
            glm::vec2 textPos = finalPos + glm::vec2(36.0f, (scaledSize.y - textSize.y) * 0.5f);

            DrawCommand textCmd;
            textCmd.type = DrawCommand::Type::Text;
            textCmd.pos = textPos;
            textCmd.color = Unicorn::UI::Color::Black;
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



    void UIContext::CheckInputChanges() {
        // Quick input check without full UI processing
        glm::vec2 currentMousePos = Input::GetMousePosition();
        bool currentMouseButtons[3] = {
            Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT),
            Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT),
            Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)
        };
        float currentWheelDelta = Input::GetMouseWheelDelta();

        // Check if input changed
        bool inputChanged = (currentMousePos != m_MousePos ||
            currentMouseButtons[0] != m_MouseButtons[0] ||
            currentMouseButtons[1] != m_MouseButtons[1] ||
            currentMouseButtons[2] != m_MouseButtons[2] ||
            currentWheelDelta != 0.0f);

        if (inputChanged) {
            m_IsDirty = true;
        }

        // Update animations
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        if (m_AnimController) {
            m_AnimController->Update(deltaTime);
            if (m_AnimController->HasActiveAnimations()) {
                m_IsDirty = true;
            }
        }
    }

    bool UIContext::IsPointInRect(const glm::vec2& point, const glm::vec2& rectPos,
        const glm::vec2& rectSize) const {
        return point.x >= rectPos.x && point.x <= rectPos.x + rectSize.x &&
            point.y >= rectPos.y && point.y <= rectPos.y + rectSize.y;
    }

} // namespace Unicorn::UI