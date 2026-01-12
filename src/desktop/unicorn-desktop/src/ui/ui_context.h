#pragma once

#include "draw_command.h"
#include "icon_manager.h"
#include "ui_animation.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>

namespace Unicorn::UI {

    class UIRenderer;
    struct FontRenderOptions;

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
        static const glm::vec4 TextSecondary;
        static const glm::vec4 ButtonNormal;
        static const glm::vec4 ButtonHover;
        static const glm::vec4 ButtonActive;
    };

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

    struct ScrollPhysics {
        glm::vec2 velocity = { 0, 0 };
        glm::vec2 offset = { 0, 0 };
        glm::vec2 target = { 0, 0 };
        float friction = 0.88f;
        float springStiffness = 280.0f;
        float springDamping = 22.0f;
        float minVelocity = 0.5f;
        bool hasInertia = false;
    };

    struct ScrollableRegion {
        std::string id;
        glm::vec2 pos;
        glm::vec2 size;
        glm::vec2 contentSize;
        ScrollPhysics physics;
    };

    class UIContext {
    public:
        UIContext();
        ~UIContext();

        void Init();
        void Shutdown();
        void BeginFrame();
        void EndFrame();
        void Render();

        void OnWindowResize(uint32_t width, uint32_t height);

        void BeginWindow(const std::string& title,
            const glm::vec2& pos,
            const glm::vec2& size,
            const WindowBorderStyle& borderStyle = WindowBorderStyle());
        void EndWindow();

        void BeginHorizontal();
        void EndHorizontal();
        void Spacing(float pixels = 0.0f);
        void Separator(float line, int weight);
        void NewLine();

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

        void Panel(const glm::vec2& size, const std::function<void()>& content);

        bool IsItemHovered() const { return m_LastWidgetState.hovered; }
        bool IsItemActive() const { return m_LastWidgetState.active; }
        bool IsItemClicked() const { return m_LastWidgetState.clicked; }

        glm::vec2 GetMousePos() const { return m_MousePos; }
        bool IsMouseButtonDown(int button) const;
        bool IsKeyPressed(int key) const;
        bool IsKeyPressedWithRepeat(int key);

        const std::vector<DrawCommand>& GetDrawCommands() const { return m_DrawCommands; }
        float GetDeltaTime() const { return m_DeltaTime; }
        std::vector<LayoutContext>& GetLayoutStack() { return m_LayoutStack; }

        UIRenderer& GetRenderer() { return *m_Renderer; }
        const UIRenderer& GetRenderer() const { return *m_Renderer; }

        void BeginScrollablePanel(const std::string& id, const glm::vec2& size,
            BorderStyle borderStyle = BorderStyle::Inset);
        void EndScrollablePanel();
        void BeginGlobalScroll(const glm::vec2& pos, const glm::vec2& size);
        void EndGlobalScroll();

        IconManager& GetIconManager() { return *m_IconManager; }
        AnimationController& GetAnimController() { return *m_AnimController; }

        void CheckInputChanges();

        void MarkDirty() { m_IsDirty = true; }
        bool IsDirty() const { return m_IsDirty; }
        void ClearDirty() { m_IsDirty = false; }

        bool HasActiveAnimations() const;

        void UpdateAnimations(float dt) {
            if (m_AnimController) {
                m_AnimController->Update(dt);
                if (m_AnimController->HasActiveAnimations()) {
                    m_IsDirty = true;
                }
            }
        }

        glm::vec2 GetScrolledMousePos() const {
            if (!m_ActiveScrollRegionID.empty()) {
                auto it = m_ScrollRegions.find(m_ActiveScrollRegionID);
                if (it != m_ScrollRegions.end()) {
                    return m_MousePos - it->second.physics.offset;
                }
            }
            return m_MousePos;
        }

        void SetScrollPhysics(float friction, float stiffness, float damping) {
            m_DefaultPhysics.friction = friction;
            m_DefaultPhysics.springStiffness = stiffness;
            m_DefaultPhysics.springDamping = damping;
        }

        bool IsPointInRect(const glm::vec2& point, const glm::vec2& rectPos,
            const glm::vec2& rectSize) const;
        std::string GetScrollRegionUnderMouse() const;

    private:
        std::string GenerateID(const std::string& label);
        WidgetState ProcessWidget(const glm::vec2& pos, const glm::vec2& size);
        void AddDrawCommand(const DrawCommand& cmd);
        glm::vec2 CalcTextSize(const std::string& text);
        size_t GetCursorPositionFromX(const std::string& text, float targetX);

        void UpdatePhysicsScroll(float dt);
        void UpdateScrollPhysics(ScrollPhysics& physics, float dt);

        int m_CurrentCursor = 0;

        std::unique_ptr<IconManager> m_IconManager;
        std::unique_ptr<AnimationController> m_AnimController;

        std::unordered_map<std::string, bool> m_WidgetPressStates;
        std::unordered_set<std::string> m_LastHoveredWidgets;
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

        std::string m_ActiveInputID;
        std::string* m_ActiveInputBuffer = nullptr;
        bool m_BackspaceHandled = false;

        float m_DeltaTime = 0.0f;
        int m_FrameCount = 0;

        bool m_IsDirty = true;

        struct GlobalScroll {
            bool active = false;
            glm::vec2 viewportPos;
            glm::vec2 viewportSize;
            float contentHeight = 0;
            float maxScroll = 0;
            float lastWindowBottom = 0;
            std::string pageId;  // ADD THIS - to track per-page scroll
            ScrollPhysics physics;
        } m_GlobalScroll;

        std::unordered_map<std::string, glm::vec2> m_PageScrollOffsets;

        ScrollPhysics m_DefaultPhysics;

        struct TextInputState {
            std::string id;
            std::string* buffer = nullptr;
            size_t cursorPos = 0;
            size_t selectionStart = 0;
            size_t selectionEnd = 0;
            bool hasSelection = false;
            float scrollOffset = 0.0f;
            bool isDragging = false;
            glm::vec2 lastMousePos;

            void ClearSelection() {
                hasSelection = false;
                selectionStart = 0;
                selectionEnd = 0;
            }

            void DeleteSelection() {
                if (!hasSelection || !buffer) return;
                size_t start = glm::min(selectionStart, selectionEnd);
                size_t end = glm::max(selectionStart, selectionEnd);
                buffer->erase(start, end - start);
                cursorPos = start;
                ClearSelection();
            }

            std::string GetSelectedText() const {
                if (!hasSelection || !buffer) return "";
                size_t start = glm::min(selectionStart, selectionEnd);
                size_t end = glm::max(selectionStart, selectionEnd);
                return buffer->substr(start, end - start);
            }
        } m_TextInput;

        struct KeyRepeatState {
            int key = -1;
            double lastPressTime = 0.0;
            double lastRepeatTime = 0.0;
            bool isRepeating = false;
            const double initialDelay = 0.5;
            const double repeatInterval = 0.03;
        };

        std::unordered_map<int, KeyRepeatState> m_KeyStates;

        struct WindowData {
            std::string title;
            glm::vec2 pos;
            glm::vec2 size;
        };
        WindowData* m_CurrentWindow = nullptr;
    };

}