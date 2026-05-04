#include "Build.h"
#include "Camera.h"
#include "Player.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

static const float kBuildCube[] = {
    -0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f, 0.5f,-0.5f, 0,0,-1,
     0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f,-0.5f,-0.5f, 0,0,-1,
    -0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f, 0.5f, 0.5f, 0,0, 1,
     0.5f, 0.5f, 0.5f, 0,0, 1, -0.5f, 0.5f, 0.5f, 0,0, 1, -0.5f,-0.5f, 0.5f, 0,0, 1,
    -0.5f, 0.5f, 0.5f,-1,0, 0, -0.5f, 0.5f,-0.5f,-1,0, 0, -0.5f,-0.5f,-0.5f,-1,0, 0,
    -0.5f,-0.5f,-0.5f,-1,0, 0, -0.5f,-0.5f, 0.5f,-1,0, 0, -0.5f, 0.5f, 0.5f,-1,0, 0,
     0.5f, 0.5f, 0.5f, 1,0, 0,  0.5f, 0.5f,-0.5f, 1,0, 0,  0.5f,-0.5f,-0.5f, 1,0, 0,
     0.5f,-0.5f,-0.5f, 1,0, 0,  0.5f,-0.5f, 0.5f, 1,0, 0,  0.5f, 0.5f, 0.5f, 1,0, 0,
    -0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f, 0.5f, 0,-1,0,
     0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f,-0.5f, 0,-1,0,
    -0.5f, 0.5f,-0.5f, 0, 1,0,  0.5f, 0.5f,-0.5f, 0, 1,0,  0.5f, 0.5f, 0.5f, 0, 1,0,
     0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f,-0.5f, 0, 1,0,
};

BuildManager::~BuildManager() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}

void BuildManager::setupGL() {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kBuildCube), kBuildCube, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glBindVertexArray(0);
}

void BuildManager::init() {
    setupGL();
    wood = 0; brick = 0; metal = 0;
}

void BuildManager::reset() {
    pieces_.clear();
    wood = 0; brick = 0; metal = 0;
    buildMode = false;
    curYaw    = 0.0f;
    curType   = BuildType::Wall;
}

glm::vec3 BuildManager::pieceSize(BuildType t) {
    switch (t) {
        case BuildType::Wall:  return {2.5f, 3.0f, 0.30f};
        case BuildType::Floor: return {2.5f, 0.30f, 2.5f};
        case BuildType::Ramp:  return {2.5f, 0.30f, 3.54f};
    }
    return {1.0f, 1.0f, 1.0f};
}

glm::vec3 BuildManager::pieceColor(BuildType t) {
    switch (t) {
        case BuildType::Wall:  return {0.80f, 0.64f, 0.42f};
        case BuildType::Floor: return {0.76f, 0.60f, 0.38f};
        case BuildType::Ramp:  return {0.72f, 0.56f, 0.35f};
    }
    return {0.8f, 0.6f, 0.4f};
}

void BuildManager::cycleType(int delta) {
    int t = (static_cast<int>(curType) + delta + 3) % 3;
    curType = static_cast<BuildType>(t);
}

void BuildManager::rotate90() {
    curYaw = fmodf(curYaw + 90.0f, 360.0f);
}

glm::vec3 BuildManager::ghostPos(const Player& p, const Camera& cam) const {
    glm::vec3 fwd = cam.forwardFlat();
    glm::vec3 gp  = p.position + fwd * 4.5f;
    auto snap = [](float v, float g) { return roundf(v / g) * g; };
    gp.x = snap(gp.x, 2.5f);
    gp.z = snap(gp.z, 2.5f);
    gp.y = p.position.y;
    return gp;
}

bool BuildManager::place(const Player& p, const Camera& cam) {
    if (wood < 10) return false;
    BuildPiece bp;
    bp.pos  = ghostPos(p, cam);
    bp.type = curType;
    bp.yaw  = curYaw;
    pieces_.push_back(bp);
    wood -= 10;
    return true;
}

