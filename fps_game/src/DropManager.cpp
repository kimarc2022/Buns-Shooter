#include "DropManager.h"
#include "Shader.h"
#include "Font.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

static const float kCube[] = {
    -0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f, 0.5f,-0.5f, 0,0,-1,
     0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f,-0.5f,-0.5f, 0,0,-1,
    -0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f, 0.5f, 0.5f, 0,0, 1,
     0.5f, 0.5f, 0.5f, 0,0, 1, -0.5f, 0.5f, 0.5f, 0,0, 1, -0.5f,-0.5f, 0.5f, 0,0, 1,
    -0.5f, 0.5f, 0.5f,-1,0, 0, -0.5f, 0.5f,-0.5f,-1,0, 0, -0.5f,-0.5f,-0.5f,-1,0, 0,
    -0.5f,-0.5f,-0.5f,-1,0, 0, -0.5f,-0.5f, 0.5f,-1,0, 0, -0.5f, 0.5f, 0.5f,-1,0, 0,
     0.5f, 0.5f, 0.5f, 1,0, 0,  0.5f, 0.5f,-0.5f, 1,0, 0,  0.5f,-0.5f,-0.5f, 1,0, 0,
     0.5f,-0.5f,-0.5f, 1,0, 0,  0.5f,-0.5f, 0.5f, 1,0, 0,  0.5f, 0.5f, 0.5f, 1,0, 0,
    -0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f, 0.5f, 0,-1,0,
     0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f,-0.5f, 0,-1,0,
    -0.5f, 0.5f,-0.5f, 0, 1,0,  0.5f, 0.5f,-0.5f, 0, 1,0,  0.5f, 0.5f, 0.5f, 0, 1,0,
     0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f,-0.5f, 0, 1,0,
};

DropManager::~DropManager() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}

void DropManager::setupGL() {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCube), kCube, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glBindVertexArray(0);
}

void DropManager::init(unsigned seed) {
    rng_ = std::mt19937(seed);
    setupGL();
    reset();
}

