#include "Effects.h"

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

Effects::~Effects() {
    if (lineVBO_) glDeleteBuffers(1, &lineVBO_);
    if (lineVAO_) glDeleteVertexArrays(1, &lineVAO_);
}

bool Effects::init(const std::string& shaderDir) {
    if (!lineShader_.loadFromFiles(shaderDir + "/line.vert", shaderDir + "/line.frag"))
        return false;

    glGenVertexArrays(1, &lineVAO_);
    glGenBuffers(1, &lineVBO_);
    glBindVertexArray(lineVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO_);
    // Pre-allocate for up to 256 tracers (2 verts × 3 floats).
    glBufferData(GL_ARRAY_BUFFER, 256 * 2 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    return true;
}

// ── Spawn helpers ─────────────────────────────────────────────────────────────

void Effects::spawnDamageNum(const glm::vec3& worldPos, float dmg, bool headshot) {
    dmgNums_.push_back({worldPos, dmg, kDmgDur, kDmgDur, headshot});
}

void Effects::spawnTracer(const glm::vec3& start, const glm::vec3& end) {
    tracers_.push_back({start, end, kTracerDur, kTracerDur});
}

void Effects::triggerHitMarker()    { hitTimer_    = kHitDur;    }
void Effects::triggerDamageFlash()  { damageTimer_ = kDamageDur; }
void Effects::triggerMuzzleFlash()  { muzzleTimer_ = kMuzzleDur; }

void Effects::triggerKill() {
    killFeed_.push_back(kKillDur);
    if (killFeed_.size() > 5)
        killFeed_.erase(killFeed_.begin());
}

float Effects::killFeedAlpha(int i) const {
    if (i < 0 || i >= static_cast<int>(killFeed_.size())) return 0.0f;
    return std::min(killFeed_[i] / kKillDur, 1.0f);
}

// ── Update ────────────────────────────────────────────────────────────────────

void Effects::update(float dt) {
    hitTimer_    = std::max(0.0f, hitTimer_    - dt);
    damageTimer_ = std::max(0.0f, damageTimer_ - dt);
    muzzleTimer_ = std::max(0.0f, muzzleTimer_ - dt);

    for (auto& d : dmgNums_) d.life -= dt;
    dmgNums_.erase(std::remove_if(dmgNums_.begin(), dmgNums_.end(),
        [](const DamageNumber& d){ return d.life <= 0.0f; }), dmgNums_.end());

    for (auto& t : tracers_) t.life -= dt;
    tracers_.erase(std::remove_if(tracers_.begin(), tracers_.end(),
        [](const BulletTracer& t){ return t.life <= 0.0f; }), tracers_.end());

    for (auto& k : killFeed_) k -= dt;
    killFeed_.erase(std::remove_if(killFeed_.begin(), killFeed_.end(),
        [](float f){ return f <= 0.0f; }), killFeed_.end());
}

// ── Render tracers in 3-D ─────────────────────────────────────────────────────

void Effects::renderTracers(const glm::mat4& projView) {
    if (tracers_.empty()) return;

    // Build line vertex data (2 world-space points per tracer).
    std::vector<float> verts;
    verts.reserve(tracers_.size() * 6);
    for (const auto& t : tracers_) {
        verts.push_back(t.start.x); verts.push_back(t.start.y); verts.push_back(t.start.z);
        verts.push_back(t.end.x);   verts.push_back(t.end.y);   verts.push_back(t.end.z);
    }

    glBindVertexArray(lineVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO_);
    GLsizeiptr needed = static_cast<GLsizeiptr>(verts.size() * sizeof(float));
    glBufferSubData(GL_ARRAY_BUFFER, 0, needed, verts.data());

    lineShader_.use();
    lineShader_.setMat4("uVP", projView);
    lineShader_.setVec4("uColor", glm::vec4(1.0f, 0.92f, 0.45f, 0.88f));

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(tracers_.size() * 2));
    glLineWidth(1.0f);

    glBindVertexArray(0);
}
