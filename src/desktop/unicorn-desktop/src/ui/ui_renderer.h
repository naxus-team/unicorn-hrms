#pragma once

#include "font_manager.h"
#include "draw_command.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Unicorn::UI {

    struct UIVertex {
        glm::vec2 position;
        glm::vec4 color;
        glm::vec2 texCoord;
        glm::vec2 rectPos;      // Rectangle position for SDF
        glm::vec2 rectSize;     // Rectangle size for SDF
        float rounding;         // Corner rounding radius
    };

    class UIRenderer {
    public:
        UIRenderer();
        ~UIRenderer();

        void Init(uint32_t windowWidth, uint32_t windowHeight);
        void Shutdown();

        void BeginFrame();
        void EndFrame();
        void RenderDrawCommands(const std::vector<DrawCommand>& commands);

        void SetViewport(uint32_t width, uint32_t height);
        void OnWindowResize(uint32_t width, uint32_t height);

        // Drawing primitives
        void DrawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
        void DrawRoundedRect(const glm::vec2& pos, const glm::vec2& size,
            const glm::vec4& color, float rounding);
        void DrawText(const glm::vec2& pos, const std::string& text, const glm::vec4& color);
        void DrawLine(const glm::vec2& start, const glm::vec2& end,
            const glm::vec4& color, float thickness = 1.0f);

        // Scissor/Clipping support
        void PushScissor(const glm::vec2& pos, const glm::vec2& size);
        void PopScissor();
        void FlushBatch();  // Make public for UIContext

        FontManager& GetFontManager() { return *m_FontManager; }
        const FontManager& GetFontManager() const { return *m_FontManager; }

    private:
        void InitShaders();
        void InitTextShaders();
        void AddQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color,
            const glm::vec2& rectPos, const glm::vec2& rectSize, float rounding);

        static constexpr size_t MaxVertices = 10000;
        static constexpr size_t MaxIndices = 15000;

        std::vector<UIVertex> m_VertexBuffer;
        std::vector<uint32_t> m_IndexBuffer;

        uint32_t m_VAO = 0;
        uint32_t m_VBO = 0;
        uint32_t m_IBO = 0;
        uint32_t m_TextVAO = 0;
        uint32_t m_TextVBO = 0;

        uint32_t m_ShaderProgram = 0;
        uint32_t m_TextShaderProgram = 0;

        glm::mat4 m_Projection;
        uint32_t m_WindowWidth = 0;
        uint32_t m_WindowHeight = 0;

        std::unique_ptr<FontManager> m_FontManager;

        // Scissor stack for clipping
        struct ScissorRect {
            int x, y, width, height;
        };
        std::vector<ScissorRect> m_ScissorStack;
    };

} // namespace Unicorn::UI