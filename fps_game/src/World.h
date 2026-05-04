#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

struct Box {
    glm::vec3 center;
    glm::vec3 size;
    glm::vec3 color;
};

struct Building {
    glm::vec3   center;       // world center (y = half height)
    glm::vec3   size;
    glm::vec3   wallColor;
    glm::vec3   roofColor;
    glm::vec3   trimColor;
    bool        doorOpen  = false;
    int         doorFace  = 0;    // 0=+Z  1=-Z  2=+X  3=-X
    const char* poiName   = nullptr;
    bool        isTerrain = false; // hill/mountain — simple rendering, full collision
};

struct MapPOI {
    const char* name;
    float wx, wz;
};

class World {
public:
    ~World();
    void init();
    void render(Shader& shader, const glm::vec3& viewPos);

    const std::vector<Box>&      boxes()     const { return boxes_; }
    const std::vector<Building>& buildings() const { return buildings_; }

    static constexpr int kPoiCount = 20;
    static const MapPOI  kPois[kPoiCount];

    // True if pos is inside a non-terrain closed building (used by camera).
    bool isInsideBuilding(const glm::vec3& pos) const;

    // True if pos is over an inland water zone (lake/river).
    bool isInlandWater   (const glm::vec3& pos) const;

    // Ray-AABB test against all obstacles (buildings + boxes).
    float raycast(const glm::vec3& origin, const glm::vec3& dir,
                  float maxDist, glm::vec3* hitPoint = nullptr) const;

    // Push player out of any closed building. Returns true if player landed on a roof.
    bool resolvePlayerBuilding(glm::vec3& pos, float radius, float height) const;

    // Toggle nearest door within 2.5 m. Returns building index or -1.
    int  tryToggleDoor(const glm::vec3& playerPos);

    // Returns building index if within door interaction range, else -1.
    int  nearDoor(const glm::vec3& playerPos) const;

    // World-space centre of the door on a given building.
    glm::vec3 doorCenter(const Building& b) const;

    void beginBoxRender();
    void endBoxRender();
    void drawBox(Shader& sh, const glm::vec3& center,
                 const glm::vec3& size, const glm::vec3& color);

private:
    GLuint cubeVAO_   = 0, cubeVBO_   = 0;
    GLuint groundVAO_ = 0, groundVBO_ = 0;
    GLuint waterVAO_  = 0, waterVBO_  = 0;
    int    waterVertCount_ = 6;

    std::vector<Box>      boxes_;       // trees, rocks, low walls
    std::vector<Building> buildings_;   // solid structures with doors

    void setupCube();
    void setupGround();
    void setupWater();
    void populate();

    void rb(Shader& sh, const glm::vec3& c,
            const glm::vec3& s, const glm::vec3& col) const;
    void renderBuilding(Shader& sh, const Building& b) const;
};
