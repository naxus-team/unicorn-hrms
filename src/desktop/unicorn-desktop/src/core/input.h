#pragma once
#include <glm/glm.hpp>
#include <functional>

namespace Unicorn {
    class Input {
    public:
        static bool IsKeyPressed(int keycode);
        static bool IsMouseButtonPressed(int button);
        static float GetMouseWheelDelta();
        static void SetMouseWheelDelta(float delta);
        static void ResetMouseWheel();
        static glm::vec2 GetMousePosition();
        static void SetCharCallback(std::function<void(unsigned int)> callback);
        static unsigned int GetLastChar();
        static void OnCharInput(unsigned int codepoint);

        static std::function<void(unsigned int)> s_CharCallback;
        static unsigned int s_LastChar;
    };
}