#pragma once
#include <glm/glm.hpp>

namespace Unicorn {
    class Input {
    public:
        static bool IsKeyPressed(int keycode);
        static bool IsMouseButtonPressed(int button);
        static float GetMouseWheelDelta();
        static void SetMouseWheelDelta(float delta);
        static glm::vec2 GetMousePosition();
    };
}
