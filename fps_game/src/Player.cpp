#include "Player.h"
#include "Camera.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <iostream>

static constexpr float kGravity = -18.0f;

// ── Weapons ───────────────────────────────────────────────────────────────────

void Player::initWeapons() {
    static const WeaponType kTypes[kWeaponCount] = {
        WeaponType::Pistol, WeaponType::Rifle, WeaponType::Shotgun,
        WeaponType::Sniper, WeaponType::SMG
    };
    for (int i = 0; i < kWeaponCount; ++i) {
        weapons[i].type        = kTypes[i];
        weapons[i].ammo        = 0;
        weapons[i].reserve     = 0;
        weapons[i].fireTimer   = 0.0f;
        weapons[i].reloadTimer = 0.0f;
        weapons[i].reloading   = false;
        hasWeapon[i]           = false;
    }
    currentWeapon     = 0;
    pickaxeActive     = true;
    pickaxeCooldown   = 0.0f;
    pickaxeSwingAnim  = 0.0f;
    wood  = 0;
    brick = 0;
    metal = 0;
}

void Player::cycleWeapon(int delta) {
    int anyOwned = -1;
    for (int i = 0; i < kWeaponCount; ++i)
        if (hasWeapon[i]) { anyOwned = i; break; }
    if (anyOwned < 0) return;   // nothing to cycle to

    weapons[currentWeapon].reloading = false;
    int next = (currentWeapon + kWeaponCount + delta) % kWeaponCount;
    for (int tries = 0; tries < kWeaponCount; ++tries) {
        if (hasWeapon[next]) { currentWeapon = next; pickaxeActive = false; return; }
        next = (next + kWeaponCount + delta) % kWeaponCount;
    }
}

void Player::selectWeapon(int slot) {
    if (slot < 0 || slot >= kWeaponCount) return;
    if (!hasWeapon[slot]) return;
    weapons[currentWeapon].reloading = false;
    currentWeapon = slot;
    pickaxeActive = false;
}

void Player::giveWeapon(WeaponType t, int ammo, int reserve, int nameIdx) {
    int slot = static_cast<int>(t);
    weapons[slot].type        = t;
    weapons[slot].nameIdx     = nameIdx;
    weapons[slot].ammo        = ammo;
    weapons[slot].reserve     = reserve;
    weapons[slot].fireTimer   = 0.0f;
    weapons[slot].reloadTimer = 0.0f;
    weapons[slot].reloading   = false;
    hasWeapon[slot]           = true;
    currentWeapon             = slot;
    pickaxeActive             = false;
}

// ── Items ─────────────────────────────────────────────────────────────────────

void Player::takeDamage(float dmg) {
    shieldRegenTimer_ = kShieldRegenDelay;
    if (shield > 0.0f) {
        float absorbed = dmg < shield ? dmg : shield;
        shield -= absorbed;
        dmg    -= absorbed;
    }
    health -= dmg;
    if (health < 0.0f) health = 0.0f;
}

bool Player::useHeal() {
    // Health items first
    if (health < 100.0f) {
        if (medkits > 0)  { health = std::min(100.0f, health + 50.0f); --medkits;  return true; }
        if (bandages > 0) { health = std::min(100.0f, health + 25.0f); --bandages; return true; }
    }
    // Shield items
    if (shield < maxShield) {
        if (shieldPotions > 0) { shield = std::min(maxShield, shield + 50.0f); --shieldPotions; return true; }
        if (miniShields   > 0 && shield < 50.0f) { shield = std::min(50.0f, shield + 25.0f); --miniShields; return true; }
    }
    return false;
}

// ── Dash ──────────────────────────────────────────────────────────────────────

void Player::activateDash(const glm::vec3& dir) {
    if (dashCooldown > 0.0f || dashing) return;
    dashDir      = dir;
    dashing      = true;
    dashTimer    = kDashDuration;
    dashCooldown = kDashCooldown;
}

// ── Grapple ───────────────────────────────────────────────────────────────────

void Player::activateGrapple(const glm::vec3& point) {
    if (grappleCooldown > 0.0f) return;
    grapplePoint  = point;
    grappleActive = true;
}

void Player::cancelGrapple() {
    if (grappleActive) {
        grappleActive   = false;
        grappleCooldown = kGrappleCooldown;
    }
}

// ── Input ─────────────────────────────────────────────────────────────────────

void Player::handleInput(GLFWwindow* win, const Camera& cam, float dt) {
    (void)dt;
    // Dash and grapple override movement input.
    if (dashing || grappleActive) return;

    glm::vec3 wishDir(0.0f);
    glm::vec3 fwd = cam.forwardFlat();
    glm::vec3 rgt = cam.rightFlat();

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) wishDir += fwd;
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) wishDir -= fwd;
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) wishDir += rgt;
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) wishDir -= rgt;

    float speed = moveSpeed;
    if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speed *= sprintMul;

    if (glm::length(wishDir) > 0.0001f) {
        wishDir    = glm::normalize(wishDir);
        velocity.x = wishDir.x * speed;
        velocity.z = wishDir.z * speed;
    } else {
        velocity.x = 0.0f;
        velocity.z = 0.0f;
    }

    if (onGround && glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS) {
        velocity.y = jumpSpeed;
        onGround   = false;
    }
}

// ── Physics update ────────────────────────────────────────────────────────────

void Player::update(float dt) {
    if (dashCooldown    > 0.0f) dashCooldown    -= dt;
    if (grappleCooldown > 0.0f) grappleCooldown -= dt;
    if (pickaxeCooldown > 0.0f) pickaxeCooldown -= dt;
    if (pickaxeSwingAnim > 0.0f) pickaxeSwingAnim = std::max(0.0f, pickaxeSwingAnim - dt * 4.0f);

    // Shield regeneration: starts after kShieldRegenDelay seconds with no damage.
    if (shieldRegenTimer_ > 0.0f) {
        shieldRegenTimer_ -= dt;
    } else if (shield < maxShield) {
        shield += kShieldRegenRate * dt;
        if (shield > maxShield) shield = maxShield;
    }

    // Dash: override horizontal velocity for kDashDuration seconds.
    if (dashing) {
        dashTimer -= dt;
        velocity.x = dashDir.x * kDashSpeed;
        velocity.z = dashDir.z * kDashSpeed;
        if (dashTimer <= 0.0f) {
            dashing    = false;
            velocity.x = 0.0f;
            velocity.z = 0.0f;
        }
    }

    // Grapple: override velocity entirely to pull toward hook point.
    if (grappleActive) {
        glm::vec3 toHook = grapplePoint - position;
        float dist = glm::length(toHook);
        if (dist < 1.5f) {
            cancelGrapple();
        } else {
            velocity = glm::normalize(toHook) * kGrappleSpeed;
        }
    } else {
        velocity.y += kGravity * dt;
    }

    position += velocity * dt;

    if (position.y <= 0.0f) {
        position.y = 0.0f;
        velocity.y = 0.0f;
        onGround   = true;
        if (grappleActive) cancelGrapple();
    } else {
        onGround = false;
    }

    weapons[currentWeapon].update(dt);
}

glm::vec3 Player::eyePos() const {
    return position + glm::vec3(0.0f, height * 0.92f, 0.0f);
}
