#include "renderer.h"
#include "opengl/gl_renderer.h"
#include <glad/glad.h>
#include <iostream>

namespace Unicorn {

    Renderer::Renderer() {}

    Renderer::~Renderer() {}

    void Renderer::Init() {
        if (!gladLoadGL()) {
            std::cerr << "GLAD init failed" << std::endl;
            return;
        }
        std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void Renderer::Shutdown() {}

    void Renderer::BeginFrame() {}

    void Renderer::EndFrame() {}

    void Renderer::Clear(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
        // TODO: Implement batched rendering
    }

    // Add this function for window resize handling
    void Renderer::OnWindowResize(uint32_t width, uint32_t height) {
        glViewport(0, 0, width, height);
        std::cout << "[Renderer] Viewport updated to: " << width << "x" << height << std::endl;
    }

}