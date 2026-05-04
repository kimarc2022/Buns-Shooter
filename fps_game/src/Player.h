#pragma once

#include <glm/glm.hpp>
#include "Weapon.h"

struct GLFWwindow;
class Camera;

class Player {
public:
    glm::vec3 position  {0.0f, 0.0f, 0.0f};
    glm::vec3 velocity  {0.0f};
    float     height    = 1.8f;
    float     radius    = 0.4f;
    float     health    = 100.0f;
    float     shield    = 100.0f;
    float     maxShield = 100.0f;
    float     moveSpeed = 9.0f;
    float     sprintMul = 1.9f;
    float     jumpSpeed = 7.5f;
    bool      onGround  = true;

    // ── Weapons ────────────────────────────────────────────────────────────────
    Weapon weapons[kWeaponCount];
    bool   hasWeapon[kWeaponCount] = {};  // false = slot empty / not yet looted
    int    currentWeapon = 0;
    bool   pickaxeActive = true;   // swinging pickaxe instead of shooting

    Weapon&       weapon()       { return weapons[currentWeapon]; }
    const Weapon& weapon() const { return weapons[currentWeapon]; }

    void initWeapons();
    void cycleWeapon(int delta);              // +1 = next, -1 = prev (skips empty)
    void selectWeapon(int slot);              // 0..4; no-op if slot is empty
    void giveWeapon(WeaponType t, int ammo, int reserve, int nameIdx = 0); // fills slot for that type

    // ── Pickaxe ────────────────────────────────────────────────────────────────
    float pickaxeCooldown  = 0.0f;
    float pickaxeSwingAnim = 0.0f;  // 0=idle 1=mid-swing

    // ── Materials ──────────────────────────────────────────────────────────────
    int wood  = 0;
    int brick = 0;
    int metal = 0;

    // ── Inventory ──────────────────────────────────────────────────────────────
    int  medkits       = 0;
    int  bandages      = 0;
    int  shieldPotions = 0;
    int  miniShields   = 0;
    bool useHeal();   // consume health/shield item; returns true if something used

    // ── Dash ───────────────────────────────────────────────────────────────────
    static constexpr float kDashCooldown = 3.0f;
    static constexpr float kDashDuration = 0.25f;
    static constexpr float kDashSpeed    = 18.0f;

    float     dashCooldown = 0.0f;
    bool      dashing      = false;
    float     dashTimer    = 0.0f;
    glm::vec3 dashDir      {0.0f};

    void activateDash(const glm::vec3& dir);

    // ── Grappling hook ─────────────────────────────────────────────────────────
    static constexpr float kGrappleCooldown = 8.0f;
    static constexpr float kGrappleSpeed    = 22.0f;
    static constexpr float kGrappleRange    = 25.0f;

    float     grappleCooldown = 0.0f;
    bool      grappleActive   = false;
    glm::vec3 grapplePoint    {0.0f};

    void activateGrapple(const glm::vec3& point);
    void cancelGrapple();

    // Shield absorbs damage first; resets regen delay. Call instead of health -= x.
    void takeDamage(float dmg);

    static constexpr float kShieldRegenDelay = 5.0f;
    static constexpr float kShieldRegenRate  = 8.0f;

    // ── Core ───────────────────────────────────────────────────────────────────
    void handleInput(GLFWwindow* win, const Camera& cam, float dt);
    void update(float dt);
    glm::vec3 eyePos() const;

private:
    float shieldRegenTimer_ = 0.0f;
};
