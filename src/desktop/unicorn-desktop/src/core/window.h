#pragma once

#include <string>
#include <functional>

struct GLFWwindow;

namespace Unicorn {

    struct WindowProps {
        std::string title = "Unicorn";
        uint32_t width = 1280;
        uint32_t height = 720;
    };

    class Window {
    public:
        virtual ~Window() = default;

        virtual void OnUpdate() = 0;
        virtual void SwapBuffers() = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual bool ShouldClose() const = 0;
        virtual bool WasResized() = 0;
        virtual void* GetNativeWindow() const = 0;

        static Window* Create(const WindowProps& props);
    };

} // namespace Unicorn