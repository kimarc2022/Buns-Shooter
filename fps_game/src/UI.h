#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Weapon.h"

class Shader;

// Placeholder HUD using solid-colour quads in NDC space.
// Text / font rendering arrives in step 5.
class UI {
public:
    ~UI();
    void init();

    // (x,y) = top-left corner in NDC (-1..1, +y up). Width/height in NDC units.
    void renderQuad(Shader& sh, float x, float y, float w, float h, const glm::vec4& color);

    // Top-left health bar.
    void renderHealth(Shader& sh, float health, float maxHealth);

    // Shield bar directly below the health bar (blue).
    void renderShield(Shader& sh, float shield, float maxShield);

    // Top-right alive-player bar.
    void renderPlayerCount(Shader& sh, int alive, int total);

    // FPS crosshair. spread [0..1] opens the gap; hit=true flashes red.
    void renderCrosshair(Shader& sh, float spread = 0.0f, bool hit = false);

    // Camera-mode dot (cyan=FPS, magenta=TPS).
    void renderCameraIndicator(Shader& sh, bool fps);

    // Six colour-coded difficulty pips at top-centre.
    void renderDifficulty(Shader& sh, int diff);

    // Bottom-left: current weapon ammo bar + reserve dots.
    void renderWeapon(Shader& sh, const Weapon& w);

    // Bottom-right: dash/grapple cooldown squares, medkit/bandage dot rows.
    void renderItems(Shader& sh,
                     float dashCooldown,    float dashMax,
                     float grappleCooldown, float grappleMax,
                     bool  grappleActive,
                     int   medkits, int bandages);

    // Bottom-centre: zone phase countdown bar.
    void renderZoneBar(Shader& sh, float timeLeft, float timeTotal,
                       bool shrinking, int phase);

    // Full-screen red vignette when player is outside the zone.
    void renderZoneWarning(Shader& sh, float gameTime);

private:
    GLuint vao_ = 0, vbo_ = 0;
};
