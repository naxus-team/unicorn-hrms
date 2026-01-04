#include "input.h"
#include "application.h"
#include "window.h"
#include <GLFW/glfw3.h>

namespace Unicorn {
    bool Input::IsKeyPressed(int keycode) {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        return glfwGetKey(window, keycode) == GLFW_PRESS;
    }

    bool Input::IsMouseButtonPressed(int button) {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        return glfwGetMouseButton(window, button) == GLFW_PRESS;
    }

    glm::vec2 Input::GetMousePosition() {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return {(float)x, (float)y};
    }
}
