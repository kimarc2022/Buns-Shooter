#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

static GLuint compileStage(GLenum type, const std::string& src, const std::string& tag) {
    GLuint sh = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(sh, 1, &c, nullptr);
    glCompileShader(sh);
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
        std::cerr << "[Shader] compile error (" << tag << "):\n" << log << "\n";
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

static std::string slurp(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "[Shader] cannot open " << path << "\n";
        return {};
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

Shader::~Shader() {
    if (program_) glDeleteProgram(program_);
}

bool Shader::loadFromFiles(const std::string& vp, const std::string& fp) {
    std::string vs = slurp(vp);
    std::string fs = slurp(fp);
    if (vs.empty() || fs.empty()) return false;

    GLuint v = compileStage(GL_VERTEX_SHADER,   vs, vp);
    GLuint f = compileStage(GL_FRAGMENT_SHADER, fs, fp);
    if (!v || !f) {
        if (v) glDeleteShader(v);
        if (f) glDeleteShader(f);
        return false;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, v);
    glAttachShader(program_, f);
    glLinkProgram(program_);

    GLint ok = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(program_, sizeof(log), nullptr, log);
        std::cerr << "[Shader] link error:\n" << log << "\n";
        glDeleteProgram(program_);
        program_ = 0;
    }

    glDeleteShader(v);
    glDeleteShader(f);
    return program_ != 0;
}

void Shader::use() const { glUseProgram(program_); }

void Shader::setMat4(const std::string& n, const glm::mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(program_, n.c_str()), 1, GL_FALSE, &m[0][0]);
}
void Shader::setVec2(const std::string& n, const glm::vec2& v) const {
    glUniform2fv(glGetUniformLocation(program_, n.c_str()), 1, &v.x);
}
void Shader::setVec3(const std::string& n, const glm::vec3& v) const {
    glUniform3fv(glGetUniformLocation(program_, n.c_str()), 1, &v.x);
}
void Shader::setVec4(const std::string& n, const glm::vec4& v) const {
    glUniform4fv(glGetUniformLocation(program_, n.c_str()), 1, &v.x);
}
void Shader::setFloat(const std::string& n, float v) const {
    glUniform1f(glGetUniformLocation(program_, n.c_str()), v);
}
void Shader::setInt(const std::string& n, int v) const {
    glUniform1i(glGetUniformLocation(program_, n.c_str()), v);
}
