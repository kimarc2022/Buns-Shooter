#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Shader.h"

// Bitmap font renderer using an embedded 8x8 ASCII atlas (chars 0x20-0x7F).
// Text is positioned in NDC space; charW/charH are NDC units per character.
class Font {
public:
    ~Font();

    bool init(const std::string& shaderDir);

    // Draw text at NDC top-left (x, y). charW/charH in NDC units.
    void draw(const std::string& text, float x, float y,
              float charW, float charH, const glm::vec4& color);

    // Same but with a dark drop-shadow for readability on any background.
    void drawShadowed(const std::string& text, float x, float y,
                      float charW, float charH, const glm::vec4& color);

    float textWidth(const std::string& text, float charW) const {
        return float(text.size()) * charW;
    }

private:
    Shader sh_;
    GLuint tex_ = 0;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;

    std::vector<float> verts_;   // reused each frame

    void buildAtlas();
};
