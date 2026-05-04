#pragma once

#include <glm/glm.hpp>
#include "Weapon.h"

enum class BotState { Patrol, Chase, Attack, Flee, Dead, Respawning, Falling };

struct Bot {
    // --- world state ---
    glm::vec3 position {0.0f};
    glm::vec3 velocity {0.0f};
    glm::vec3 facing   {0.0f, 0.0f, 1.0f};
    glm::vec3 color    {1.0f};

    float health    = 200.0f;
    float maxHealth = 200.0f;
    int   lives     = 3;           // total respawns remaining
    bool  onGround  = true;

    float respawnTimer = 0.0f;     // counts down when Respawning

    WeaponType weaponType = WeaponType::Pistol;
    int        medkits    = 1;

    // --- AI state machine ---
    BotState state        = BotState::Patrol;
    float    stateTimer   = 0.0f;
    float    fireTimer    = 0.0f;
    float    reactionTimer= 0.0f;

    // --- patrol sub-state ---
    glm::vec3 patrolTarget {0.0f};
    float     patrolWait  = 0.0f;

    // --- bus drop state ---
    glm::vec3 dropPos     {0.0f};  // target XZ landing area
    int       personality = 0;     // 0=aggressive 1=defensive 2=sniper

    // --- bot-vs-bot targeting ---
    int       enemyBotIdx  = -1;   // index into BotManager::bots_; -1 = target player

    // --- loot seeking ---
    int       lootItemIdx  = -1;
    bool      seekingLoot  = false;
    glm::vec3 lootTarget   {0.0f};

    // --- arena stats ---
    int  kills = 0;    // cumulative kills this arena session

    // --- stuck detection ---
    glm::vec3 lastPos   {0.0f};
    float     stuckTimer= 0.0f;

    // --- animation ---
    float walkPhase = 0.0f;

    // --- appearance ---
    int skinIdx = 0;

    // alive = has lives left and not waiting to respawn into fight
    bool isAlive()   const { return state != BotState::Dead && state != BotState::Falling; }
    bool isFalling() const { return state == BotState::Falling; }
    bool isEliminated() const { return state == BotState::Dead && lives <= 0; }

    glm::vec3 eyePos()    const { return position + glm::vec3(0, 1.70f, 0); }
    glm::vec3 bodyCenter()const { return position + glm::vec3(0, 0.90f, 0); }
};
