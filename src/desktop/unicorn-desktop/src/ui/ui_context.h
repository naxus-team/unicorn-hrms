#pragma once

#include "draw_command.h"
#include "icon_manager.h"
#include "ui_animation.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Unicorn::UI {

    // Forward declarations
    class UIRenderer;
    struct FontRenderOptions;

    // UI Colors
    struct Color {
        static const glm::vec4 Transparent;
        static const glm::vec4 White;
        static const glm::vec4 Black;
        static const glm::vec4 Primary;
        static const glm::vec4 Secondary;
        static const glm::vec4 Background;
        static const glm::vec4 Panel;
        static const glm::vec4 Border;
        static const glm::vec4 Text;
        static const glm::vec4 TextDisabled;
        static const glm::vec4 ButtonNormal;
        static const glm::vec4 ButtonHover;
        static const glm::vec4 ButtonActive;
    };

    // Widget state
    struct WidgetState {
        bool hovered = false;
        bool active = false;
        bool clicked = false;
    };

    enum class Alignment {
        Left,
        Center,
        Right
    };

    struct WindowBorderStyle {
        bool enabled = false;
        float thickness = 1.0f;
        glm::vec4 color = Color::Border;
        bool top = true;
        bool right = true;
        bool bottom = true;
        bool left = true;
    };

    // Layout info
    struct LayoutContext {
        glm::vec2 cursor = { 10.0f, 10.0f };
        glm::vec2 contentSize = { 0.0f, 0.0f };
        float spacing = 8.0f;
        float padding = 10.0f;

        enum class Direction { Vertical, Horizontal } direction = Direction::Vertical;

        void Advance(const glm::vec2& size) {
            if (direction == Direction::Vertical) {
                cursor.y += size.y + spacing;
                contentSize.x = glm::max(contentSize.x, size.x);
                contentSize.y = cursor.y;
            }
            else {
                cursor.x += size.x + spacing;
                contentSize.y = glm::max(contentSize.y, size.y);
                contentSize.x = cursor.x;
            }
        }
    };

    struct ScrollableRegion {
        glm::vec2 pos;
        glm::vec2 size;
        glm::vec2 contentSize;
        glm::vec2 scrollOffset;
        bool isDragging = false;
        glm::vec2 dragStartMouse;
        glm::vec2 dragStartScroll;
        std::string id;
    };

    // Main UI Context
    class UIContext {
    public:
        UIContext();
        ~UIContext();

        void Init();
        void Shutdown();
        void BeginFrame();
        void EndFrame();
        void Render();

        // Window resize handling
        void OnWindowResize(uint32_t width, uint32_t height);

        // Window management
        void BeginWindow(const std::string& title,
            const glm::vec2& pos,
            const glm::vec2& size,
            const WindowBorderStyle& borderStyle = WindowBorderStyle());
        void EndWindow();

        // Layout
        void BeginHorizontal();
        void EndHorizontal();
        void Spacing(float pixels = 0.0f);
        void Separator(float line, int weight);
        void NewLine();

        // Widgets
        bool Button(const std::string& label, const glm::vec2& size = glm::vec2(120, 30));
        bool ButtonWithIcon(const std::string& iconName,
            const std::string& label,
            const glm::vec2& size,
            Alignment align = Alignment::Left);

        bool IconButton(const std::string& iconName,
            const glm::vec2& size,
            Alignment align = Alignment::Left);
        void Text(const std::string& text);
        void TextColored(const glm::vec4& color, const std::string& text);
        void TextLTR(const std::string& text);
        void TextRTL(const std::string& text);
        bool Checkbox(const std::string& label, bool* value);
        bool InputText(const std::string& label, std::string& buffer, size_t maxLength = 256);
        bool InputFloat(const std::string& label, float* value, float step = 1.0f);
        bool SliderFloat(const std::string& label, float* value, float min, float max);

        // Panels
        void Panel(const glm::vec2& size, const std::function<void()>& content);

        // Input queries
        bool IsItemHovered() const { return m_LastWidgetState.hovered; }
        bool IsItemActive() const { return m_LastWidgetState.active; }
        bool IsItemClicked() const { return m_LastWidgetState.clicked; }

        glm::vec2 GetMousePos() const { return m_MousePos; }
        bool IsMouseButtonDown(int button) const;
        bool IsKeyPressed(int key) const;

        // Getters
        const std::vector<DrawCommand>& GetDrawCommands() const { return m_DrawCommands; }
        float GetDeltaTime() const { return m_DeltaTime; }
        std::vector<LayoutContext>& GetLayoutStack() { return m_LayoutStack; }


        UIRenderer& GetRenderer() { return *m_Renderer; }
        const UIRenderer& GetRenderer() const { return *m_Renderer; }

        void BeginScrollablePanel(const std::string& id, const glm::vec2& size);
        void EndScrollablePanel();
        void BeginGlobalScroll(const glm::vec2& pos, const glm::vec2& size);
        void EndGlobalScroll();

        IconManager& GetIconManager() { return *m_IconManager; }
        AnimationController& GetAnimController() { return *m_AnimController; }

        void MarkDirty() { m_IsDirty = true; }
        bool IsDirty() const { return m_IsDirty; }
        void ClearDirty() { m_IsDirty = false; }

        // Helper to check if point is in rect
        bool IsPointInRect(const glm::vec2& point, const glm::vec2& rectPos, const glm::vec2& rectSize);

    private:
        // Internal helpers
        std::string GenerateID(const std::string& label);
        WidgetState ProcessWidget(const glm::vec2& pos, const glm::vec2& size);
        void AddDrawCommand(const DrawCommand& cmd);
        glm::vec2 CalcTextSize(const std::string& text);

        std::unique_ptr<IconManager> m_IconManager;
        std::unique_ptr<AnimationController> m_AnimController;

        // State
        std::unordered_map<std::string, bool> m_WidgetPressStates;
        std::unique_ptr<UIRenderer> m_Renderer;
        std::vector<LayoutContext> m_LayoutStack;
        std::vector<DrawCommand> m_DrawCommands;
        std::vector<std::string> m_IDStack;
        std::unordered_map<std::string, ScrollableRegion> m_ScrollRegions;
        std::string m_ActiveScrollRegionID;

        glm::vec2 m_MousePos = { 0, 0 };
        glm::vec2 m_LastMousePos = { 0, 0 };
        bool m_MouseButtons[3] = { false, false, false };
        float m_MouseWheelDelta = 0.0f;

        std::string m_HoveredID;
        std::string m_ActiveID;
        WidgetState m_LastWidgetState;

        float m_DeltaTime = 0.0f;
        int m_FrameCount = 0;

        bool m_IsDirty = true;

        struct GlobalScroll {
            bool active = false;
            glm::vec2 offset = { 0, 0 };
            glm::vec2 viewportPos;
            glm::vec2 viewportSize;
            float contentHeight = 0;
            float maxScroll = 0;
        } m_GlobalScroll;

        // Current window
        struct WindowData {
            std::string title;
            glm::vec2 pos;
            glm::vec2 size;
        };
        WindowData* m_CurrentWindow = nullptr;
    };

} // namespace Unicorn::UI