void BuildManager::renderBox(Shader& sh, const glm::vec3& c, const glm::vec3& s,
                              const glm::vec3& col, float alpha, float yaw,
                              float xTilt) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), c);
    m = glm::rotate(m, glm::radians(yaw),   glm::vec3(0, 1, 0));
    if (xTilt != 0.0f)
        m = glm::rotate(m, glm::radians(xTilt), glm::vec3(1, 0, 0));
    m = glm::scale(m, s);
    sh.setMat4  ("uModel", m);
    sh.setVec3  ("uColor", col);
    sh.setFloat ("uAlpha", alpha);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void BuildManager::renderPiece(Shader& sh, const BuildPiece& bp, float alpha) const {
    glm::vec3 col  = pieceColor(bp.type);
    glm::vec3 dark = col * 0.72f;

    switch (bp.type) {
    case BuildType::Wall: {
        glm::vec3 ctr = bp.pos + glm::vec3(0, 1.5f, 0);
        glm::vec3 sz  = pieceSize(BuildType::Wall);
        renderBox(sh, ctr, sz, col, alpha, bp.yaw);
        // Top and bottom frame beams
        renderBox(sh, ctr + glm::vec3(0,  sz.y*0.5f-0.11f, 0),
                  {sz.x, 0.22f, sz.z+0.05f}, dark, alpha, bp.yaw);
        renderBox(sh, ctr + glm::vec3(0, -sz.y*0.5f+0.11f, 0),
                  {sz.x, 0.22f, sz.z+0.05f}, dark, alpha, bp.yaw);
        break;
    }
    case BuildType::Floor: {
        glm::vec3 ctr = bp.pos + glm::vec3(0, 0.15f, 0);
        glm::vec3 sz  = pieceSize(BuildType::Floor);
        renderBox(sh, ctr, sz, col, alpha, bp.yaw);
        // Plank seam
        renderBox(sh, ctr + glm::vec3(0, 0.16f, 0),
                  {sz.x*0.98f, 0.05f, 0.10f}, dark, alpha, bp.yaw);
        break;
    }
    case BuildType::Ramp: {
        // Tilted 45 degrees, rising forward (+Z local)
        glm::vec3 ctr = bp.pos + glm::vec3(0, 1.25f, 0);
        renderBox(sh, ctr, pieceSize(BuildType::Ramp), col, alpha, bp.yaw, -45.0f);
        break;
    }
    }
}

void BuildManager::render(Shader& sh, const glm::vec3& lightDir, const glm::vec3& eye) {
    if (pieces_.empty()) return;
    sh.use();
    sh.setVec3 ("uLightDir", lightDir);
    sh.setVec3 ("uViewPos",  eye);
    glBindVertexArray(vao_);
    for (const BuildPiece& bp : pieces_)
        renderPiece(sh, bp, 1.0f);
    sh.setFloat("uAlpha", 1.0f);
    glBindVertexArray(0);
}

void BuildManager::renderGhost(Shader& sh, const glm::vec3& lightDir, const glm::vec3& eye,
                                const Player& p, const Camera& cam) {
    BuildPiece ghost;
    ghost.pos  = ghostPos(p, cam);
    ghost.type = curType;
    ghost.yaw  = curYaw;

    sh.use();
    sh.setVec3("uLightDir", lightDir);
    sh.setVec3("uViewPos",  eye);
    glBindVertexArray(vao_);
    glDepthMask(GL_FALSE);
    renderPiece(sh, ghost, 0.45f);
    glDepthMask(GL_TRUE);
    sh.setFloat("uAlpha", 1.0f);
    glBindVertexArray(0);
}

void BuildManager::resolvePlayerCollision(glm::vec3& pos, float radius, float height) const {
    for (const BuildPiece& bp : pieces_) {
        if (bp.type == BuildType::Ramp) continue;

        glm::vec3 sz  = pieceSize(bp.type);
        glm::vec3 ctr = bp.pos;
        if      (bp.type == BuildType::Wall)  ctr.y += 1.5f;
        else if (bp.type == BuildType::Floor) ctr.y += 0.15f;

        glm::vec3 mn = ctr - sz * 0.5f;
        glm::vec3 mx = ctr + sz * 0.5f;

        if (pos.y + height < mn.y || pos.y > mx.y) continue;
        float px = pos.x, pz = pos.z;
        if (px + radius < mn.x || px - radius > mx.x) continue;
        if (pz + radius < mn.z || pz - radius > mx.z) continue;

        float oL = (px + radius) - mn.x;
        float oR = mx.x - (px - radius);
        float oB = (pz + radius) - mn.z;
        float oF = mx.z - (pz - radius);
        float minO = std::min({oL, oR, oB, oF});
        if      (minO == oL) pos.x -= oL;
        else if (minO == oR) pos.x += oR;
        else if (minO == oB) pos.z -= oB;
        else                 pos.z += oF;
    }
}