void DropManager::reset() {
    items_.clear();
    chests_.clear();

    // ── Chest positions: mostly inside buildings ─────────────────────────────
    // Gold chests — one per building, plus a few outdoor spawns
    static const glm::vec3 kGoldChests[] = {
        // Launch Site
        {  1.0f, 0, -1.0f}, { 12.0f, 0,  2.5f}, { -8.0f, 0,  3.0f}, { -4.0f, 0, -8.0f},
        // Pleasant Park
        {-14.0f, 0,-52.0f}, {  4.0f, 0,-56.0f}, { 16.0f, 0,-50.0f},
        { -2.0f, 0,-67.0f}, { 10.0f, 0,-70.0f},
        // Dusty Depot
        { -7.0f, 0, 52.0f}, { 13.0f, 0, 58.0f}, {-14.0f, 0, 63.0f},
        // Salty Springs
        { 52.0f, 0, -6.0f}, { 61.0f, 0,  7.0f}, { 56.0f, 0,-18.0f},
        // Tilted Towers
        {-56.0f, 0,  0.0f}, {-60.0f, 0, 13.0f}, {-64.0f, 0,  6.0f},
        // Retail Row
        { 48.0f, 0,-50.0f}, { 57.0f, 0,-58.0f}, { 63.0f, 0,-47.0f}, { 52.0f, 0,-44.0f},
        // Lonely Lodge
        {-51.0f, 0, 52.0f}, {-60.0f, 0, 61.0f}, {-48.0f, 0, 59.0f},
        // Loot Lake
        {-48.0f, 0,-52.0f}, {-58.0f, 0,-60.0f},
        // Fatal Fields
        { 50.0f, 0, 52.0f}, { 60.0f, 0, 60.0f},
        // Mid-ring settlements
        { 95.0f, 0, 20.0f}, {-98.0f, 0,-18.0f}, { 20.0f, 0, 98.0f},
        {-90.0f, 0, 70.0f}, { 88.0f, 0,-72.0f}, {-95.0f, 0, -8.0f},
        // Haunted Hills
        {-140.0f,0,-110.0f},{-128.0f,0,-106.0f},{-152.0f,0,-122.0f},
        // Neo City
        { 140.0f,0,  50.0f},{ 128.0f,0,  44.0f},{ 153.0f,0,  61.0f},
        // Snobby Shores
        { 163.0f,0,   0.0f},{ 175.0f,0, -12.0f},{ 153.0f,0,  13.0f},
        // Tomato Town
        {   0.0f,0,-163.0f},{  14.0f,0,-172.0f},{ -12.0f,0,-153.0f},
        // Wailing Woods
        {-163.0f,0,  80.0f},{-151.0f,0,  92.0f},{-171.0f,0,  68.0f},
        // Greasy Grove
        { -80.0f,0, 163.0f},{ -68.0f,0, 152.0f},{ -93.0f,0, 173.0f},
        // Moisty Mire
        { 140.0f,0,-133.0f},{ 154.0f,0,-143.0f},{ 130.0f,0,-122.0f},
        // Lucky Landing
        {-163.0f,0,   5.0f},{-151.0f,0,  17.0f},{-173.0f,0,  -6.0f},
        // Paradise Palms
        {  30.0f,0, 178.0f},{  47.0f,0, 188.0f},{  18.0f,0, 168.0f},
        // Risky Reels
        { 157.0f,0, -83.0f},{ 168.0f,0, -71.0f},{ 145.0f,0, -93.0f},
        // Shifty Shafts
        { -92.0f,0,-163.0f},{ -79.0f,0,-172.0f},{-103.0f,0,-153.0f},
    };
    for (const auto& p : kGoldChests)
        chests_.push_back({p + glm::vec3(0, 0.4f, 0), false, false, 0.0f});

    // Blue (rare) chests — one per POI in the main building
    static const glm::vec3 kBlueChests[] = {
        {   0.5f, 0,  -1.5f},  // Launch Site
        {  -2.5f, 0, -67.5f},  // Pleasant Park
        {  -7.5f, 0,  52.5f},  // Dusty Depot
        {  52.5f, 0,  -6.5f},  // Salty Springs
        { -56.5f, 0,   0.5f},  // Tilted Towers
        {  52.5f, 0, -44.5f},  // Retail Row
        { -51.5f, 0,  52.5f},  // Lonely Lodge
        { -48.5f, 0, -52.5f},  // Loot Lake
        {  50.5f, 0,  52.5f},  // Fatal Fields
        {-140.5f, 0,-110.5f},  // Haunted Hills
        { 140.5f, 0,  50.5f},  // Neo City
        { 163.5f, 0,   0.5f},  // Snobby Shores
        {   0.5f, 0,-163.5f},  // Tomato Town
        {-163.5f, 0,  80.5f},  // Wailing Woods
        { -80.5f, 0, 163.5f},  // Greasy Grove
        { 140.5f, 0,-133.5f},  // Moisty Mire
        {-163.5f, 0,   5.5f},  // Lucky Landing
        {  30.5f, 0, 178.5f},  // Paradise Palms
        { 157.5f, 0, -83.5f},  // Risky Reels
        { -92.5f, 0,-163.5f},  // Shifty Shafts
    };
    for (const auto& p : kBlueChests)
        chests_.push_back({p + glm::vec3(0, 0.4f, 0), false, true, 0.0f});
}

void DropManager::spawnFloorLoot() {
    std::uniform_real_distribution<float> posD(-200.0f, 200.0f);
    std::uniform_int_distribution<int>    wepD(0, kWeaponCount - 1);
    std::uniform_int_distribution<int>    ammoD(8, 30);
    std::uniform_int_distribution<int>    nameD(0, 11);

    // Weapon drops
    for (int i = 0; i < 50; ++i) {
        DroppedItem it;
        it.pos       = {posD(rng_), 0.25f, posD(rng_)};
        it.hasWeapon = true;
        it.weaponType= static_cast<WeaponType>(wepD(rng_));
        it.nameIdx   = nameD(rng_);
        const auto& ws = getWeaponStats(it.weaponType);
        it.ammo      = ws.magSize;
        it.reserve   = ammoD(rng_) * 2;
        it.wood      = std::uniform_int_distribution<int>(5, 25)(rng_);
        it.bobTimer  = std::uniform_real_distribution<float>(0, 6.28f)(rng_);
        items_.push_back(it);
    }

    // Ammo packs spread densely across map — auto-collected on proximity
    std::uniform_int_distribution<int> reserveD(30, 90);
    for (int wt = 0; wt < kWeaponCount; ++wt) {
        for (int j = 0; j < 20; ++j) {
            DroppedItem it;
            it.pos        = {posD(rng_), 0.25f, posD(rng_)};
            it.hasWeapon  = false;
            it.weaponType = static_cast<WeaponType>(wt);
            it.reserve    = reserveD(rng_);
            it.bobTimer   = std::uniform_real_distribution<float>(0, 6.28f)(rng_);
            items_.push_back(it);
        }
    }
}

