#include "window.h"
#include "input.h"
#include "application.h"
#include "../ui/ui_context.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace Unicorn {

    static bool s_WindowResized = false;
    static uint32_t s_NewWidth = 0;
    static uint32_t s_NewHeight = 0;

    class WindowsWindow : public Window {
    public:
        WindowsWindow(const WindowProps& props) {
            glfwSetErrorCallback([](int error, const char* description) {
                std::cerr << "GLFW Error " << error << ": " << description << std::endl;
                });

            if (!glfwInit()) {
                throw std::runtime_error("Failed to initialize GLFW");
            }

            glfwWindowHint(GLFW_SAMPLES, 8);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

            m_Window = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);

            if (!m_Window) {
                glfwTerminate();
                throw std::runtime_error("Failed to create GLFW window");
            }

            glfwMakeContextCurrent(m_Window);
            glfwSwapInterval(1);

            m_Data.width = props.width;
            m_Data.height = props.height;

            glfwSetWindowUserPointer(m_Window, &m_Data);

            // ========== EVENT-DRIVEN CALLBACKS (Wake from glfwWaitEvents) ==========

            // 1. Framebuffer resize
            glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
                WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
                if (data) {
                    data->width = width;
                    data->height = height;
                }
                s_WindowResized = true;
                s_NewWidth = width;
                s_NewHeight = height;

                Application::Get().GetUI().MarkDirty();

                // Wake glfwWaitEvents()
                glfwPostEmptyEvent();
                });

            // 2. Mouse button - ONLY wake on actual button state change
            glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
                Application::Get().GetUI().MarkDirty();
                glfwPostEmptyEvent();
                });

            // 3. Scroll
            glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
                Input::SetMouseWheelDelta(static_cast<float>(yoffset));
                Application::Get().GetUI().MarkDirty();
                glfwPostEmptyEvent();
                });

            // 4. Key press - ONLY wake on actual key event
            glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                Application::Get().GetUI().MarkDirty();
                glfwPostEmptyEvent();
                });

            // ========================================
            // CRITICAL: NO mouse move callback!
            // Mouse position is read ONLY when rendering
            // This prevents waking from glfwWaitEvents on every pixel movement
            // ========================================

            // Mouse move callback is DISABLED for maximum idle efficiency
            // Position is read on-demand in Input::GetMousePosition()

            // If you NEED hover effects, enable this with throttling:
            /*
            glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
                static auto lastUpdate = std::chrono::high_resolution_clock::now();
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate);

                // Throttle to 30 FPS max (33ms between updates)
                if (elapsed.count() < 33) return;

                lastUpdate = now;
                Application::Get().GetUI().MarkDirty();
                glfwPostEmptyEvent();
            });
            */

            glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                Application::Get().GetUI().MarkDirty();
                glfwPostEmptyEvent();
                });

            glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint) {
                Input::OnCharInput(codepoint);
                Application::Get().GetUI().MarkDirty();
                glfwPostEmptyEvent();
                });

            // 6. Window focus - wake ONLY on focus gain (not focus loss)
            glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, int focused) {
                if (focused == GLFW_TRUE) {
                    Application::Get().GetUI().MarkDirty();
                    glfwPostEmptyEvent();
                }
                });

            m_HandCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
            m_ArrowCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
            m_IBeamCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        }

        ~WindowsWindow() {
            if (m_HandCursor) glfwDestroyCursor(m_HandCursor);
            if (m_ArrowCursor) glfwDestroyCursor(m_ArrowCursor);
            if (m_IBeamCursor) glfwDestroyCursor(m_IBeamCursor);

            if (m_Window) {
                glfwDestroyWindow(m_Window);
            }
            glfwTerminate();
        }

        void SetCursor(int cursorType) override {
            GLFWcursor* cursor = m_ArrowCursor;

            switch (cursorType) {
            case 1: cursor = m_HandCursor; break;  // Hand/Pointer
            case 2: cursor = m_IBeamCursor; break; // Text I-beam
            default: cursor = m_ArrowCursor; break; // Arrow
            }

            glfwSetCursor(m_Window, cursor);
        }

        void OnUpdate() override {}

        void SwapBuffers() override {
            glfwSwapBuffers(m_Window);
        }

        bool ShouldClose() const override {
            return glfwWindowShouldClose(m_Window);
        }

        uint32_t GetWidth() const override {
            return m_Data.width;
        }

        uint32_t GetHeight() const override {
            return m_Data.height;
        }

        bool WasResized() override {
            if (s_WindowResized) {
                m_Data.width = s_NewWidth;
                m_Data.height = s_NewHeight;
                s_WindowResized = false;
                return true;
            }
            return false;
        }

        bool IsResizing() const override {
            return false;
        }

        void* GetNativeWindow() const override {
            return m_Window;
        }

    private:
        GLFWwindow* m_Window = nullptr;
        GLFWcursor* m_HandCursor = nullptr;
        GLFWcursor* m_ArrowCursor = nullptr;
        GLFWcursor* m_IBeamCursor = nullptr;

        struct WindowData {
            uint32_t width, height;
        };
        WindowData m_Data;
    };

    Window* Window::Create(const WindowProps& props) {
        return new WindowsWindow(props);
    }

} // namespace Unicorn