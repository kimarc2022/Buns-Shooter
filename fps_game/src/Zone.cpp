#include "Zone.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>

static constexpr float kPI      = 3.14159265f;
static constexpr int   kRingSeg = 64;   // ring polygon segments
static constexpr float kInitRadius = 230.0f;

const Zone::Phase Zone::kPhases[Zone::kPhaseCount] = {
    // { waitTime, shrinkTime, targetRadius, targetCenter, damagePerSec }
    { 15.0f, 20.0f, 150.0f, {  8.0f,-12.0f},  2.0f },
    {  8.0f, 14.0f,  80.0f, { -6.0f, 10.0f},  4.0f },
    {  6.0f, 10.0f,  35.0f, {  4.0f, -5.0f},  8.0f },
    {  4.0f,  8.0f,   8.0f, {  0.0f,  0.0f}, 16.0f },
};

Zone::~Zone() {
    if (ringVBO_) glDeleteBuffers(1, &ringVBO_);
    if (ringVAO_) glDeleteVertexArrays(1, &ringVAO_);
}

void Zone::init() {
    // Allocate ring mesh: kRingSeg quads (2 triangles × 3 verts), 6 floats per vert.
    ringBuf_.resize(kRingSeg * 6 * 6, 0.0f);
    ringVertCount_ = kRingSeg * 6;

    glGenVertexArrays(1, &ringVAO_);
    glGenBuffers(1, &ringVBO_);
    glBindVertexArray(ringVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, ringVBO_);

    buildRingMesh();
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(ringBuf_.size() * sizeof(float)),
                 ringBuf_.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    phaseTimer_ = kPhases[0].waitTime;
    std::cout << "[Zone] Safe zone active (radius=" << kInitRadius
              << "). First shrink in " << kPhases[0].waitTime << "s.\n";
}

void Zone::reset() {
    phase_       = 0;
    shrinking_   = false;
    phaseTimer_  = kPhases[0].waitTime;
    radius_      = kInitRadius;
    startRadius_ = kInitRadius;
    center_      = { 0.0f, 0.0f };
    startCenter_ = { 0.0f, 0.0f };
    elapsed_     = 0.0f;
    buildRingMesh();
    uploadRing();
}

// Build flat ring on ground (y=0.08) from current radius_/center_.
void Zone::buildRingMesh() {
    const float inner = radius_;
    const float outer = radius_ + 3.0f;
    const float y     = 0.08f;
    const float cx    = center_.x;
    const float cz    = center_.y;   // center_.y is the Z world coordinate

    int idx = 0;
    for (int i = 0; i < kRingSeg; ++i) {
        float a0 = 2.0f * kPI * float(i)     / float(kRingSeg);
        float a1 = 2.0f * kPI * float(i + 1) / float(kRingSeg);

        float ix0 = cx + inner * cosf(a0),  iz0 = cz + inner * sinf(a0);
        float ix1 = cx + inner * cosf(a1),  iz1 = cz + inner * sinf(a1);
        float ox0 = cx + outer * cosf(a0),  oz0 = cz + outer * sinf(a0);
        float ox1 = cx + outer * cosf(a1),  oz1 = cz + outer * sinf(a1);

        auto pushV = [&](float x, float z) {
            ringBuf_[idx++] = x;
            ringBuf_[idx++] = y;
            ringBuf_[idx++] = z;
            ringBuf_[idx++] = 0.0f;
            ringBuf_[idx++] = 1.0f;
            ringBuf_[idx++] = 0.0f;
        };
        // Quad as two triangles
        pushV(ix0, iz0); pushV(ox0, oz0); pushV(ix1, iz1);
        pushV(ox0, oz0); pushV(ox1, oz1); pushV(ix1, iz1);
    }
}

void Zone::uploadRing() {
    glBindBuffer(GL_ARRAY_BUFFER, ringVBO_);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(ringBuf_.size() * sizeof(float)),
                 ringBuf_.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Zone::update(float dt) {
    elapsed_ += dt;
    if (paused_) return;

    if (phase_ >= kPhaseCount) return;

    const Phase& ph = kPhases[phase_];
    phaseTimer_ -= dt;

    if (!shrinking_) {
        if (phaseTimer_ <= 0.0f) {
            shrinking_   = true;
            phaseTimer_  = ph.shrinkTime;
            startRadius_ = radius_;
            startCenter_ = center_;
            std::cout << "[Zone] Zone shrinking! Phase " << (phase_ + 1)
                      << " → radius=" << ph.targetRadius << "\n";
        }
    } else {
        float dur = ph.shrinkTime;
        float t   = (dur - phaseTimer_) / dur;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        t = t * t * (3.0f - 2.0f * t);   // smooth-step

        radius_   = startRadius_   + (ph.targetRadius    - startRadius_)   * t;
        center_.x = startCenter_.x + (ph.targetCenter.x  - startCenter_.x) * t;
        center_.y = startCenter_.y + (ph.targetCenter.y  - startCenter_.y) * t;

        buildRingMesh();
        uploadRing();

        if (phaseTimer_ <= 0.0f) {
            radius_    = ph.targetRadius;
            center_    = ph.targetCenter;
            shrinking_ = false;
            ++phase_;
            if (phase_ < kPhaseCount) {
                phaseTimer_ = kPhases[phase_].waitTime;
                std::cout << "[Zone] Zone stable at radius=" << radius_
                          << ". Next shrink in " << phaseTimer_ << "s.\n";
            } else {
                phaseTimer_ = 0.0f;
                std::cout << "[Zone] Final circle! Damage=" << damagePerSec() << "/s\n";
            }
            buildRingMesh();
            uploadRing();
        }
    }
}

bool Zone::isInside(const glm::vec3& pos) const {
    float dx = pos.x - center_.x;
    float dz = pos.z - center_.y;
    return (dx * dx + dz * dz) <= radius_ * radius_;
}

float Zone::damagePerSec() const {
    if (phase_ >= kPhaseCount) return 15.0f;
    return kPhases[phase_].damagePerSec;
}

float Zone::phaseTimeFull() const {
    if (phase_ >= kPhaseCount) return 1.0f;
    return shrinking_ ? kPhases[phase_].shrinkTime : kPhases[phase_].waitTime;
}

void Zone::render(Shader& sh, const glm::vec3& lightDir, const glm::vec3& viewPos) {
    sh.use();
    sh.setVec3("uLightDir", lightDir);
    sh.setVec3("uViewPos",  viewPos);
    sh.setMat4("uModel",    glm::mat4(1.0f));

    // Pulse faster and brighter when the zone is actively shrinking.
    float speed = shrinking_ ? 6.0f : 1.8f;
    float pulse = 0.65f + 0.35f * sinf(elapsed_ * speed);
    sh.setVec3("uColor", glm::vec3(0.15f * pulse, 0.80f * pulse, 1.0f * pulse));
    sh.setFloat("uAlpha", 0.72f);

    glBindVertexArray(ringVAO_);
    glDrawArrays(GL_TRIANGLES, 0, ringVertCount_);
    glBindVertexArray(0);

    // Restore full opacity for subsequent world-shader draws.
    sh.setFloat("uAlpha", 1.0f);
}
