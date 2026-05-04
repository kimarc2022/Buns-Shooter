#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

// Thin RAII wrapper around an OpenGL program built from a vertex/fragment pair.
class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath);
    void use() const;
    GLuint id() const { return program_; }

    void setMat4 (const std::string& name, const glm::mat4& m) const;
    void setVec2 (const std::string& name, const glm::vec2& v) const;
    void setVec3 (const std::string& name, const glm::vec3& v) const;
    void setVec4 (const std::string& name, const glm::vec4& v) const;
    void setFloat(const std::string& name, float v) const;
    void setInt  (const std::string& name, int v) const;

private:
    GLuint program_ = 0;
};
