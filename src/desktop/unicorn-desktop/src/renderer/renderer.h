#pragma once
#include <glm/glm.hpp>

namespace Unicorn {
    class Renderer {
    public:
        Renderer();
        ~Renderer();

        void Init();
        void Shutdown();
        void BeginFrame();
        void EndFrame();

        void Clear(const glm::vec4& color);
        void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);

        // Add this function for window resize handling
        void OnWindowResize(uint32_t width, uint32_t height);
    };
}