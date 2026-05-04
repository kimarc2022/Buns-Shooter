#pragma once
#include <glm/glm.hpp>

enum class WeaponType { Pistol = 0, Rifle, Shotgun, Sniper, SMG };
static constexpr int kWeaponCount = 5;

struct WeaponStats {
    const char* name;
    float       damage;          // per pellet / bullet
    float       fireInterval;    // seconds between shots
    int         pelletsPerShot;
    float       spreadRad;       // cone half-angle (radians)
    float       range;           // max hit distance (m)
    int         magSize;
    float       reloadTime;
    glm::vec3   hudColor;
};

inline const WeaponStats& getWeaponStats(WeaponType t) {
    static const WeaponStats kTable[kWeaponCount] = {
        {"Pistol",  42.0f, 0.28f, 1,  0.018f,  90.0f, 15, 1.0f, {0.80f,0.80f,0.80f}},
        {"Rifle",   22.0f, 0.10f, 1,  0.012f, 150.0f, 30, 2.0f, {0.25f,0.90f,0.25f}},
        {"Shotgun", 14.0f, 0.75f, 8,  0.11f,   30.0f,  8, 2.5f, {0.95f,0.50f,0.08f}},
        {"Sniper",  80.0f, 1.90f, 1,  0.002f, 400.0f,  5, 3.2f, {0.25f,0.65f,1.00f}},
        {"SMG",     10.0f, 0.09f, 1,  0.050f,  65.0f, 30, 2.0f, {1.00f,0.90f,0.15f}},
    };
    int idx = static_cast<int>(t);
    return kTable[(idx >= 0 && idx < kWeaponCount) ? idx : 0];
}

// ── Weapon variant names (12 per category) ────────────────────────────────────

inline const char* getVariantName(WeaponType t, int idx) {
    static const char* kPistol[12] = {
        "Falcon MK-II", "Desert Wolf", "Thunderbolt .45", "Wraith 9mm",
        "Shadow Revolver", "Phantom M18", "Iron Serpent", "Razorclaw Compact",
        "Coyote .357", "Nightfall Auto", "Tombstone Special", "Copper Viper"
    };
    static const char* kRifle[12] = {
        "Havoc AR-15", "Titan Assault", "Ironclad MK5", "Deadstorm M4",
        "Razorback Carbine", "Hellfire AR", "Vortex XM8", "Nemesis Assault",
        "Ghost Stalker", "Apex Hunter", "Reaper's Touch", "Storm Breaker"
    };
    static const char* kShotgun[12] = {
        "Judgement SG-7", "Hellmouth Pump", "Tombcleaver", "Razorblast DB",
        "Widow Maker", "Inferno Scatter", "Voidbore 12G", "Carnage Express",
        "Iron Thunder", "Deadfall DB", "Pestilence SG", "Bonecrusher"
    };
    static const char* kSniper[12] = {
        "Eclipse LR-11", "Phantom Eye", "Desolation .50", "Widowmaker SR",
        "Death Whisper", "Nemesis Long", "Void Seeker", "Celestial Mark",
        "Iron Silence", "Shadow Reach", "Reckoning SR", "Apex Predator"
    };
    static const char* kSMG[12] = {
        "Cyclone SMG", "Wasp Mini", "Razorwind 9", "Viper Compact",
        "Swarm MK3", "Phantom Rush", "Ghost Runner", "Tempest SMG",
        "Fury Compact", "Hellfire Burst", "Neon Strike", "Velocity X"
    };
    int i = (idx >= 0 && idx < 12) ? idx : 0;
    switch (t) {
        case WeaponType::Pistol:  return kPistol[i];
        case WeaponType::Rifle:   return kRifle[i];
        case WeaponType::Shotgun: return kShotgun[i];
        case WeaponType::Sniper:  return kSniper[i];
        case WeaponType::SMG:     return kSMG[i];
    }
    return kPistol[i];
}

inline WeaponStats getVariantStats(WeaponType t, int nameIdx) {
    WeaponStats s = getWeaponStats(t);
    s.damage       *= (0.85f + (nameIdx % 4) * 0.10f);
    s.fireInterval *= (0.90f + (nameIdx % 3) * 0.08f);
    return s;
}

// ─────────────────────────────────────────────────────────────────────────────

struct Weapon {
    WeaponType type        = WeaponType::Pistol;
    int        nameIdx     = 0;    // 0..11 variant index
    int        ammo        = 12;
    int        reserve     = 48;
    float      fireTimer   = 0.0f;
    float      reloadTimer = 0.0f;
    bool       reloading   = false;

    bool canFire() const { return !reloading && ammo > 0 && fireTimer <= 0.0f; }

    void startReload() {
        const auto& s = getWeaponStats(type);
        if (!reloading && reserve > 0 && ammo < s.magSize) {
            reloading   = true;
            reloadTimer = s.reloadTime;
        }
    }

    void update(float dt) {
        if (fireTimer > 0.0f) fireTimer -= dt;
        if (reloading) {
            reloadTimer -= dt;
            if (reloadTimer <= 0.0f) {
                const auto& s = getWeaponStats(type);
                int need = s.magSize - ammo;
                int take = need < reserve ? need : reserve;
                ammo    += take;
                reserve -= take;
                reloading = false;
            }
        }
    }

    void addAmmo(int n) {
        int cap = getWeaponStats(type).magSize * 6;
        reserve += n;
        if (reserve > cap) reserve = cap;
    }
};
