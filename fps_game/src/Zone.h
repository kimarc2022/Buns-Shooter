#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

// Shrinking safe zone: renders a ring on the ground; damages anything outside.
class Zone {
public:
    ~Zone();
    void init();
    void reset();           // restart without re-allocating GL resources
    void update(float dt);
    void setPaused(bool p) { paused_ = p; }
    bool isPaused()  const { return paused_; }
    void render(Shader& sh, const glm::vec3& lightDir, const glm::vec3& viewPos);

    float     radius()       const { return radius_; }
    glm::vec2 center()       const { return center_; }
    float     damagePerSec() const;
    float     phaseTimeLeft() const { return phaseTimer_; }
    float     phaseTimeFull() const;
    bool      isShrinking()  const { return shrinking_; }
    int       phase()        const { return phase_; }
    bool      isInside(const glm::vec3& pos) const;

private:
    struct Phase {
        float waitTime;
        float shrinkTime;
        float targetRadius;
        glm::vec2 targetCenter;
        float damagePerSec;
    };

    static const int   kPhaseCount = 4;
    static const Phase kPhases[kPhaseCount];

    int       phase_       = 0;
    float     phaseTimer_  = 0.0f;
    bool      shrinking_   = false;

    float     radius_      = 95.0f;
    glm::vec2 center_      { 0.0f, 0.0f };
    float     startRadius_ = 95.0f;
    glm::vec2 startCenter_ { 0.0f, 0.0f };

    float elapsed_ = 0.0f;   // drives ring pulse animation
    bool  paused_  = false;  // zone frozen until player lands and delay expires

    GLuint             ringVAO_      = 0;
    GLuint             ringVBO_      = 0;
    std::vector<float> ringBuf_;
    int                ringVertCount_ = 0;

    void buildRingMesh();
    void uploadRing();
};
