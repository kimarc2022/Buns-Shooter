#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <random>
#include <string>

#include "Weapon.h"

class Shader;
class Font;

// ── Loot items on the ground ──────────────────────────────────────────────────

struct DroppedItem {
    glm::vec3  pos;
    bool       hasWeapon   = false;
    WeaponType weaponType  = WeaponType::Pistol;
    int        nameIdx     = 0;    // weapon variant index (0..11)
    int        ammo        = 0;
    int        reserve     = 0;
    int        wood        = 0;
    int        brick       = 0;
    int        metal       = 0;
    float      bobTimer    = 0.0f;
    bool       fromBlue    = false;   // blue chest quality
};

// ── Chests ────────────────────────────────────────────────────────────────────

struct Chest {
    glm::vec3 pos;
    bool      opened   = false;
    bool      isBlue   = false;   // blue = rare, gold = common
    float     glowPhase= 0.0f;
};

class DropManager {
public:
    ~DropManager();
    void init(unsigned seed = 42u);
    void reset();
    void update(float dt);
    void render(Shader& sh, const glm::vec3& lightDir, const glm::vec3& eye);

    // ── Spawning ──────────────────────────────────────────────────────────────
    void spawnFromBot  (const glm::vec3& pos, WeaponType wt);
    void spawnFromChest(const glm::vec3& pos, bool isBlue);
    void spawnFloorLoot();   // random spawns across map at reset
    void spawnDroppedItem(const DroppedItem& di);  // spawn a pre-built item

    // ── Interaction ───────────────────────────────────────────────────────────
    // Returns index of item within pickup range, or -1
    int tryPickup  (const glm::vec3& playerPos);
    // Returns chest index if nearby and un-opened, or -1
    int tryOpenChest(const glm::vec3& playerPos);

    const DroppedItem& item (int i) const { return items_[i];  }
    const Chest&       chest(int i) const { return chests_[i]; }
    int itemCount () const { return static_cast<int>(items_.size());  }
    int chestCount() const { return static_cast<int>(chests_.size()); }
    void removeItem(int i);   // call after tryPickup to actually consume the item

    // Draw floating names + pickup hint above items near player (2D HUD pass)
    void renderLabels(const glm::vec3& playerPos, const glm::mat4& projView,
                      Font& font, int width, int height);

private:
    std::vector<DroppedItem> items_;
    std::vector<Chest>       chests_;

    std::mt19937 rng_;

    GLuint vao_ = 0, vbo_ = 0;
    void setupGL();
    void renderBox(Shader& sh, const glm::vec3& c, const glm::vec3& s, const glm::vec3& col) const;
};
