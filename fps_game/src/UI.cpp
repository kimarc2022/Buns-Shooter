#include "UI.h"
#include "Shader.h"

#include <algorithm>
#include <cmath>

// Unit quad in [0..1]x[0..1]; the vertex shader maps it to NDC.
static const float kQuadVerts[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
};

UI::~UI() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}

void UI::init() {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVerts), kQuadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void UI::renderQuad(Shader& sh, float x, float y, float w, float h, const glm::vec4& color) {
    sh.use();
    sh.setVec2("uPos",  glm::vec2(x, y));
    sh.setVec2("uSize", glm::vec2(w, h));
    sh.setVec4("uColor", color);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void UI::renderHealth(Shader& sh, float health, float maxHealth) {
    float pct = std::clamp(health / maxHealth, 0.0f, 1.0f);

    // Outer frame (top-left of screen).
    const float x = -0.95f, y =  0.92f;
    const float w =  0.40f, h =  0.06f;
    renderQuad(sh, x - 0.005f, y + 0.005f, w + 0.01f, h + 0.01f, {0,0,0,0.65f});
    renderQuad(sh, x, y, w, h, {0.10f, 0.10f, 0.10f, 0.85f});

    // Fill: green->yellow->red as health drops.
    glm::vec3 col = (pct > 0.5f)
        ? glm::mix(glm::vec3(1, 0.85f, 0.1f), glm::vec3(0.15f, 0.95f, 0.25f), (pct - 0.5f) * 2.0f)
        : glm::mix(glm::vec3(0.95f, 0.15f, 0.15f), glm::vec3(1, 0.85f, 0.1f), pct * 2.0f);
    renderQuad(sh, x + 0.005f, y - 0.005f, (w - 0.01f) * pct, h - 0.01f, glm::vec4(col, 1.0f));
}

void UI::renderShield(Shader& sh, float shield, float maxShield) {
    float pct = std::clamp(shield / maxShield, 0.0f, 1.0f);

    // Sits directly below the health bar (gap of 0.01).
    const float x = -0.95f, y = 0.84f;
    const float w =  0.40f, h = 0.045f;
    renderQuad(sh, x - 0.005f, y + 0.005f, w + 0.01f, h + 0.01f, {0,0,0,0.65f});
    renderQuad(sh, x, y, w, h, {0.10f, 0.10f, 0.10f, 0.85f});

    glm::vec3 col = (pct > 0.3f)
        ? glm::mix(glm::vec3(0.15f, 0.55f, 1.0f), glm::vec3(0.50f, 0.85f, 1.0f), pct)
        : glm::mix(glm::vec3(0.10f, 0.10f, 0.60f), glm::vec3(0.15f, 0.55f, 1.0f), pct / 0.3f);
    if (pct > 0.0f)
        renderQuad(sh, x + 0.005f, y - 0.005f, (w - 0.01f) * pct, h - 0.01f,
                   glm::vec4(col, 1.0f));
}

void UI::renderPlayerCount(Shader& sh, int alive, int total) {
    float pct = (total > 0) ? float(alive) / float(total) : 0.0f;

    // Top-right corner.
    const float w = 0.40f, h = 0.04f;
    const float x = 0.95f - w, y = 0.92f;

    renderQuad(sh, x - 0.005f, y + 0.005f, w + 0.01f, h + 0.01f, {0,0,0,0.65f});
    renderQuad(sh, x, y, w, h, {0.10f, 0.10f, 0.10f, 0.85f});
    renderQuad(sh, x + 0.005f, y - 0.005f, (w - 0.01f) * pct, h - 0.01f,
               {0.30f, 0.75f, 1.0f, 1.0f});

    // Tick marks every 10 players.
    for (int i = 1; i < 10; ++i) {
        float tx = x + (w * (i / 10.0f));
        renderQuad(sh, tx, y, 0.002f, h, {0,0,0,0.8f});
    }
}

void UI::renderCrosshair(Shader& sh, float spread, bool hit) {
    // spread widens the gap; hit flashes orange-red.
    glm::vec4 c = hit
        ? glm::vec4(1.0f, 0.25f, 0.10f, 0.97f)
        : glm::vec4(1.0f, 1.0f,  1.0f,  0.85f);

    // In UI convention: (x,y) = top-left in NDC, height goes down.
    const float gap = 0.016f + spread * 0.048f;  // center-to-arm-start
    const float arm = 0.030f;                      // arm length
    const float ath = arm * 0.80f;                 // arm height (vertical arms)
    const float thk = 0.0035f;                     // half-thickness

    // Right arm
    renderQuad(sh,  gap,          -thk,  arm,  thk * 2.0f, c);
    // Left arm
    renderQuad(sh, -gap - arm,    -thk,  arm,  thk * 2.0f, c);
    // Top arm (y = gap+ath, height = ath  →  spans NDC y from gap+ath down to gap)
    renderQuad(sh, -thk,           gap,  thk * 2.0f, ath, c);
    // Bottom arm (y = -gap, height = ath  →  spans from -gap down to -gap-ath)
    renderQuad(sh, -thk,          -gap - ath, thk * 2.0f, ath, c);
    // Center dot
    renderQuad(sh, -thk,          -thk,  thk * 2.0f, thk * 2.0f, c);
}

void UI::renderCameraIndicator(Shader& sh, bool fps) {
    glm::vec4 col = fps ? glm::vec4(0.20f, 0.95f, 1.00f, 1.0f)
                        : glm::vec4(1.00f, 0.30f, 0.85f, 1.0f);
    renderQuad(sh, -0.95f, 0.78f, 0.04f, 0.04f, col);
}

void UI::renderDifficulty(Shader& sh, int diff) {
    // One pip per difficulty level, spaced across the top-centre of the screen.
    static const glm::vec3 kCols[6] = {
        {0.20f, 0.90f, 0.25f},   // Easy    – green
        {0.20f, 0.70f, 0.95f},   // Medium  – cyan
        {0.95f, 0.90f, 0.15f},   // Hard    – yellow
        {0.95f, 0.50f, 0.10f},   // Insane  – orange
        {0.95f, 0.18f, 0.18f},   // Extreme – red
        {0.60f, 0.05f, 0.95f},   // Demon   – purple
    };

    const float W = 0.055f, H = 0.055f, GAP = 0.012f;
    const float totalW = 6.0f * W + 5.0f * GAP;
    float x = -totalW * 0.5f;

    for (int i = 0; i < 6; ++i) {
        bool active = (i == diff);
        glm::vec4 col(kCols[i], active ? 1.0f : 0.22f);

        // Bright outline for the active pip.
        if (active)
            renderQuad(sh, x - 0.004f, 0.975f + 0.004f, W + 0.008f, H + 0.008f,
                       {1, 1, 1, 0.55f});
        renderQuad(sh, x, 0.975f, W, H, col);
        x += W + GAP;
    }
}

// ── Weapon bar (bottom-left) ──────────────────────────────────────────────────
// Layout: dark background bar, coloured ammo-fill, reserve dots below.
void UI::renderWeapon(Shader& sh, const Weapon& w) {
    const WeaponStats& ws = getWeaponStats(w.type);
    const float bx = -0.95f, by = -0.82f;
    const float bw =  0.40f, bh =  0.06f;

    // Shadow + background
    renderQuad(sh, bx - 0.005f, by + 0.005f, bw + 0.01f, bh + 0.01f, {0,0,0,0.65f});
    renderQuad(sh, bx, by, bw, bh, {0.10f, 0.10f, 0.10f, 0.85f});

    // Ammo fill — orange when reloading, weapon colour otherwise.
    float ammoPct = ws.magSize > 0 ? float(w.ammo) / float(ws.magSize) : 0.0f;
    glm::vec3 fillCol = w.reloading ? glm::vec3(1.0f, 0.50f, 0.05f) : ws.hudColor;
    if (ammoPct > 0.0f)
        renderQuad(sh, bx + 0.005f, by - 0.005f,
                   (bw - 0.01f) * ammoPct, bh - 0.01f,
                   glm::vec4(fillCol, 1.0f));

    // Weapon-type colour strip on left edge (always full height).
    renderQuad(sh, bx, by, 0.018f, bh, glm::vec4(ws.hudColor, 0.9f));

    // Reserve dots: one square per magazine worth of reserve, up to 6.
    int mags = (ws.magSize > 0) ? (w.reserve / ws.magSize) : 0;
    if (mags > 6) mags = 6;
    for (int i = 0; i < mags; ++i) {
        float rx = bx + float(i) * 0.068f;
        renderQuad(sh, rx, by - 0.030f, 0.058f, 0.020f,
                   glm::vec4(ws.hudColor * 0.65f, 0.80f));
    }
}

// ── Items bar (bottom-right) ──────────────────────────────────────────────────
// Dash square | Grapple square | medkit dots | bandage dots
void UI::renderItems(Shader& sh,
                     float dashCooldown,    float dashMax,
                     float grappleCooldown, float grappleMax,
                     bool  grappleActive,
                     int   medkits, int bandages) {
    // --- Dash indicator ---
    const float ix = 0.60f, iy = -0.80f, isz = 0.09f;
    renderQuad(sh, ix - 0.004f, iy + 0.004f, isz + 0.008f, isz + 0.008f, {0,0,0,0.65f});
    renderQuad(sh, ix, iy, isz, isz, {0.10f, 0.10f, 0.10f, 0.85f});
    float dashReady = dashMax > 0.0f ? 1.0f - (dashCooldown / dashMax) : 1.0f;
    if (dashReady < 0.0f) dashReady = 0.0f;
    glm::vec3 dashCol = (dashReady >= 1.0f) ? glm::vec3(0.20f, 1.00f, 0.45f)
                                             : glm::vec3(0.10f, 0.55f, 0.20f);
    renderQuad(sh, ix + 0.004f, iy - 0.004f,
               (isz - 0.008f) * dashReady, isz - 0.008f,
               glm::vec4(dashCol, 1.0f));

    // --- Grapple indicator ---
    const float gx = ix + isz + 0.025f;
    renderQuad(sh, gx - 0.004f, iy + 0.004f, isz + 0.008f, isz + 0.008f, {0,0,0,0.65f});
    renderQuad(sh, gx, iy, isz, isz, {0.10f, 0.10f, 0.10f, 0.85f});
    float grappleReady = grappleMax > 0.0f ? 1.0f - (grappleCooldown / grappleMax) : 1.0f;
    if (grappleReady < 0.0f) grappleReady = 0.0f;
    glm::vec3 grappleCol = grappleActive   ? glm::vec3(1.00f, 0.85f, 0.10f)
                         : (grappleReady >= 1.0f) ? glm::vec3(0.15f, 0.80f, 1.00f)
                                                  : glm::vec3(0.08f, 0.40f, 0.60f);
    renderQuad(sh, gx + 0.004f, iy - 0.004f,
               (isz - 0.008f) * (grappleActive ? 1.0f : grappleReady), isz - 0.008f,
               glm::vec4(grappleCol, 1.0f));

    // --- Medkit dots (red) ---
    const float dotW = 0.042f, dotH = 0.030f, dotGap = 0.008f;
    float mx = ix, my = iy - isz - 0.020f;
    for (int i = 0; i < medkits && i < 5; ++i) {
        renderQuad(sh, mx + float(i) * (dotW + dotGap), my, dotW, dotH,
                   {0.95f, 0.15f, 0.15f, 0.90f});
    }

    // --- Bandage dots (yellow-orange) ---
    float bx2 = ix, by2 = my - dotH - 0.010f;
    for (int i = 0; i < bandages && i < 9; ++i) {
        renderQuad(sh, bx2 + float(i) * (dotW + dotGap), by2, dotW, dotH,
                   {0.95f, 0.80f, 0.20f, 0.90f});
    }
}

// ── Zone bar (bottom-centre) ──────────────────────────────────────────────────
// Shows time remaining in the current zone phase (waiting or shrinking).
void UI::renderZoneBar(Shader& sh, float timeLeft, float timeTotal,
                       bool shrinking, int phase) {
    const float w = 0.38f, h = 0.04f;
    const float x = -w * 0.5f, y = -0.94f;

    // Shadow + background.
    renderQuad(sh, x - 0.005f, y + 0.005f, w + 0.01f, h + 0.01f, {0,0,0,0.65f});
    renderQuad(sh, x, y, w, h, {0.10f, 0.10f, 0.10f, 0.85f});

    // Fill: cyan = waiting (safe), orange-red = shrinking (danger).
    float pct = (timeTotal > 0.0f)
        ? std::clamp(timeLeft / timeTotal, 0.0f, 1.0f) : 0.0f;

    glm::vec4 fillCol = shrinking
        ? glm::vec4(1.0f, 0.35f + 0.15f * pct, 0.05f, 1.0f)   // orange → red
        : glm::vec4(0.15f, 0.80f, 1.0f, 1.0f);                 // cyan

    if (pct > 0.0f)
        renderQuad(sh, x + 0.005f, y - 0.005f, (w - 0.01f) * pct, h - 0.01f, fillCol);

    // Thin phase-marker ticks (one per completed phase, up to 3).
    const int kMaxPhase = 4;
    for (int i = 1; i < kMaxPhase; ++i) {
        float tx = x + w * (float(i) / float(kMaxPhase));
        glm::vec4 tc = (i <= phase)
            ? glm::vec4(0.95f, 0.20f, 0.20f, 0.90f)   // passed phase: red
            : glm::vec4(0.00f, 0.00f, 0.00f, 0.70f);   // future phase: dark
        renderQuad(sh, tx, y, 0.003f, h, tc);
    }
}

// ── Zone warning vignette ─────────────────────────────────────────────────────
// Four screen-edge bars that pulse red when the player is outside the zone.
void UI::renderZoneWarning(Shader& sh, float gameTime) {
    float pulse  = 0.45f + 0.30f * sinf(gameTime * 5.0f);
    glm::vec4 c  { 1.0f, 0.15f, 0.05f, pulse };

    const float T = 0.10f;   // bar thickness in NDC

    renderQuad(sh, -1.0f,  1.0f - T, 2.0f, T, c);   // top
    renderQuad(sh, -1.0f, -1.0f,     2.0f, T, c);   // bottom
    renderQuad(sh, -1.0f, -1.0f,     T, 2.0f, c);   // left
    renderQuad(sh,  1.0f - T, -1.0f, T, 2.0f, c);   // right
}
