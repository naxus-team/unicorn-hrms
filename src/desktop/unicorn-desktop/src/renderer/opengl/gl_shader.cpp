#include "gl_shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Unicorn {
    GLShader::GLShader(const std::string& vertSrc, const std::string& fragSrc) {
        // TODO: Compile shaders
        m_Program = 0;
    }
    GLShader::~GLShader() { glDeleteProgram(m_Program); }
    void GLShader::Bind() const { glUseProgram(m_Program); }
    void GLShader::Unbind() const { glUseProgram(0); }
    void GLShader::SetInt(const std::string& name, int value) {
        glUniform1i(glGetUniformLocation(m_Program, name.c_str()), value);
    }
    void GLShader::SetFloat(const std::string& name, float value) {
        glUniform1f(glGetUniformLocation(m_Program, name.c_str()), value);
    }
    void GLShader::SetVec2(const std::string& name, const glm::vec2& value) {
        glUniform2fv(glGetUniformLocation(m_Program, name.c_str()), 1, glm::value_ptr(value));
    }
    void GLShader::SetVec4(const std::string& name, const glm::vec4& value) {
        glUniform4fv(glGetUniformLocation(m_Program, name.c_str()), 1, glm::value_ptr(value));
    }
    void GLShader::SetMat4(const std::string& name, const glm::mat4& value) {
        glUniformMatrix4fv(glGetUniformLocation(m_Program, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }
}
