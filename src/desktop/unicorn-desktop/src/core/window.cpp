#include "window.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace Unicorn {

    // Static callback data
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
                std::cerr << "GLFW init failed" << std::endl;
                throw std::runtime_error("Failed to initialize GLFW");
            }

            std::cout << "GLFW initialized successfully" << std::endl;

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            // Enable MSAA (4x samples for smooth edges)
            glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

            m_Window = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);

            if (!m_Window) {
                std::cerr << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
                throw std::runtime_error("Failed to create GLFW window");
            }

            std::cout << "Window created successfully with MSAA 4x" << std::endl;

            glfwMakeContextCurrent(m_Window);

            m_Data.width = props.width;
            m_Data.height = props.height;

            // Set user pointer for callbacks
            glfwSetWindowUserPointer(m_Window, &m_Data);

            // Setup framebuffer resize callback
            glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
                WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
                if (data) {
                    data->width = width;
                    data->height = height;
                }

                s_WindowResized = true;
                s_NewWidth = width;
                s_NewHeight = height;

                std::cout << "[Window] Framebuffer resized to: " << width << "x" << height << std::endl;
                });

            // Setup window size callback
            glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
                WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
                if (data) {
                    data->width = width;
                    data->height = height;
                }

                std::cout << "[Window] Window resized to: " << width << "x" << height << std::endl;
                });
        }

        ~WindowsWindow() {
            if (m_Window) {
                glfwDestroyWindow(m_Window);
            }
            glfwTerminate();
        }

        void OnUpdate() override {
            glfwPollEvents();
        }

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

        void* GetNativeWindow() const override {
            return m_Window;
        }

    private:
        GLFWwindow* m_Window = nullptr;

        struct WindowData {
            uint32_t width, height;
        };
        WindowData m_Data;
    };

    Window* Window::Create(const WindowProps& props) {
        return new WindowsWindow(props);
    }

}