void DropManager::spawnFromBot(const glm::vec3& pos, WeaponType wt) {
    std::uniform_int_distribution<int>   matD(15, 40);
    std::uniform_int_distribution<int>   ammoD(5, 20);
    std::uniform_int_distribution<int>   nameD(0, 11);

    DroppedItem it;
    it.pos       = pos + glm::vec3(0, 0.25f, 0);
    it.hasWeapon = true;
    it.weaponType= wt;
    it.nameIdx   = nameD(rng_);
    const auto& ws = getWeaponStats(wt);
    it.ammo      = ws.magSize;
    it.reserve   = ammoD(rng_) * 3;
    it.wood      = matD(rng_);
    it.brick     = matD(rng_) / 2;
    it.metal     = matD(rng_) / 2;
    it.bobTimer  = std::uniform_real_distribution<float>(0, 6.28f)(rng_);
    items_.push_back(it);
}

void DropManager::spawnFromChest(const glm::vec3& pos, bool isBlue) {
    std::uniform_int_distribution<int> wepD(0, kWeaponCount - 1);
    std::uniform_int_distribution<int> matD(isBlue ? 40 : 20, isBlue ? 80 : 45);
    std::uniform_int_distribution<int> nameD(0, 11);

    // Weapon
    {
        DroppedItem it;
        it.pos       = pos + glm::vec3(0, 0.3f, 0);
        it.hasWeapon = true;
        // Blue chests bias toward better weapons
        if (isBlue) {
            int roll = std::uniform_int_distribution<int>(0, 2)(rng_);
            it.weaponType = (roll == 0) ? WeaponType::Sniper
                          : (roll == 1) ? WeaponType::Rifle
                          :               WeaponType::SMG;
        } else {
            it.weaponType = static_cast<WeaponType>(wepD(rng_));
        }
        it.nameIdx = nameD(rng_);
        const auto& ws = getWeaponStats(it.weaponType);
        it.ammo    = ws.magSize;
        it.reserve = ws.magSize * (isBlue ? 6 : 4);
        it.wood    = matD(rng_);
        it.brick   = matD(rng_);
        it.metal   = matD(rng_);
        it.fromBlue= isBlue;
        it.bobTimer= std::uniform_real_distribution<float>(0, 6.28f)(rng_);
        items_.push_back(it);
    }
    // Extra ammo pack
    {
        DroppedItem it2;
        it2.pos    = pos + glm::vec3(0.6f, 0.25f, 0.3f);
        it2.hasWeapon = false;
        it2.wood   = matD(rng_);
        it2.brick  = matD(rng_);
        it2.metal  = matD(rng_);
        it2.bobTimer= std::uniform_real_distribution<float>(0, 6.28f)(rng_);
        items_.push_back(it2);
    }
}

void DropManager::spawnDroppedItem(const DroppedItem& di) {
    items_.push_back(di);
}

void DropManager::update(float dt) {
    for (auto& it : items_) it.bobTimer += dt * 2.2f;
    for (auto& ch : chests_) ch.glowPhase += dt * 2.8f;
}

