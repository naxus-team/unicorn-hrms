#pragma once

#include <string>
#include <glm/glm.hpp>

namespace Unicorn::UI {

    // Draw command for rendering
    struct DrawCommand {
        enum class Type {
            Rect,
            RoundedRect,
            Text,
            Line,
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
    };

} // namespace Unicorn::UI