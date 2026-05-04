#pragma once

#include <cstddef>

enum class Difficulty { Easy = 0, Medium, Hard, Insane, Extreme, Demon, COUNT };

// Per-difficulty tuning knobs for every AI behaviour.
struct DifficultyParams {
    const char* name;
    float accuracy;         // probability a shot hits the player  [0..1]
    float aimJitterDeg;     // random angle added to aim (degrees)
    float reactionSec;      // seconds before a bot "notices" a visible player
    float detectionRange;   // max sight distance (world units)
    float attackRange;      // switch to ATTACK when closer than this
    float moveSpeedMul;     // multiplier on base walk speed (6 m/s)
    float fireIntervalSec;  // minimum time between shots
    float fleeHealthPct;    // flee when health drops below this fraction
};

inline const DifficultyParams& getDifficulty(Difficulty d) {
    static const DifficultyParams kTable[static_cast<int>(Difficulty::COUNT)] = {
    //  name         acc    jitter  react  detect   atk    spd    fire   flee
        {"Easy",    0.14f, 15.0f,  1.60f,  24.0f, 12.0f, 0.58f, 1.70f, 0.62f},
        {"Medium",  0.24f, 11.0f,  1.10f,  34.0f, 16.0f, 0.70f, 1.25f, 0.48f},
        {"Hard",    0.34f,  8.0f,  0.75f,  46.0f, 19.0f, 0.80f, 0.95f, 0.34f},
        {"Insane",  0.44f,  5.8f,  0.52f,  56.0f, 22.0f, 0.90f, 0.74f, 0.22f},
        {"Extreme", 0.53f,  3.8f,  0.33f,  64.0f, 25.0f, 0.98f, 0.57f, 0.14f},
        {"Demon",   0.62f,  2.4f,  0.18f,  72.0f, 28.0f, 1.06f, 0.43f, 0.07f},
    };
    return kTable[static_cast<int>(d)];
}
