#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"

// Floating damage number (projected from world space to screen each frame).
struct DamageNumber {
    glm::vec3 worldPos;
    float     value;
    float     life;
    float     maxLife;
    bool      headshot;   // red+large if true, yellow if false
};

// Bullet tracer: bright line from gun to impact point.
struct BulletTracer {
    glm::vec3 start, end;
    float     life;
    float     maxLife;
};

class Effects {
public:
    ~Effects();
    bool init(const std::string& shaderDir);

    // Spawn events ─────────────────────────────────────────────────────────
    void spawnDamageNum(const glm::vec3& worldPos, float dmg, bool headshot = false);
    void spawnTracer(const glm::vec3& start, const glm::vec3& end);
    void triggerHitMarker();
    void triggerDamageFlash();
    void triggerMuzzleFlash();
    void triggerKill();

    // Per-frame update ─────────────────────────────────────────────────────
    void update(float dt);

    // Render 3-D tracers (call before disabling depth test).
    void renderTracers(const glm::mat4& projView);

    // Accessors for Game to drive its own HUD rendering ────────────────────
    const std::vector<DamageNumber>& damageNumbers() const { return dmgNums_; }

    float hitMarkerAlpha()   const { return hitTimer_    > 0.0f ? hitTimer_    / kHitDur    : 0.0f; }
    float damageFlashAlpha() const { return damageTimer_ > 0.0f ? damageTimer_ / kDamageDur : 0.0f; }
    float muzzleFlashAlpha() const { return muzzleTimer_ > 0.0f ? muzzleTimer_ / kMuzzleDur : 0.0f; }

    int   killFeedCount() const { return static_cast<int>(killFeed_.size()); }
    float killFeedAlpha(int i) const;

private:
    static constexpr float kHitDur    = 0.18f;
    static constexpr float kDamageDur = 0.35f;
    static constexpr float kMuzzleDur = 0.07f;
    static constexpr float kTracerDur = 0.14f;
    static constexpr float kDmgDur    = 1.5f;
    static constexpr float kKillDur   = 3.5f;

    Shader lineShader_;
    GLuint lineVAO_ = 0, lineVBO_ = 0;

    std::vector<DamageNumber> dmgNums_;
    std::vector<BulletTracer> tracers_;
    std::vector<float>        killFeed_;  // remaining life per entry

    float hitTimer_    = 0.0f;
    float damageTimer_ = 0.0f;
    float muzzleTimer_ = 0.0f;
};
