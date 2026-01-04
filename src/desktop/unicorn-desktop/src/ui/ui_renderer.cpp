#include "ui_renderer.h"
#include "../core/window.h"
#include <glad/glad.h>
#include <iostream>
#include <cmath>

namespace Unicorn::UI {

    // ============================================
    // Blender-Style Shaders with Per-Vertex Rect Data
    // ============================================

    static const char* roundedRectVertexShader = R"(
        #version 330 core
        layout(location = 0) in vec2 a_Position;
        layout(location = 1) in vec4 a_Color;
        layout(location = 2) in vec2 a_TexCoord;
        layout(location = 3) in vec2 a_RectPos;
        layout(location = 4) in vec2 a_RectSize;
        layout(location = 5) in float a_Rounding;
        
        uniform mat4 u_Projection;
        
        out vec4 v_Color;
        out vec2 v_TexCoord;
        out vec2 v_FragPos;
        out vec2 v_RectPos;
        out vec2 v_RectSize;
        out float v_Rounding;
        
        void main() {
            v_Color = a_Color;
            v_TexCoord = a_TexCoord;
            v_FragPos = a_Position;
            v_RectPos = a_RectPos;
            v_RectSize = a_RectSize;
            v_Rounding = a_Rounding;
            gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
        }
    )";

    static const char* roundedRectFragmentShader = R"(
        #version 330 core
        
        in vec4 v_Color;
        in vec2 v_TexCoord;
        in vec2 v_FragPos;
        in vec2 v_RectPos;
        in vec2 v_RectSize;
        in float v_Rounding;
        
        out vec4 FragColor;
        
        // SDF for rounded rectangle (Blender style)
        float roundedBoxSDF(vec2 centerPos, vec2 size, float radius) {
            return length(max(abs(centerPos) - size + radius, 0.0)) - radius;
        }
        
        void main() {
            if (v_Rounding > 0.5) {
                // Calculate distance from rounded rectangle edge
                vec2 rectCenter = v_RectPos + v_RectSize * 0.5;
                vec2 fragToCenter = v_FragPos - rectCenter;
                vec2 halfSize = v_RectSize * 0.5;
                
                float distance = roundedBoxSDF(fragToCenter, halfSize, v_Rounding);
                
                // Ultra-smooth antialiasing (0.5px for MSAA)
                float smoothEdge = 0.5;
                float alpha = 1.0 - smoothstep(-smoothEdge, smoothEdge, distance);
                
                FragColor = vec4(v_Color.rgb, v_Color.a * alpha);
            } else {
                // No rounding - simple rect
                FragColor = v_Color;
            }
        }
    )";

    // Text shader (unchanged)
    static const char* textVertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec4 vertex;
        
        out vec2 TexCoords;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
            TexCoords = vertex.zw;
        }
    )";

    static const char* textFragmentShaderSource = R"(
        #version 330 core
        
        in vec2 TexCoords;
        out vec4 color;
        
        uniform sampler2D text;
        uniform vec4 textColor;
        uniform int useSubpixel;
        
        void main() {
            if (useSubpixel == 1) {
                vec3 sample = texture(text, TexCoords).rgb;
                color = vec4(textColor.rgb, (sample.r + sample.g + sample.b) / 3.0 * textColor.a);
            } else {
                vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
                color = textColor * sampled;
            }
        }
    )";

    UIRenderer::UIRenderer() {
        m_VertexBuffer.reserve(MaxVertices);
        m_IndexBuffer.reserve(MaxIndices);
        m_FontManager = std::make_unique<FontManager>();
    }

    UIRenderer::~UIRenderer() {
        Shutdown();
    }

    void UIRenderer::Init(uint32_t windowWidth, uint32_t windowHeight) {
        m_WindowWidth = windowWidth;
        m_WindowHeight = windowHeight;

        std::cout << "[UIRenderer] Initializing Blender-style UI..." << std::endl;

        // Enable MSAA (Multisample Anti-Aliasing)
        glEnable(GL_MULTISAMPLE);

        if (!m_FontManager->Init()) {
            std::cerr << "[UIRenderer] Failed to initialize font manager" << std::endl;
        }

        FontRenderOptions fontOptions;
        fontOptions.useKerning = false;          // Disable extra kerning
        fontOptions.useAntialiasing = true;
        fontOptions.aaMode = FontRenderOptions::AntialiasMode::Grayscale;
        fontOptions.letterSpacing = 0.0f;        // No extra letter spacing
        fontOptions.lineHeight = 1.0f;           // Normal line height

#ifdef _WIN32
        m_FontManager->LoadFontWithOptions("default", "./assets/fonts/cairo.ttf", 14, fontOptions);
#else
        m_FontManager->LoadFontWithOptions("default", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16, fontOptions);
#endif

        InitShaders();
        InitTextShaders();

        // Create buffers for shapes with NEW vertex layout
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_IBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, MaxVertices * sizeof(UIVertex), nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, MaxIndices * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);

        // Attribute 0: Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, position));

        // Attribute 1: Color
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, color));

        // Attribute 2: TexCoord
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, texCoord));

        // Attribute 3: RectPos (NEW)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, rectPos));

        // Attribute 4: RectSize (NEW)
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, rectSize));

        // Attribute 5: Rounding (NEW)
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, rounding));

        glBindVertexArray(0);

        // Create buffers for text rendering
        glGenVertexArrays(1, &m_TextVAO);
        glGenBuffers(1, &m_TextVBO);

        glBindVertexArray(m_TextVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_TextVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        SetViewport(windowWidth, windowHeight);

        std::cout << "[UIRenderer] ✓ Blender-style UI initialized with smooth rounded corners" << std::endl;
    }

    void UIRenderer::InitShaders() {
        uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &roundedRectVertexShader, nullptr);
        glCompileShader(vertexShader);

        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cerr << "[UIRenderer] Vertex shader compilation failed: " << infoLog << std::endl;
            return;
        }

        uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &roundedRectFragmentShader, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cerr << "[UIRenderer] Fragment shader compilation failed: " << infoLog << std::endl;
            glDeleteShader(vertexShader);
            return;
        }

        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, vertexShader);
        glAttachShader(m_ShaderProgram, fragmentShader);
        glLinkProgram(m_ShaderProgram);

        glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_ShaderProgram, 512, nullptr, infoLog);
            std::cerr << "[UIRenderer] Shader linking failed: " << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void UIRenderer::InitTextShaders() {
        uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &textVertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cerr << "[UIRenderer] Text vertex shader compilation failed: " << infoLog << std::endl;
            return;
        }

        uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &textFragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cerr << "[UIRenderer] Text fragment shader compilation failed: " << infoLog << std::endl;
            glDeleteShader(vertexShader);
            return;
        }

        m_TextShaderProgram = glCreateProgram();
        glAttachShader(m_TextShaderProgram, vertexShader);
        glAttachShader(m_TextShaderProgram, fragmentShader);
        glLinkProgram(m_TextShaderProgram);

        glGetProgramiv(m_TextShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_TextShaderProgram, 512, nullptr, infoLog);
            std::cerr << "[UIRenderer] Text shader linking failed: " << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void UIRenderer::Shutdown() {
        if (m_FontManager) {
            m_FontManager->Shutdown();
        }

        if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO) glDeleteBuffers(1, &m_VBO);
        if (m_IBO) glDeleteBuffers(1, &m_IBO);
        if (m_TextVAO) glDeleteVertexArrays(1, &m_TextVAO);
        if (m_TextVBO) glDeleteBuffers(1, &m_TextVBO);
        if (m_ShaderProgram) glDeleteProgram(m_ShaderProgram);
        if (m_TextShaderProgram) glDeleteProgram(m_TextShaderProgram);

        m_VAO = m_VBO = m_IBO = m_TextVAO = m_TextVBO = 0;
        m_ShaderProgram = m_TextShaderProgram = 0;

        std::cout << "[UIRenderer] Shutdown" << std::endl;
    }

    void UIRenderer::SetViewport(uint32_t width, uint32_t height) {
        m_WindowWidth = width;
        m_WindowHeight = height;
        m_Projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
        glViewport(0, 0, width, height);
    }

    void UIRenderer::OnWindowResize(uint32_t width, uint32_t height) {
        SetViewport(width, height);
    }

    void UIRenderer::BeginFrame() {
        m_VertexBuffer.clear();
        m_IndexBuffer.clear();
    }

    void UIRenderer::EndFrame() {}

    void UIRenderer::RenderDrawCommands(const std::vector<DrawCommand>& commands) {
        m_VertexBuffer.clear();
        m_IndexBuffer.clear();
        m_ScissorStack.clear();
        glDisable(GL_SCISSOR_TEST);

        for (const auto& cmd : commands) {
            switch (cmd.type) {
            case DrawCommand::Type::PushScissor:
                // Flush current batch before scissor
                if (!m_VertexBuffer.empty()) {
                    FlushBatch();
                    m_VertexBuffer.clear();
                    m_IndexBuffer.clear();
                }
                PushScissor(cmd.pos, cmd.size);
                break;

            case DrawCommand::Type::PopScissor:
                // Flush before popping scissor
                if (!m_VertexBuffer.empty()) {
                    FlushBatch();
                    m_VertexBuffer.clear();
                    m_IndexBuffer.clear();
                }
                PopScissor();
                break;

            case DrawCommand::Type::Rect:
                AddQuad(cmd.pos, cmd.size, cmd.color, cmd.pos, cmd.size, 0.0f);
                break;

            case DrawCommand::Type::RoundedRect:
                AddQuad(cmd.pos, cmd.size, cmd.color, cmd.pos, cmd.size, cmd.rounding);
                break;

            case DrawCommand::Type::Line:
                DrawLine(cmd.pos, cmd.pos + cmd.size, cmd.color, cmd.thickness);
                break;

            case DrawCommand::Type::Text:
                // Flush shapes before text
                if (!m_VertexBuffer.empty()) {
                    FlushBatch();
                    m_VertexBuffer.clear();
                    m_IndexBuffer.clear();
                }
                // Render text
                if (m_FontManager) {
                    auto& textShaper = m_FontManager->GetTextShaper();
                    switch (cmd.textDirection) {
                    case 1:
                        textShaper.SetDirection(TextShaper::TextDirection::LTR);
                        break;
                    case 2:
                        textShaper.SetDirection(TextShaper::TextDirection::RTL);
                        break;
                    default:
                        textShaper.SetDirection(TextShaper::TextDirection::Auto);
                        break;
                    }
                }
                DrawText(cmd.pos, cmd.text, cmd.color);
                break;
            }
        }

        // Final flush
        if (!m_VertexBuffer.empty()) {
            FlushBatch();
        }

        // Clean up
        while (!m_ScissorStack.empty()) {
            PopScissor();
        }
    }

    void UIRenderer::DrawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
        AddQuad(pos, size, color, pos, size, 0.0f);
    }

    void UIRenderer::DrawRoundedRect(const glm::vec2& pos, const glm::vec2& size,
        const glm::vec4& color, float rounding) {
        AddQuad(pos, size, color, pos, size, rounding);
    }

    void UIRenderer::DrawText(const glm::vec2& pos, const std::string& text, const glm::vec4& color) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(m_TextShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(m_TextShaderProgram, "projection"),
            1, GL_FALSE, &m_Projection[0][0]);
        glUniform4f(glGetUniformLocation(m_TextShaderProgram, "textColor"),
            color.r, color.g, color.b, color.a);
        glUniform1i(glGetUniformLocation(m_TextShaderProgram, "useSubpixel"), 0);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(m_TextVAO);

        auto shapedGlyphs = m_FontManager->ShapeText(text);

        float baselineY = pos.y;
        const Character& refChar = m_FontManager->GetCharacter('H');
        baselineY += refChar.bearing.y;

        float x = pos.x;

        for (size_t i = 0; i < shapedGlyphs.size(); i++) {
            const auto& glyph = shapedGlyphs[i];
            const Character& ch = m_FontManager->GetCharacterByGlyphIndex(glyph.glyphIndex);

            float kerning = 0.0f;
            if (i > 0 && m_FontManager->GetRenderOptions().useKerning) {
                kerning = m_FontManager->GetKerning(
                    shapedGlyphs[i - 1].codepoint,
                    glyph.codepoint
                );
            }

            float xpos = x + glyph.offset.x + ch.bearing.x + kerning;
            float ypos = baselineY + glyph.offset.y - ch.bearing.y;

            float w = ch.size.x;
            float h = ch.size.y;

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 1.0f },
                { xpos,     ypos,       0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 0.0f },

                { xpos,     ypos + h,   0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 0.0f },
                { xpos + w, ypos + h,   1.0f, 1.0f }
            };

            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glBindBuffer(GL_ARRAY_BUFFER, m_TextVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += glyph.advance.x + kerning;

            // Apply letter spacing from font options
            float extraSpacing = m_FontManager->GetRenderOptions().letterSpacing;
            x += extraSpacing;
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_BLEND);
    }

    void UIRenderer::PushScissor(const glm::vec2& pos, const glm::vec2& size) {
        // Convert from top-left origin to bottom-left origin (OpenGL)
        int x = (int)pos.x;
        int y = (int)(m_WindowHeight - pos.y - size.y);
        int width = (int)size.x;
        int height = (int)size.y;

        // If there's already a scissor active, intersect with it
        if (!m_ScissorStack.empty()) {
            const auto& parent = m_ScissorStack.back();

            int x1 = glm::max(x, parent.x);
            int y1 = glm::max(y, parent.y);
            int x2 = glm::min(x + width, parent.x + parent.width);
            int y2 = glm::min(y + height, parent.y + parent.height);

            x = x1;
            y = y1;
            width = glm::max(0, x2 - x1);
            height = glm::max(0, y2 - y1);
        }

        ScissorRect rect = { x, y, width, height };
        m_ScissorStack.push_back(rect);

        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, width, height);
    }

    void UIRenderer::PopScissor() {
        if (m_ScissorStack.empty()) {
            return;
        }

        m_ScissorStack.pop_back();

        if (m_ScissorStack.empty()) {
            glDisable(GL_SCISSOR_TEST);
        }
        else {
            const auto& rect = m_ScissorStack.back();
            glScissor(rect.x, rect.y, rect.width, rect.height);
        }
    }

    void UIRenderer::DrawLine(const glm::vec2& start, const glm::vec2& end,
        const glm::vec4& color, float thickness) {
        glm::vec2 dir = glm::normalize(end - start);
        glm::vec2 perp(-dir.y, dir.x);
        glm::vec2 offset = perp * (thickness * 0.5f);

        uint32_t indexStart = (uint32_t)m_VertexBuffer.size();

        // Lines don't need rounding, use 0
        glm::vec2 dummyPos(0, 0);
        glm::vec2 dummySize(0, 0);

        m_VertexBuffer.push_back({ start + offset, color, {0, 0}, dummyPos, dummySize, 0.0f });
        m_VertexBuffer.push_back({ start - offset, color, {0, 1}, dummyPos, dummySize, 0.0f });
        m_VertexBuffer.push_back({ end - offset, color, {1, 1}, dummyPos, dummySize, 0.0f });
        m_VertexBuffer.push_back({ end + offset, color, {1, 0}, dummyPos, dummySize, 0.0f });

        m_IndexBuffer.push_back(indexStart + 0);
        m_IndexBuffer.push_back(indexStart + 1);
        m_IndexBuffer.push_back(indexStart + 2);
        m_IndexBuffer.push_back(indexStart + 2);
        m_IndexBuffer.push_back(indexStart + 3);
        m_IndexBuffer.push_back(indexStart + 0);
    }

    void UIRenderer::AddQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color,
        const glm::vec2& rectPos, const glm::vec2& rectSize, float rounding) {
        uint32_t indexStart = (uint32_t)m_VertexBuffer.size();

        // All 4 vertices get the SAME rect data for SDF calculation
        m_VertexBuffer.push_back({ pos, color, {0, 0}, rectPos, rectSize, rounding });
        m_VertexBuffer.push_back({ pos + glm::vec2(size.x, 0), color, {1, 0}, rectPos, rectSize, rounding });
        m_VertexBuffer.push_back({ pos + size, color, {1, 1}, rectPos, rectSize, rounding });
        m_VertexBuffer.push_back({ pos + glm::vec2(0, size.y), color, {0, 1}, rectPos, rectSize, rounding });

        m_IndexBuffer.push_back(indexStart + 0);
        m_IndexBuffer.push_back(indexStart + 1);
        m_IndexBuffer.push_back(indexStart + 2);
        m_IndexBuffer.push_back(indexStart + 2);
        m_IndexBuffer.push_back(indexStart + 3);
        m_IndexBuffer.push_back(indexStart + 0);
    }

    void UIRenderer::FlushBatch() {
        if (m_VertexBuffer.empty() || m_ShaderProgram == 0) {
            return;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(m_ShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, "u_Projection"),
            1, GL_FALSE, &m_Projection[0][0]);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_VertexBuffer.size() * sizeof(UIVertex),
            m_VertexBuffer.data());

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_IndexBuffer.size() * sizeof(uint32_t),
            m_IndexBuffer.data());

        glDrawElements(GL_TRIANGLES, (GLsizei)m_IndexBuffer.size(), GL_UNSIGNED_INT, nullptr);

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "[UIRenderer] OpenGL error: " << err << std::endl;
        }

        glBindVertexArray(0);
        glUseProgram(0);
        glDisable(GL_BLEND);
    }

} // namespace Unicorn::UI