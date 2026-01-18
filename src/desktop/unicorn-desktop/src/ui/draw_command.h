#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Unicorn::UI {

    // Border style types
    enum class BorderStyle {
        None,
        Solid,      // Single solid line
        Inset,      // 3D inset effect (sunken)
        Outset,     // 3D outset effect (raised)
        Groove,     // Double border with groove effect
        Ridge       // Double border with ridge effect
    };

    // Draw command for rendering
    struct DrawCommand {
        enum class Type {
            Rect,
            RoundedRect,
            Text,
            Line,
            Icon,
            PushScissor,
            PopScissor
        };

        Type type;
        glm::vec2 pos;
        glm::vec2 size;
        glm::vec4 color;
        float rounding = 0.0f;
        float thickness = 1.0f;
        std::string text;
        int textDirection = 0; // 0 = Auto, 1 = LTR, 2 = RTL
        uint32_t textureID = 0;

        // Border properties
        BorderStyle borderStyle = BorderStyle::None;
        float borderWidth = 1.0f;
        glm::vec4 borderColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.08f);
    };

    inline void DrawBorder(std::vector<DrawCommand>& commands, const glm::vec2& pos,
        const glm::vec2& size, BorderStyle style, float width,
        const glm::vec4& baseColor, float rounding = 0.0f) {

        if (style == BorderStyle::None) return;

        // Draw the border as a background rounded rectangle with the border color
        DrawCommand borderBg;
        borderBg.type = DrawCommand::Type::RoundedRect;
        borderBg.pos = pos;
        borderBg.size = size;
        borderBg.color = baseColor; // Border color fills the whole rect
        borderBg.rounding = rounding;
        commands.push_back(borderBg);

        // Draw the inner content area (slightly smaller) to create border effect
        // This will be drawn by BeginScrollablePanel as the white background
        // So we don't need to draw it here - just set up the border outline
    }

} // namespace Unicorn::UI