void DropManager::renderBox(Shader& sh, const glm::vec3& c, const glm::vec3& s, const glm::vec3& col) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), c);
    m = glm::scale(m, s);
    sh.setMat4("uModel", m);
    sh.setVec3("uColor", col);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void DropManager::render(Shader& sh, const glm::vec3& lightDir, const glm::vec3& eye) {
    sh.use();
    sh.setVec3("uLightDir", lightDir);
    sh.setVec3("uViewPos",  eye);

    glBindVertexArray(vao_);

    // ── Render chests ─────────────────────────────────────────────────────────
    for (const auto& ch : chests_) {
        if (ch.opened) continue;
        float glow = 0.5f + 0.5f * sinf(ch.glowPhase);
        glm::vec3 base = ch.isBlue
            ? glm::vec3(0.10f + glow*0.10f, 0.55f + glow*0.20f, 1.00f)
            : glm::vec3(0.90f + glow*0.08f, 0.72f + glow*0.12f, 0.05f);
        glm::vec3 dark = base * 0.55f;
        glm::vec3 p    = ch.pos;

        sh.setFloat("uAlpha", 1.0f);
        // Main body
        renderBox(sh, p,                          {0.72f,0.52f,0.56f}, base);
        // Lid
        renderBox(sh, p + glm::vec3(0,0.35f,0),  {0.72f,0.22f,0.56f}, dark);
        // Lock clasp
        renderBox(sh, p + glm::vec3(0,0.12f,0.29f), {0.14f,0.16f,0.08f}, dark * 1.4f);
        // Trim lines
        renderBox(sh, p + glm::vec3(0,0.24f,0),  {0.72f,0.04f,0.56f}, dark * 0.6f);
        // Glow ring at base
        sh.setFloat("uAlpha", 0.55f * glow);
        renderBox(sh, p + glm::vec3(0,-0.28f,0), {0.90f,0.05f,0.72f}, base * 1.2f);
        sh.setFloat("uAlpha", 1.0f);
    }

    // ── Render dropped items ──────────────────────────────────────────────────
    for (const auto& it : items_) {
        float bob = sinf(it.bobTimer) * 0.06f;
        glm::vec3 p = it.pos + glm::vec3(0, bob, 0);

        if (it.hasWeapon) {
            static const glm::vec3 kWepCol[5] = {
                {0.65f,0.65f,0.68f},  // Pistol
                {0.25f,0.70f,0.28f},  // Rifle
                {0.78f,0.50f,0.22f},  // Shotgun
                {0.22f,0.48f,0.88f},  // Sniper
                {0.65f,0.25f,0.78f},  // SMG
            };
            glm::vec3 wc = kWepCol[static_cast<int>(it.weaponType)];
            float glow  = it.fromBlue ? (0.7f + 0.3f*sinf(it.bobTimer*1.5f)) : 1.0f;
            sh.setFloat("uAlpha", 1.0f);
            // Gun body
            renderBox(sh, p,                         {0.55f, 0.18f, 0.22f}, wc * glow);
            // Barrel
            renderBox(sh, p + glm::vec3(0.28f,0,0),  {0.20f, 0.08f, 0.09f}, wc * 0.7f);
            // Stock
            renderBox(sh, p + glm::vec3(-0.22f,-0.04f,0), {0.14f, 0.22f, 0.10f}, wc * 0.85f);
            // Glow aura under rare items
            if (it.fromBlue) {
                sh.setFloat("uAlpha", 0.30f * glow);
                renderBox(sh, p + glm::vec3(0,-0.15f,0), {0.70f,0.04f,0.70f}, {0.20f,0.60f,1.0f});
                sh.setFloat("uAlpha", 1.0f);
            }
        } else {
            // Material/ammo pack — small glowing crate
            float g = 0.6f + 0.4f * sinf(it.bobTimer);
            sh.setFloat("uAlpha", 1.0f);
            renderBox(sh, p, {0.35f, 0.35f, 0.35f}, {0.88f * g, 0.72f * g, 0.25f * g});
            renderBox(sh, p + glm::vec3(0, 0.18f,0), {0.36f,0.04f,0.36f}, {1,1,1,});
        }
    }

    glBindVertexArray(0);
}

