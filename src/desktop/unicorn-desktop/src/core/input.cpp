#include "input.h"
#include "../core/application.h"
#include "../core/window.h"
#include <GLFW/glfw3.h>

namespace Unicorn {
    static float s_MouseWheelDelta = 0.0f;
    static bool s_MouseWheelConsumed = false;

    std::function<void(unsigned int)> Input::s_CharCallback;
    unsigned int Input::s_LastChar = 0;

    bool Input::IsKeyPressed(int keycode) {
        GLFWwindow* window = static_cast<GLFWwindow*>(
            Application::Get().GetWindow().GetNativeWindow()
            );
        int state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(int button) {
        GLFWwindow* window = static_cast<GLFWwindow*>(
            Application::Get().GetWindow().GetNativeWindow()
            );
        int state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    float Input::GetMouseWheelDelta() {
        if (s_MouseWheelConsumed) {
            return 0.0f;
        }

        float delta = s_MouseWheelDelta;
        s_MouseWheelConsumed = true;
        return delta;
    }

    void Input::SetMouseWheelDelta(float delta) {
        s_MouseWheelDelta = delta;
        s_MouseWheelConsumed = false;
    }

    void Input::ResetMouseWheel() {
        s_MouseWheelDelta = 0.0f;
        s_MouseWheelConsumed = false;
    }

    glm::vec2 Input::GetMousePosition() {
        GLFWwindow* window = static_cast<GLFWwindow*>(
            Application::Get().GetWindow().GetNativeWindow()
            );
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    void Input::OnCharInput(unsigned int codepoint) {
        s_LastChar = codepoint;
        if (s_CharCallback) {
            s_CharCallback(codepoint);
        }
    }

    unsigned int Input::GetLastChar() {
        unsigned int lastChar = s_LastChar;
        s_LastChar = 0;
        return lastChar;
    }

    void Input::SetCharCallback(std::function<void(unsigned int)> callback) {
        s_CharCallback = callback;
    }
}