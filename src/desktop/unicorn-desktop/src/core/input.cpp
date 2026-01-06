#include "input.h"
#include "application.h"
#include "window.h"
#include <GLFW/glfw3.h>

namespace Unicorn {
    static float s_MouseWheelDelta = 0.0f;

    bool Input::IsKeyPressed(int keycode) {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        return glfwGetKey(window, keycode) == GLFW_PRESS;
    }

    bool Input::IsMouseButtonPressed(int button) {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        return glfwGetMouseButton(window, button) == GLFW_PRESS;
    }

    float Input::GetMouseWheelDelta() {
        float delta = s_MouseWheelDelta;
        s_MouseWheelDelta = 0.0f; // Reset after reading
        return delta;
    }

    void Input::SetMouseWheelDelta(float delta) {
        s_MouseWheelDelta = delta;
    }

    glm::vec2 Input::GetMousePosition() {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return {(float)x, (float)y};
    }
}
