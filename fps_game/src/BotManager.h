#pragma once

#include <vector>
#include <random>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Bot.h"
#include "Difficulty.h"
#include "Skin.h"

class World;
class Shader;
class Player;
class DropManager;

struct ShotResult {
    bool      hit      = false;
    glm::vec3 hitPos   = {};
    float     damage   = 0.0f;
    bool      killed   = false;  // bot permanently eliminated (out of lives)
    bool      headshot = false;
    WeaponType dropWeapon = WeaponType::Pistol;
};

struct DropRequest {
    glm::vec3  pos;
    WeaponType weaponType;
};

class BotManager {
public:
    ~BotManager();

    void init(int count, const World& world);
    void reset(const World& world);

    // Advance all bot AIs by dt seconds.
    void update(float dt, Player& player, const World& world, Difficulty diff,
                glm::vec2 zoneCenter, float zoneRadius, float zoneDamagePerSec,
                DropManager& drops);

    // Render all bots using the world shader (must have uView/uProj/uLightDir set by caller).
    void render(Shader& shader, const glm::vec3& lightDir, const glm::vec3& viewPos);

    // Render one humanoid figure (shared by bots and player).
    void renderHumanoid(Shader& sh, const glm::vec3& pos, const glm::vec3& facing,
                         float walkPhase, const Skin& skin) const;

    // Render the local player character (TPS mode).
    void renderPlayerCharacter(Shader& sh, const glm::vec3& lightDir, const glm::vec3& viewPos,
                                const glm::vec3& pos, const glm::vec3& facing,
                                float walkPhase, int skinIdx);

    // Player fires a hitscan shot from origin in dir; returns hit info.
    ShotResult playerShot(const glm::vec3& origin, const glm::vec3& dir, float damage = 100.0f);

    int aliveCount()     const;   // bots still in game (not permanently eliminated)
    int eliminatedCount()const;   // permanently dead bots
    int totalCount()     const { return static_cast<int>(bots_.size()); }
    const std::vector<Bot>& bots()    const { return bots_; }
    std::vector<Bot>&       botsRef()       { return bots_; }

    // Loot drops from permanently killed bots — poll each frame, then clear
    const std::vector<DropRequest>& pendingDrops() const { return pendingDrops_; }
    void clearDrops() { pendingDrops_.clear(); }

private:
    std::vector<Bot>         bots_;
    std::vector<DropRequest> pendingDrops_;
    std::mt19937             rng_ { 9001u };

    // Zone state cached at the start of each update() call.
    glm::vec2 zoneCenter_    { 0.0f, 0.0f };
    float     zoneRadius_    = 95.0f;
    float     zoneDamage_    = 0.0f;

    GLuint cubeVAO_ = 0;
    GLuint cubeVBO_ = 0;

    void setupGL();

    // Per-bot update helpers -------------------------------------------------
    void updateBot(Bot& bot, float dt, Player& player,
                   const World& world, const DifficultyParams& p,
                   DropManager& drops);

    void doPatrol(Bot& bot, float dt, const World& world,
                  const DifficultyParams& p,
                  bool canSeePlayer, float distToPlayer);

    void doChase(Bot& bot, float dt, const World& world,
                 const DifficultyParams& p,
                 const glm::vec3& toPlayer, bool canSeePlayer, float distToPlayer);

    void doAttack(Bot& bot, float dt, const DifficultyParams& p,
                  Player& player, bool canSeePlayer, float distToPlayer);

    void doFlee(Bot& bot, float dt, const World& world,
                const DifficultyParams& p,
                const glm::vec3& fromPlayer, float distToPlayer);

    // Geometry helpers -------------------------------------------------------
    bool    hasLOS          (const glm::vec3& a, const glm::vec3& b, const World& w) const;
    void    resolveCollision(Bot& bot, const World& world) const;
    glm::vec3 steer         (const Bot& bot, glm::vec3 desired, const World& world) const;
    glm::vec3 randomPoint   (const glm::vec3& near, float radius);

    void renderBox(Shader& sh, const glm::vec3& center, const glm::vec3& size,
                   const glm::vec3& color) const;
};
