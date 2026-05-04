#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

enum class PickupKind { Pistol=0, Rifle, Shotgun, Sniper, SMG, Medkit, Bandage, ShieldPotion, MiniShield };
static constexpr int kPickupKindCount = 9;

struct Pickup {
    PickupKind kind;
    glm::vec3  pos;
    bool       taken   = false;
    float      respawn = 0.0f;  // countdown to re-appear
    float      spin    = 0.0f;  // visual rotation (radians)
};

class PickupManager {
public:
    ~PickupManager();
    void init(unsigned seed);
    void update(float dt);
    void render(Shader& sh, const glm::vec3& lightDir, const glm::vec3& eye);

    // Returns (int)PickupKind if a pickup was collected at pos, else -1.
    int tryCollect(const glm::vec3& pos, float radius = 1.8f);

    const std::vector<Pickup>& pickups() const { return pickups_; }

private:
    std::vector<Pickup> pickups_;
    GLuint vao_ = 0, vbo_ = 0;
    void setupGL();
    static glm::vec3 kindColor(PickupKind k);
};
