#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;
class Camera;
class Player;

enum class BuildType { Wall = 0, Floor, Ramp };

struct BuildPiece {
    glm::vec3 pos;   // bottom-centre of piece in world space
    BuildType type;
    float     yaw;   // Y-rotation degrees
};

class BuildManager {
public:
    ~BuildManager();
    void init();
    void reset();   // clear all placed pieces, restore wood

    bool      buildMode = false;
    BuildType curType   = BuildType::Wall;
    float     curYaw    = 0.0f;
    int       wood      = 0;
    int       brick     = 0;
    int       metal     = 0;

    void cycleType(int delta);
    void rotate90();

    // Returns true and deducts wood if placement succeeded.
    bool place(const Player& p, const Camera& cam);

    void render    (Shader& sh, const glm::vec3& lightDir, const glm::vec3& eye);
    void renderGhost(Shader& sh, const glm::vec3& lightDir, const glm::vec3& eye,
                     const Player& p, const Camera& cam);

    // Push pos out of any build-piece AABBs (for player collision).
    void resolvePlayerCollision(glm::vec3& pos, float radius, float height) const;

    static glm::vec3 pieceSize (BuildType t);
    static glm::vec3 pieceColor(BuildType t);

private:
    std::vector<BuildPiece> pieces_;
    GLuint vao_ = 0, vbo_ = 0;

    void setupGL();
    glm::vec3 ghostPos(const Player& p, const Camera& cam) const;
    void renderPiece(Shader& sh, const BuildPiece& bp, float alpha) const;
    void renderBox  (Shader& sh, const glm::vec3& c, const glm::vec3& s,
                     const glm::vec3& col, float alpha, float yaw,
                     float xTilt = 0.0f) const;
};
