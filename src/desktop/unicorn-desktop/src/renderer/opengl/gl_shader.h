#pragma once
#include "../shader.h"

namespace Unicorn {
    class GLShader : public Shader {
    public:
        GLShader(const std::string& vertSrc, const std::string& fragSrc);
        ~GLShader();
        void Bind() const override;
        void Unbind() const override;
        void SetInt(const std::string& name, int value) override;
        void SetFloat(const std::string& name, float value) override;
        void SetVec2(const std::string& name, const glm::vec2& value) override;
        void SetVec4(const std::string& name, const glm::vec4& value) override;
        void SetMat4(const std::string& name, const glm::mat4& value) override;
    private:
        uint32_t m_Program;
    };
}
