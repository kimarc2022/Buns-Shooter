#include "Pickup.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <random>

static const float kCube[] = {
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

PickupManager::~PickupManager() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}

void PickupManager::setupGL() {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCube), kCube, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glBindVertexArray(0);
}

void PickupManager::init(unsigned seed) {
    setupGL();
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> posD(-180.0f, 180.0f);
    std::uniform_real_distribution<float> spinD(0.0f, 6.28f);

    auto add = [&](PickupKind k, int n) {
        for (int i = 0; i < n; ++i) {
            Pickup p;
            p.kind = k;
            p.pos  = glm::vec3(posD(rng), 0.0f, posD(rng));
            p.spin = spinD(rng);
            pickups_.push_back(p);
        }
    };

    add(PickupKind::Medkit,       20);
    add(PickupKind::Bandage,      25);
    add(PickupKind::ShieldPotion, 20);
    add(PickupKind::MiniShield,   30);
}

void PickupManager::update(float dt) {
    for (auto& p : pickups_) {
        p.spin += dt * 1.6f;
        if (p.taken) {
            p.respawn -= dt;
            if (p.respawn <= 0.0f) p.taken = false;
        }
    }
}

int PickupManager::tryCollect(const glm::vec3& pos, float radius) {
    for (auto& p : pickups_) {
        if (p.taken) continue;
        glm::vec3 diff = p.pos - pos;
        diff.y = 0.0f;
        if (glm::length(diff) < radius) {
            p.taken   = true;
            p.respawn = 25.0f;
            return static_cast<int>(p.kind);
        }
    }
    return -1;
}

glm::vec3 PickupManager::kindColor(PickupKind k) {
    switch (k) {
        case PickupKind::Pistol:  return {0.80f, 0.80f, 0.80f};
        case PickupKind::Rifle:   return {0.20f, 0.90f, 0.20f};
        case PickupKind::Shotgun: return {0.95f, 0.50f, 0.05f};
        case PickupKind::Sniper:  return {0.20f, 0.60f, 1.00f};
        case PickupKind::SMG:     return {1.00f, 0.90f, 0.10f};
        case PickupKind::Medkit:       return {0.95f, 0.15f, 0.15f};
        case PickupKind::Bandage:      return {0.95f, 0.80f, 0.25f};
        case PickupKind::ShieldPotion: return {0.15f, 0.55f, 1.00f};
        case PickupKind::MiniShield:   return {0.40f, 0.80f, 1.00f};
        default:                       return {1.0f,  1.0f,  1.0f};
    }
}

void PickupManager::render(Shader& sh, const glm::vec3& lightDir, const glm::vec3& eye) {
    sh.use();
    sh.setVec3 ("uLightDir", lightDir);
    sh.setVec3 ("uViewPos",  eye);
    sh.setFloat("uAlpha",    1.0f);

    glBindVertexArray(vao_);
    for (const Pickup& p : pickups_) {
        if (p.taken) continue;
        glm::mat4 m = glm::translate(glm::mat4(1.0f),
                                     p.pos + glm::vec3(0.0f, 0.55f, 0.0f));
        m = glm::rotate(m, p.spin, glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(0.45f));
        sh.setMat4("uModel", m);
        sh.setVec3("uColor", kindColor(p.kind));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
}
