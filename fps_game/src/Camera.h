#pragma once

#include <glm/glm.hpp>

class Player;
class World;

enum class CameraMode { FPS, TPS };

// Free-look camera that follows a Player. Yaw/pitch driven by mouse input.
//   FPS: positioned at the player's eyes, looks along the camera forward.
//   TPS: orbits behind & above the player, looking back at the player.
class Camera {
public:
    CameraMode mode = CameraMode::FPS;

    float yaw   = -90.0f;   // looking down -Z
    float pitch =   0.0f;
    float fov   =  70.0f;

    float tpsDistance    = 5.0f;
    float tpsHeight      = 0.60f;
    float tpsRightOffset = 1.20f;

    void processMouse(float dx, float dy, float sensitivity = 0.12f);
    void toggleMode();

    // Camera basis vectors derived from yaw/pitch.
    glm::vec3 forward() const;
    glm::vec3 right()   const;

    // Forward projected onto the XZ plane (used to drive player movement).
    glm::vec3 forwardFlat() const;
    glm::vec3 rightFlat()   const;

    glm::vec3 position(const Player& p) const;
    glm::vec3 positionWithClip(const Player& p, const World& world) const;  // TPS clip vs buildings
    glm::mat4 view    (const Player& p) const;
    glm::mat4 viewClipped(const Player& p, const World& world) const;
    glm::mat4 projection(float aspect, float nearP = 0.05f, float farP = 500.0f) const;
};
