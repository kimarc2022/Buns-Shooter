#include "Camera.h"
#include "Player.h"
#include "World.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

void Camera::processMouse(float dx, float dy, float sensitivity) {
    yaw   += dx * sensitivity;
    pitch -= dy * sensitivity;
    // Clamp pitch to avoid gimbal flip at the poles.
    pitch = std::clamp(pitch, -89.0f, 89.0f);
}

void Camera::toggleMode() {
    mode = (mode == CameraMode::FPS) ? CameraMode::TPS : CameraMode::FPS;
}

glm::vec3 Camera::forward() const {
    float cy = cosf(glm::radians(yaw));
    float sy = sinf(glm::radians(yaw));
    float cp = cosf(glm::radians(pitch));
    float sp = sinf(glm::radians(pitch));
    return glm::normalize(glm::vec3(cy * cp, sp, sy * cp));
}

glm::vec3 Camera::right() const {
    return glm::normalize(glm::cross(forward(), glm::vec3(0, 1, 0)));
}

glm::vec3 Camera::forwardFlat() const {
    glm::vec3 f = forward();
    f.y = 0.0f;
    return glm::normalize(f);
}

glm::vec3 Camera::rightFlat() const {
    return glm::normalize(glm::cross(forwardFlat(), glm::vec3(0, 1, 0)));
}

glm::vec3 Camera::position(const Player& p) const {
    if (mode == CameraMode::FPS) {
        return p.eyePos();
    }
    // TPS: over-right-shoulder.
    // Camera sits to the LEFT of the player so the character appears on the RIGHT of the frame.
    glm::vec3 anchor = p.position + glm::vec3(0.0f, 1.4f, 0.0f);
    return anchor - forward() * tpsDistance + right() * tpsRightOffset + glm::vec3(0.0f, tpsHeight, 0.0f);
}

glm::mat4 Camera::view(const Player& p) const {
    glm::vec3 eye = position(p);
    return glm::lookAt(eye, eye + forward(), glm::vec3(0, 1, 0));
}

glm::vec3 Camera::positionWithClip(const Player& p, const World& world) const {
    if (mode == CameraMode::FPS) return p.eyePos();

    glm::vec3 anchor = p.position + glm::vec3(0.0f, 1.4f, 0.0f);

    // If anchor is inside a building, stay at near-FPS to avoid looking through walls
    if (world.isInsideBuilding(anchor))
        return p.eyePos();

    glm::vec3 ideal     = position(p);
    glm::vec3 dir       = ideal - anchor;
    float     idealDist = glm::length(dir);
    if (idealDist < 0.001f) return ideal;

    glm::vec3 hitPt;
    float t = world.raycast(anchor, glm::normalize(dir), idealDist, &hitPt);
    if (t < idealDist) {
        float safeDist = std::max(0.2f, t - 0.3f);
        return anchor + glm::normalize(dir) * safeDist;
    }
    return ideal;
}

glm::mat4 Camera::viewClipped(const Player& p, const World& world) const {
    glm::vec3 eye = positionWithClip(p, world);
    return glm::lookAt(eye, eye + forward(), glm::vec3(0, 1, 0));
}

glm::mat4 Camera::projection(float aspect, float nearP, float farP) const {
    return glm::perspective(glm::radians(fov), aspect, nearP, farP);
}