void DropManager::renderLabels(const glm::vec3& playerPos, const glm::mat4& projView,
                                Font& font, int width, int height) {
    const float kPickupDist = 3.5f;
    const float CWS = 0.019f, CHS = 0.034f;
    const float CWX = 0.015f, CHX = 0.027f;

    static const char* kWepNames[5] = {"PISTOL","RIFLE","SHOTGUN","SNIPER","SMG"};

    for (int i = 0; i < (int)items_.size(); ++i) {
        const DroppedItem& it = items_[i];
        float dist = glm::length(it.pos - playerPos);

        // Show label if within render range
        if (dist > 18.0f) continue;

        float bob = sinf(it.bobTimer) * 0.06f;
        glm::vec3 labelWorld = it.pos + glm::vec3(0, 0.8f + bob, 0);
        glm::vec4 clip = projView * glm::vec4(labelWorld, 1.0f);
        if (clip.w < 0.01f) continue;
        float nx = clip.x / clip.w, ny = clip.y / clip.w;
        if (fabsf(nx) > 1.1f || fabsf(ny) > 1.1f) continue;

        float alpha = dist < 10.0f ? 1.0f : 1.0f - (dist - 10.0f) / 8.0f;

        // Weapon name
        if (it.hasWeapon) {
            const char* wn = kWepNames[static_cast<int>(it.weaponType)];
            static const glm::vec4 kWepColor[5] = {
                {0.85f,0.85f,0.88f,1}, {0.35f,0.95f,0.35f,1},
                {0.95f,0.65f,0.25f,1}, {0.35f,0.65f,1.00f,1}, {0.85f,0.45f,1.00f,1}
            };
            glm::vec4 col = kWepColor[static_cast<int>(it.weaponType)];
            col.a = alpha;
            float tw = font.textWidth(wn, CWS);
            font.drawShadowed(wn, nx - tw*0.5f, ny + 0.02f, CWS, CHS, col);

            // Ammo count
            std::string ammoStr = std::to_string(it.ammo) + "+" + std::to_string(it.reserve);
            float aw = font.textWidth(ammoStr, CWX);
            font.drawShadowed(ammoStr, nx - aw*0.5f, ny - CHX, CWX, CHX, {1,1,0.7f,alpha*0.85f});
        } else {
            std::string nameStr;
            if (it.reserve > 0) {
                nameStr = std::string(kWepNames[static_cast<int>(it.weaponType)]) + " AMMO";
            } else {
                nameStr = "LOOT";
            }
            font.drawShadowed(nameStr.c_str(), nx - font.textWidth(nameStr.c_str(),CWS)*0.5f, ny+0.02f,
                              CWS, CHS, {1.0f,0.88f,0.25f,alpha});
        }

        // Pickup hint when close
        if (dist < kPickupDist) {
            const std::string hint = "[E] PICK UP";
            float pulse = 0.7f + 0.3f * sinf(it.bobTimer * 3.0f);
            font.drawShadowed(hint, nx - font.textWidth(hint,CWX)*0.5f, ny - CHX*2,
                              CWX, CHX, {0.25f, 1.0f, 0.45f, alpha * pulse});
        }
    }

    // Chest labels
    for (int i = 0; i < (int)chests_.size(); ++i) {
        const Chest& ch = chests_[i];
        if (ch.opened) continue;
        float dist = glm::length(ch.pos - playerPos);
        if (dist > 6.0f) continue;

        glm::vec3 lw = ch.pos + glm::vec3(0, 1.0f, 0);
        glm::vec4 clip = projView * glm::vec4(lw, 1.0f);
        if (clip.w < 0.01f) continue;
        float nx = clip.x / clip.w, ny = clip.y / clip.w;
        if (fabsf(nx) > 1.1f || fabsf(ny) > 1.1f) continue;

        const char* ctype = ch.isBlue ? "SUPPLY CRATE" : "CHEST";
        glm::vec4 cc = ch.isBlue ? glm::vec4(0.25f,0.80f,1.0f,1.0f) : glm::vec4(1.0f,0.85f,0.1f,1.0f);
        font.drawShadowed(ctype, nx - font.textWidth(ctype,CWS)*0.5f, ny+0.02f, CWS, CHS, cc);

        float pulse = 0.7f + 0.3f * sinf(ch.glowPhase);
        const std::string hint = "[E] OPEN";
        font.drawShadowed(hint, nx - font.textWidth(hint,CWX)*0.5f, ny - CHX*2,
                          CWX, CHX, {0.25f, 1.0f, 0.45f, pulse});
    }
    (void)width; (void)height;
}

int DropManager::tryPickup(const glm::vec3& playerPos) {
    const float kDist = 2.5f;
    for (int i = 0; i < (int)items_.size(); ++i) {
        if (glm::length(items_[i].pos - playerPos) <= kDist) {
            // Remove item from list and return index
            // Caller reads item(i) BEFORE calling this, so we return index
            // and caller removes after handling
            return i;
        }
    }
    return -1;
}

void DropManager::removeItem(int i) {
    if (i < 0 || i >= (int)items_.size()) return;
    items_[i] = items_.back();
    items_.pop_back();
}

int DropManager::tryOpenChest(const glm::vec3& playerPos) {
    const float kDist = 2.8f;
    for (int i = 0; i < (int)chests_.size(); ++i) {
        if (!chests_[i].opened && glm::length(chests_[i].pos - playerPos) <= kDist) {
            chests_[i].opened = true;
            return i;
        }
    }
    return -1;
}
