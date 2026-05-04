#include "BotManager.h"
#include "World.h"
#include "Player.h"
#include "Shader.h"
#include "Skin.h"
#include "DropManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

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

// ── Setup / teardown ─────────────────────────────────────────────────────────

BotManager::~BotManager() {
    if (cubeVBO_) glDeleteBuffers(1, &cubeVBO_);
    if (cubeVAO_) glDeleteVertexArrays(1, &cubeVAO_);
}

void BotManager::setupGL() {
    glGenVertexArrays(1, &cubeVAO_);
    glGenBuffers(1, &cubeVBO_);
    glBindVertexArray(cubeVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCube), kCube, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glBindVertexArray(0);
}

void BotManager::init(int count, const World& world) {
    setupGL();
    bots_.resize(count);
    for (int i = 0; i < count; ++i)
        bots_[i].skinIdx = i % kSkinCount;
    reset(world);
}

void BotManager::reset(const World& world) {
    (void)world;
    pendingDrops_.clear();
    std::uniform_real_distribution<float> xzD(-200.0f, 200.0f);
    std::uniform_real_distribution<float> waitD(0.0f, 3.0f);
    std::uniform_int_distribution<int>    weaponD(0, kWeaponCount - 1);
    std::uniform_int_distribution<int>    persD(0, 2);
    // Bus path: from (-350, 350, 0) to (350, 350, 0)
    std::uniform_real_distribution<float> busTD(0.05f, 0.95f);
    std::uniform_real_distribution<float> busOffD(-40.0f, 40.0f);
    std::uniform_real_distribution<float> fallVelD(-30.0f, -18.0f);

    for (auto& b : bots_) {
        // Drop from a random position along the bus path
        float t   = busTD(rng_);
        float busX = -350.0f + t * 700.0f;
        float offX = busOffD(rng_);
        float offZ = busOffD(rng_);
        b.position      = glm::vec3(busX + offX, 350.0f, offZ);
        b.velocity      = glm::vec3(
            std::uniform_real_distribution<float>(-5.0f, 5.0f)(rng_),
            fallVelD(rng_),
            std::uniform_real_distribution<float>(-5.0f, 5.0f)(rng_));
        b.facing        = glm::vec3(0.0f, 0.0f, 1.0f);
        b.health        = b.maxHealth;
        b.lives         = 1;
        b.onGround      = false;
        b.weaponType    = static_cast<WeaponType>(weaponD(rng_));
        b.medkits       = 1;
        b.state         = BotState::Falling;
        b.personality   = persD(rng_);
        b.stateTimer    = 0.0f;
        b.fireTimer     = waitD(rng_);
        b.reactionTimer = 0.0f;
        b.respawnTimer  = 0.0f;
        b.dropPos       = glm::vec3(xzD(rng_), 0.0f, xzD(rng_));
        b.patrolTarget  = b.dropPos;
        b.patrolWait    = waitD(rng_);
        b.lastPos       = b.position;
        b.stuckTimer    = 0.0f;
    }
}

int BotManager::aliveCount() const {
    int c = 0;
    for (const auto& b : bots_)
        c += !b.isEliminated() ? 1 : 0;  // falling bots still count — they haven't landed yet
    return c;
}

int BotManager::eliminatedCount() const {
    int c = 0;
    for (const auto& b : bots_)
        c += b.isEliminated() ? 1 : 0;
    return c;
}

// ── Public update ────────────────────────────────────────────────────────────

void BotManager::update(float dt, Player& player, const World& world, Difficulty diff,
                         glm::vec2 zoneCenter, float zoneRadius, float zoneDamagePerSec,
                         DropManager& drops) {
    zoneCenter_ = zoneCenter;
    zoneRadius_ = zoneRadius;
    zoneDamage_ = zoneDamagePerSec;

    const DifficultyParams& p = getDifficulty(diff);
    for (auto& bot : bots_) {
        if (bot.isEliminated()) continue;

        // Handle respawning state
        if (bot.state == BotState::Respawning) {
            bot.respawnTimer -= dt;
            if (bot.respawnTimer <= 0.0f) {
                std::uniform_real_distribution<float> xzD(-80.0f, 80.0f);
                float rx = xzD(rng_), rz = xzD(rng_);
                bot.position   = {rx, 120.0f, rz};  // drop from height
                bot.dropPos    = {rx, 0.0f, rz};
                bot.health     = bot.maxHealth;
                bot.velocity   = glm::vec3(0.0f, -25.0f, 0.0f);
                bot.onGround   = false;
                bot.state       = BotState::Falling;
                bot.stateTimer  = 0.0f;
                bot.reactionTimer= 0.0f;
                bot.enemyBotIdx = -1;
                bot.patrolTarget= bot.dropPos;
                bot.seekingLoot = false;
                bot.lootItemIdx = -1;
            }
            continue;
        }

        updateBot(bot, dt, player, world, p, drops);
    }
}

// POI positions for smarter patrol targets — one per named POI across entire map
static const glm::vec2 kPatrolPOIs[] = {
    {   1.0f,   -1.0f },   // Launch Site
    {   0.0f,  -58.0f },   // Pleasant Park
    {   0.0f,   56.0f },   // Dusty Depot
    {  55.0f,   -2.0f },   // Salty Springs
    { -56.0f,    3.0f },   // Tilted Towers
    {  55.0f,  -52.0f },   // Retail Row
    { -54.0f,   55.0f },   // Lonely Lodge
    { -52.0f,  -56.0f },   // Loot Lake
    {  56.0f,   56.0f },   // Fatal Fields
    {-140.0f, -110.0f },   // Haunted Hills
    { 140.0f,   50.0f },   // Neo City
    { 162.0f,    0.0f },   // Snobby Shores
    {   0.0f, -162.0f },   // Tomato Town
    {-162.0f,   80.0f },   // Wailing Woods
    { -80.0f,  162.0f },   // Greasy Grove
    { 140.0f, -132.0f },   // Moisty Mire
    {-162.0f,    5.0f },   // Lucky Landing
    {  30.0f,  178.0f },   // Paradise Palms
    { 157.0f,  -82.0f },   // Risky Reels
    { -92.0f, -162.0f },   // Shifty Shafts
};
static constexpr int kNumPatrolPOIs = 20;

// ── Per-bot update ───────────────────────────────────────────────────────────

void BotManager::updateBot(Bot& bot, float dt, Player& player,
                            const World& world, const DifficultyParams& p,
                            DropManager& drops) {
    // Falling state is handled entirely in the switch; skip zone/medkit logic
    if (bot.state == BotState::Falling) {
        // Just fall through to the switch below
        glm::vec3 toPlayer   = player.position - bot.position;
        float     dist       = glm::length(toPlayer);
        (void)dist;
        switch (bot.state) {
            case BotState::Falling: {
                bot.velocity.y -= 22.0f * dt;
                if (bot.velocity.y < -55.0f) bot.velocity.y = -55.0f;
                glm::vec3 toTarget = bot.dropPos - bot.position;
                toTarget.y = 0.0f;
                if (glm::length(toTarget) > 2.0f) {
                    glm::vec3 dir = glm::normalize(toTarget);
                    bot.velocity.x = dir.x * 8.0f;
                    bot.velocity.z = dir.z * 8.0f;
                }
                bot.position += bot.velocity * dt;
                if (bot.position.y <= 0.0f) {
                    bot.position.y = 0.0f;
                    bot.velocity   = glm::vec3(0.0f);
                    bot.onGround   = true;
                    bot.state      = BotState::Patrol;
                    bot.stateTimer = 0.0f;
                    bot.patrolTarget = randomPoint(bot.position, 30.0f);
                }
                return;
            }
            default: break;
        }
        return;
    }

    if (bot.medkits > 0 && bot.health < bot.maxHealth * 0.45f &&
        (bot.state == BotState::Flee || bot.state == BotState::Chase)) {
        bot.health = std::min(bot.health + 75.0f, bot.maxHealth);
        --bot.medkits;
    }

    // ── Loot seeking ──────────────────────────────────────────────────────────
    // Validate existing seek target (item may have been collected by someone else)
    if (bot.seekingLoot) {
        bool valid = bot.lootItemIdx >= 0 && bot.lootItemIdx < drops.itemCount();
        if (!valid) { bot.seekingLoot = false; bot.lootItemIdx = -1; }
    }
    // During patrol, scan for nearby items to pick up
    if (!bot.seekingLoot && bot.state == BotState::Patrol) {
        const float kLootRange = 18.0f;
        float bestDist = kLootRange;
        int   bestIdx  = -1;
        for (int ii = 0; ii < drops.itemCount(); ++ii) {
            float d = glm::length(drops.item(ii).pos - bot.position);
            if (d < bestDist) { bestDist = d; bestIdx = ii; }
        }
        if (bestIdx >= 0) {
            bot.seekingLoot = true;
            bot.lootItemIdx = bestIdx;
            bot.lootTarget  = drops.item(bestIdx).pos;
        }
    }
    // Move toward and collect loot
    if (bot.seekingLoot && bot.lootItemIdx >= 0 && bot.lootItemIdx < drops.itemCount()) {
        const DroppedItem& di = drops.item(bot.lootItemIdx);
        float d = glm::length(di.pos - bot.position);
        if (d < 1.8f) {
            if (di.hasWeapon) bot.weaponType = di.weaponType;
            drops.removeItem(bot.lootItemIdx);
            bot.seekingLoot = false;
            bot.lootItemIdx = -1;
        } else if (bot.state == BotState::Patrol) {
            bot.patrolTarget = bot.lootTarget;
            bot.patrolWait   = 0.0f;
        }
    }

    // Zone damage
    {
        glm::vec2 botXZ(bot.position.x, bot.position.z);
        float distToCenter = glm::length(botXZ - zoneCenter_);
        if (distToCenter > zoneRadius_) {
            bot.health -= zoneDamage_ * dt;
            if (bot.state == BotState::Patrol || bot.state == BotState::Flee) {
                glm::vec2 dir2 = distToCenter > 0.01f
                    ? glm::normalize(zoneCenter_ - botXZ)
                    : glm::vec2(1.0f, 0.0f);
                glm::vec2 target2 = zoneCenter_ + dir2 * (zoneRadius_ * 0.7f);
                bot.patrolTarget = glm::vec3(target2.x, 0.0f, target2.y);
                bot.patrolWait   = 0.0f;
            }
        }
    }

    if (!bot.onGround) bot.velocity.y -= 18.0f * dt;

    glm::vec3 toPlayer   = player.position - bot.position;
    float     dist       = glm::length(toPlayer);
    bool      inRange    = dist < p.detectionRange;
    bool      canSee     = inRange && hasLOS(bot.eyePos(), player.eyePos(), world);

    // ── Bot-vs-bot: find nearest visible enemy bot ────────────────────────────
    {
        float bestBotDist = p.detectionRange * 1.8f;
        int   bestIdx     = -1;
        for (int i = 0; i < (int)bots_.size(); ++i) {
            const Bot& other = bots_[i];
            if (&other == &bot || !other.isAlive()) continue;
            float d = glm::length(other.position - bot.position);
            if (d < bestBotDist && hasLOS(bot.eyePos(), other.eyePos(), world)) {
                bestBotDist = d;
                bestIdx = i;
            }
        }
        // Player takes priority only sometimes — bots fight each other aggressively
        if (!canSee && bestIdx >= 0) {
            bot.enemyBotIdx = bestIdx;
        } else if (canSee && bot.enemyBotIdx >= 0) {
            // Already targeting a bot — only switch to player 20% of the time
            if (std::uniform_real_distribution<float>(0,1)(rng_) < 0.20f)
                bot.enemyBotIdx = -1;
            // else keep targeting the bot
        } else if (canSee) {
            bot.enemyBotIdx = -1;  // no bot target, go for player
        } else {
            bot.enemyBotIdx = -1;
        }
    }

    // If targeting a bot, redirect AI toward that bot
    glm::vec3 effectiveTarget = toPlayer;
    float     effectiveDist   = dist;
    bool      effectiveSee    = canSee;
    if (bot.enemyBotIdx >= 0 && bot.enemyBotIdx < (int)bots_.size()) {
        const Bot& enemy = bots_[bot.enemyBotIdx];
        effectiveTarget  = enemy.position - bot.position;
        effectiveDist    = glm::length(effectiveTarget);
        effectiveSee     = hasLOS(bot.eyePos(), enemy.eyePos(), world);
        if (!enemy.isAlive()) {
            bot.enemyBotIdx = -1;
            effectiveTarget = toPlayer;
            effectiveDist   = dist;
            effectiveSee    = canSee;
        }
    }

    switch (bot.state) {
        case BotState::Patrol:    doPatrol(bot, dt, world, p, effectiveSee, effectiveDist);         break;
        case BotState::Chase:     doChase (bot, dt, world, p, effectiveTarget, effectiveSee, effectiveDist);break;
        case BotState::Attack:    doAttack(bot, dt, p, player, effectiveSee, effectiveDist);        break;
        case BotState::Flee:      doFlee  (bot, dt, world, p, -effectiveTarget, effectiveDist);      break;
        case BotState::Dead:
        case BotState::Respawning:
        case BotState::Falling:   return;
    }

    bot.position += bot.velocity * dt;

    if (bot.position.y <= 0.0f) {
        bot.position.y = 0.0f;
        bot.velocity.y = 0.0f;
        bot.onGround   = true;
    } else {
        bot.onGround = false;
    }

    resolveCollision(bot, world);

    float spd = glm::length(glm::vec2(bot.velocity.x, bot.velocity.z));
    if (bot.state != BotState::Attack) bot.walkPhase += spd * dt * 1.6f;

    bot.stuckTimer += dt;
    if (bot.stuckTimer > 1.5f) {
        float moved = glm::length(bot.position - bot.lastPos);
        if (moved < 0.5f && bot.state != BotState::Attack) {
            // Pick nearest POI to get unstuck faster
            float bestPoiDist = 1e9f;
            glm::vec2 bestPoi = {bot.position.x + 20.0f, bot.position.z};
            for (int pi = 0; pi < kNumPatrolPOIs; ++pi) {
                glm::vec2 poi = kPatrolPOIs[pi];
                float pd = glm::length(poi - glm::vec2(bot.position.x, bot.position.z));
                if (pd < bestPoiDist) { bestPoiDist = pd; bestPoi = poi; }
            }
            bot.patrolTarget = glm::vec3(bestPoi.x, 0.0f, bestPoi.y);
            if (bot.state == BotState::Chase || bot.state == BotState::Flee)
                bot.state = BotState::Patrol;
        }
        bot.lastPos    = bot.position;
        bot.stuckTimer = 0.0f;
    }

    // Kill check
    if (bot.health <= 0.0f) {
        bot.health   = 0.0f;
        bot.velocity = glm::vec3(0.0f);
        --bot.lives;
        if (bot.lives > 0) {
            // Respawn after delay
            bot.state        = BotState::Respawning;
            bot.respawnTimer = 6.0f;
        } else {
            // Permanently eliminated: drop loot
            bot.state = BotState::Dead;
            pendingDrops_.push_back({bot.position, bot.weaponType});
        }
    }
}

// ── State implementations ────────────────────────────────────────────────────

void BotManager::doPatrol(Bot& bot, float dt, const World& world,
                           const DifficultyParams& p, bool canSeePlayer, float) {
    // Immediately chase if we have a bot enemy target
    if (bot.enemyBotIdx >= 0) {
        float reactTime = p.reactionSec * 0.5f;  // react faster to bot enemies
        if (bot.personality == 0) reactTime *= 0.5f;
        bot.reactionTimer += dt;
        if (bot.reactionTimer >= reactTime) {
            bot.state = BotState::Chase; bot.stateTimer = bot.reactionTimer = 0.0f; return;
        }
    }

    if (canSeePlayer) {
        bot.reactionTimer += dt;
        float reactTime = p.reactionSec;
        // Personality: aggressive reacts faster, defensive slower
        if (bot.personality == 0) reactTime *= 0.6f;
        else if (bot.personality == 1) reactTime *= 1.3f;
        if (bot.reactionTimer >= reactTime) {
            bot.state = BotState::Chase; bot.stateTimer = bot.reactionTimer = 0.0f; return;
        }
    } else { bot.reactionTimer = 0.0f; }

    bot.patrolWait -= dt;
    glm::vec3 toTarget = bot.patrolTarget - bot.position;
    toTarget.y = 0.0f;
    float tDist = glm::length(toTarget);

    if (tDist < 2.0f || bot.patrolWait <= 0.0f) {
        // 70% chance to head toward a POI, 30% random — prefer closer POIs
        float poiRoll = std::uniform_real_distribution<float>(0,1)(rng_);
        if (poiRoll < 0.70f || bot.personality == 2) {
            // Pick among the 5 closest POIs, then choose one randomly
            struct PoiEntry { float dist; int idx; };
            PoiEntry closest[5];
            int found = 0;
            for (int pi = 0; pi < kNumPatrolPOIs; ++pi) {
                glm::vec2 poi = kPatrolPOIs[pi];
                float pd = glm::length(poi - glm::vec2(bot.position.x, bot.position.z));
                if (found < 5) {
                    closest[found++] = {pd, pi};
                } else {
                    // replace farthest if this is closer
                    int farthestJ = 0;
                    for (int j = 1; j < 5; ++j)
                        if (closest[j].dist > closest[farthestJ].dist) farthestJ = j;
                    if (pd < closest[farthestJ].dist)
                        closest[farthestJ] = {pd, pi};
                }
            }
            int pick = std::uniform_int_distribution<int>(0, found-1)(rng_);
            glm::vec2 poi = kPatrolPOIs[closest[pick].idx];
            float offX = std::uniform_real_distribution<float>(-8.0f, 8.0f)(rng_);
            float offZ = std::uniform_real_distribution<float>(-8.0f, 8.0f)(rng_);
            bot.patrolTarget = glm::vec3(poi.x + offX, 0.0f, poi.y + offZ);
        } else {
            bot.patrolTarget = randomPoint(bot.position, 35.0f + bot.personality * 8.0f);
        }
        bot.patrolWait   = std::uniform_real_distribution<float>(1.0f, 3.5f)(rng_);
        bot.velocity.x = bot.velocity.z = 0.0f;
        return;
    }

    float speed = 2.8f * p.moveSpeedMul;
    if (bot.personality == 0) speed *= 1.15f;  // aggressive patrols faster
    glm::vec3 dir = steer(bot, glm::normalize(toTarget), world);
    bot.velocity.x = dir.x * speed;
    bot.velocity.z = dir.z * speed;
    bot.facing     = dir;
}

void BotManager::doChase(Bot& bot, float dt, const World& world,
                          const DifficultyParams& p,
                          const glm::vec3& toPlayer, bool canSeePlayer, float dist) {
    if (bot.health / bot.maxHealth < p.fleeHealthPct) {
        bot.state = BotState::Flee; bot.stateTimer = 0.0f; return;
    }

    // When sight is lost, head to last-known position for longer before giving up
    if (!canSeePlayer) {
        bot.stateTimer += dt;
        float loseTime = (bot.personality == 2) ? 5.0f : 8.0f;  // sniper gives up sooner
        if (bot.stateTimer > loseTime) {
            bot.state = BotState::Patrol; bot.patrolTarget = randomPoint(bot.position,15.0f);
            bot.stateTimer = 0.0f; return;
        }
        // Head toward last-known position (patrolTarget stores it)
        glm::vec3 toLastKnown = bot.patrolTarget - bot.position;
        toLastKnown.y = 0.0f;
        float lkDist = glm::length(toLastKnown);
        if (lkDist > 1.5f) {
            float speed = 9.0f * p.moveSpeedMul;
            if (bot.personality == 0) speed *= 1.2f;
            glm::vec3 dir = steer(bot, glm::normalize(toLastKnown), world);
            bot.velocity.x = dir.x * speed;
            bot.velocity.z = dir.z * speed;
            bot.facing     = dir;
        }
        return;
    } else {
        bot.stateTimer = 0.0f;
        // Update last-known position while we can still see the target
        bot.patrolTarget = (bot.enemyBotIdx >= 0 && bot.enemyBotIdx < (int)bots_.size())
            ? bots_[bot.enemyBotIdx].position
            : (bot.position + toPlayer);
        bot.patrolTarget.y = 0.0f;
    }

    float attackRange = p.attackRange;
    if (bot.personality == 2) attackRange *= 1.6f;  // sniper engages from farther
    // Bot-vs-bot fights use a slightly larger attack range so bots spread out
    if (bot.enemyBotIdx >= 0) attackRange *= 1.15f;
    if (canSeePlayer && dist <= attackRange) {
        bot.state = BotState::Attack; bot.stateTimer = 0.0f;
        bot.fireTimer = p.fireIntervalSec * 0.3f; return;
    }

    float speed = 9.0f * p.moveSpeedMul;
    if (bot.personality == 0) speed *= 1.2f;   // aggressive runs faster
    else if (bot.personality == 2) speed *= 0.8f; // sniper approaches cautiously
    glm::vec3 flatDir = toPlayer; flatDir.y = 0.0f;
    if (glm::length(flatDir) > 0.01f) {
        flatDir = glm::normalize(flatDir);
        glm::vec3 side = glm::normalize(glm::cross(flatDir, glm::vec3(0,1,0)));
        // Flanking angle: 15-45 degrees off direct path
        float flankAmp = (bot.personality == 0) ? 0.85f : 0.55f;
        flatDir = glm::normalize(flatDir + side * sinf(bot.stateTimer * 2.2f) * flankAmp);
        flatDir = steer(bot, flatDir, world);
        bot.velocity.x = flatDir.x * speed;
        bot.velocity.z = flatDir.z * speed;
        bot.facing     = flatDir;
    }
}

void BotManager::doAttack(Bot& bot, float dt, const DifficultyParams& p,
                           Player& player, bool canSeePlayer, float dist) {
    if (bot.health / bot.maxHealth < p.fleeHealthPct) {
        bot.state = BotState::Flee; bot.stateTimer = 0.0f; return;
    }
    float attackRange = p.attackRange;
    if (bot.personality == 2) attackRange *= 1.6f;
    // Bot-vs-bot: use a larger attack range so bots spread out to fight
    if (bot.enemyBotIdx >= 0) attackRange *= 1.25f;
    if (!canSeePlayer || dist > attackRange * 1.2f) {
        bot.state = BotState::Chase; bot.stateTimer = 0.0f; return;
    }
    bot.stateTimer += dt;

    // Determine aim target position
    glm::vec3 targetPos = player.position;
    glm::vec3 targetVel = player.velocity;
    bool targetIsBot = false;
    if (bot.enemyBotIdx >= 0 && bot.enemyBotIdx < (int)bots_.size()) {
        const Bot& enemy = bots_[bot.enemyBotIdx];
        if (enemy.isAlive()) {
            targetPos   = enemy.position;
            targetVel   = enemy.velocity;
            targetIsBot = true;
        }
    }

    glm::vec3 toP = targetPos - bot.position; toP.y = 0.0f;
    if (glm::length(toP) > 0.01f) bot.facing = glm::normalize(toP);

    // Personality-based strafe behavior
    float strafeFreq = 2.2f + p.moveSpeedMul;
    float strafeAmp  = 3.0f + p.moveSpeedMul * 2.8f;
    if (bot.personality == 0) {
        // Aggressive: strafe more, occasionally jump
        strafeAmp  *= 1.3f;
        strafeFreq *= 1.4f;
        if (bot.onGround && std::uniform_real_distribution<float>(0,1)(rng_) < dt * 0.5f)
            bot.velocity.y = 6.0f;
    } else if (bot.personality == 2) {
        // Sniper: barely strafe, stay still to aim
        strafeAmp  *= 0.3f;
    }

    // Bots fighting other bots reposition occasionally for a better angle
    if (targetIsBot) {
        strafeAmp  *= 1.2f;
        strafeFreq *= 1.1f;
        // Every 3-5 seconds, dash to a flanking position
        float repositionInterval = std::uniform_real_distribution<float>(3.0f, 5.0f)(rng_);
        if (fmodf(bot.stateTimer, repositionInterval) < dt * 2.0f) {
            glm::vec3 side = glm::normalize(glm::cross(bot.facing, glm::vec3(0,1,0)));
            float flankSign = (std::uniform_real_distribution<float>(0,1)(rng_) < 0.5f) ? 1.0f : -1.0f;
            bot.velocity.x += side.x * flankSign * 6.0f;
            bot.velocity.z += side.z * flankSign * 6.0f;
        }
    }

    glm::vec3 right = glm::normalize(glm::cross(bot.facing, glm::vec3(0,1,0)));
    bot.velocity.x  = right.x * sinf(bot.stateTimer * strafeFreq) * strafeAmp;
    bot.velocity.z  = right.z * sinf(bot.stateTimer * strafeFreq) * strafeAmp;

    bot.fireTimer -= dt;
    // Bots fire at will on bot enemies — no standing-still requirement
    bool readyToFire = (bot.fireTimer <= 0.0f);
    if (bot.fireTimer <= 0.0f) {
        (void)readyToFire;
        float leadTime = dist / 38.0f;
        glm::vec3 leadPos = targetPos + targetVel * leadTime;
        glm::vec3 aimDir  = glm::normalize(leadPos - (bot.position + glm::vec3(0,1.0f,0)));

        // Inaccuracy increases with distance
        float distFactor = std::min(1.0f, dist / 40.0f);
        float jitterMul  = 1.0f + distFactor * 1.5f;
        if (bot.personality == 0) jitterMul *= 0.9f;  // aggressive is more accurate up close
        if (bot.personality == 2) jitterMul *= 0.6f;  // sniper is more accurate
        // Bots are slightly more accurate against other bots (larger targets, predictable movement)
        if (targetIsBot) jitterMul *= 0.85f;

        float jRad = glm::radians(p.aimJitterDeg * jitterMul);
        glm::vec3 u  = fabsf(aimDir.y) < 0.9f ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
        glm::vec3 r2 = glm::normalize(glm::cross(aimDir, u));
        glm::vec3 u2 = glm::cross(r2, aimDir);
        std::uniform_real_distribution<float> jD(-jRad, jRad);
        aimDir = glm::normalize(aimDir + r2*jD(rng_) + u2*jD(rng_));

        float roll = std::uniform_real_distribution<float>(0,1)(rng_);
        float shotDmg = getWeaponStats(bot.weaponType).damage;
        if (roll < p.accuracy) {
            if (targetIsBot) {
                // Shoot at enemy bot
                Bot& enemy = bots_[bot.enemyBotIdx];
                if (enemy.isAlive()) {
                    enemy.health -= shotDmg;
                    if (enemy.health <= 0.0f) {
                        enemy.health   = 0.0f;
                        enemy.velocity = glm::vec3(0.0f);
                        ++bot.kills;
                        // Reward: recover some health on kill
                        bot.health = std::min(bot.health + 28.0f, bot.maxHealth);
                        --enemy.lives;
                        if (enemy.lives > 0) {
                            enemy.state        = BotState::Respawning;
                            enemy.respawnTimer = 6.0f;
                        } else {
                            enemy.state = BotState::Dead;
                            pendingDrops_.push_back({enemy.position, enemy.weaponType});
                        }
                    }
                }
            } else if (player.health > 0.0f) {
                player.takeDamage(shotDmg);
            }
        }

        // Burst fire: aggressive fires 2-4 shots in quick succession
        // Bot-vs-bot fire is slightly faster to make fights more intense
        float burstInterval = p.fireIntervalSec;
        if (bot.personality == 0 && dist < 20.0f)
            burstInterval *= 0.65f;  // faster burst at close range
        if (targetIsBot)
            burstInterval *= 0.80f;  // faster fire rate on bot enemies
        bot.fireTimer = burstInterval + std::uniform_real_distribution<float>(0,0.08f)(rng_);
    }
}

void BotManager::doFlee(Bot& bot, float dt, const World& world,
                         const DifficultyParams& p,
                         const glm::vec3& awayFromPlayer, float distToPlayer) {
    (void)dt;
    if (distToPlayer > p.detectionRange * 1.6f) {
        bot.state = BotState::Patrol; bot.patrolTarget = randomPoint(bot.position, 18.0f); return;
    }

    float speed = 9.0f * p.moveSpeedMul;
    glm::vec3 threatDir = -awayFromPlayer; threatDir.y = 0.0f;
    glm::vec3 bestCover = awayFromPlayer; bestCover.y = 0.0f;
    float bestScore = -1e9f;

    for (const Box& box : world.boxes()) {
        glm::vec3 toBox = box.center - bot.position; toBox.y = 0.0f;
        float d = glm::length(toBox);
        if (d < 1.0f || d > 22.0f) continue;
        float dot   = glm::dot(glm::normalize(toBox), glm::normalize(-threatDir));
        float score = dot * 2.0f - d * 0.05f;
        if (score > bestScore) { bestScore = score; bestCover = glm::normalize(toBox * (d + box.size.x*0.5f + 1.5f)); }
    }

    glm::vec3 flat = bestCover;
    if (glm::length(flat) > 0.01f) {
        flat = steer(bot, glm::normalize(flat), world);
        bot.velocity.x = flat.x * speed;
        bot.velocity.z = flat.z * speed;
        bot.facing     = flat;
    }
}

// ── Player hitscan shot ───────────────────────────────────────────────────────

static float raySphere(const glm::vec3& o, const glm::vec3& d,
                        const glm::vec3& c, float r) {
    glm::vec3 oc = c - o;
    float tca = glm::dot(oc, d);
    if (tca < 0.0f) return 1e9f;
    float d2 = glm::dot(oc, oc) - tca*tca;
    if (d2 > r*r) return 1e9f;
    return tca - sqrtf(r*r - d2);
}

ShotResult BotManager::playerShot(const glm::vec3& origin, const glm::vec3& dir, float damage) {
    struct Zone { float dy; float r; bool isHead; };
    static constexpr Zone kZones[3] = {
        { 2.13f, 0.28f, true  },
        { 1.40f, 0.52f, false },
        { 0.55f, 0.45f, false },
    };

    float bestT  = 120.0f;
    Bot*  hitBot = nullptr;
    bool  hs     = false;

    for (auto& bot : bots_) {
        if (!bot.isAlive() || bot.state == BotState::Respawning) continue;
        for (const Zone& z : kZones) {
            float t = raySphere(origin, dir, bot.position + glm::vec3(0,z.dy,0), z.r);
            if (t < bestT) { bestT = t; hitBot = &bot; hs = z.isHead; }
        }
    }
    if (!hitBot) return {};

    float finalDmg = hs ? damage * 2.5f : damage;
    ShotResult res;
    res.hit        = true;
    res.hitPos     = origin + dir * bestT;
    res.damage     = finalDmg;
    res.headshot   = hs;
    res.dropWeapon = hitBot->weaponType;

    hitBot->health -= finalDmg;
    if (hitBot->health <= 0.0f) {
        hitBot->health   = 0.0f;
        hitBot->velocity = glm::vec3(0.0f);
        --hitBot->lives;
        if (hitBot->lives > 0) {
            hitBot->state        = BotState::Respawning;
            hitBot->respawnTimer = 6.0f;
        } else {
            hitBot->state = BotState::Dead;
            pendingDrops_.push_back({hitBot->position, hitBot->weaponType});
            res.killed = true;
        }
    }
    return res;
}

// ── Geometry helpers ─────────────────────────────────────────────────────────

static bool segVsAABB(glm::vec3 a, glm::vec3 b, glm::vec3 mn, glm::vec3 mx) {
    glm::vec3 d = b - a;
    float tMin = 0.0f, tMax = 1.0f;
    for (int i = 0; i < 3; ++i) {
        if (fabsf(d[i]) < 1e-7f) {
            if (a[i] < mn[i] || a[i] > mx[i]) return false;
        } else {
            float inv = 1.0f / d[i];
            float t1  = (mn[i] - a[i]) * inv, t2 = (mx[i] - a[i]) * inv;
            if (t1 > t2) std::swap(t1, t2);
            tMin = std::max(tMin, t1); tMax = std::min(tMax, t2);
            if (tMin > tMax) return false;
        }
    }
    return true;
}

bool BotManager::hasLOS(const glm::vec3& a, const glm::vec3& b, const World& world) const {
    for (const Box& box : world.boxes()) {
        if (segVsAABB(a, b, box.center-box.size*0.5f, box.center+box.size*0.5f)) return false;
    }
    return true;
}

void BotManager::resolveCollision(Bot& bot, const World& world) const {
    const float R = 0.45f;
    for (const Box& box : world.boxes()) {
        glm::vec3 mn = box.center - box.size*0.5f, mx = box.center + box.size*0.5f;
        if (bot.position.y + 1.8f < mn.y || bot.position.y > mx.y) continue;
        float px = bot.position.x, pz = bot.position.z;
        if (px+R < mn.x || px-R > mx.x || pz+R < mn.z || pz-R > mx.z) continue;
        float oL = (px+R)-mn.x, oR = mx.x-(px-R), oB = (pz+R)-mn.z, oF = mx.z-(pz-R);
        float minO = std::min({oL, oR, oB, oF});
        if      (minO==oL) { bot.position.x -= oL; bot.velocity.x = 0; }
        else if (minO==oR) { bot.position.x += oR; bot.velocity.x = 0; }
        else if (minO==oB) { bot.position.z -= oB; bot.velocity.z = 0; }
        else               { bot.position.z += oF; bot.velocity.z = 0; }
    }
}

glm::vec3 BotManager::steer(const Bot& bot, glm::vec3 desired, const World& world) const {
    const float probe = 3.0f, R = 0.55f;
    glm::vec3   tip   = bot.position + desired * probe;
    for (const Box& box : world.boxes()) {
        glm::vec3 mn = box.center-box.size*0.5f-glm::vec3(R), mx = box.center+box.size*0.5f+glm::vec3(R);
        if (tip.x<mn.x||tip.x>mx.x||tip.z<mn.z||tip.z>mx.z) continue;
        glm::vec3 sv(0.0f);
        float dL=fabsf(tip.x-mn.x),dR=fabsf(tip.x-mx.x),dB=fabsf(tip.z-mn.z),dF=fabsf(tip.z-mx.z);
        float mF = std::min({dL,dR,dB,dF});
        if      (mF==dL) sv={-1,0,0};
        else if (mF==dR) sv={ 1,0,0};
        else if (mF==dB) sv={ 0,0,-1};
        else             sv={ 0,0, 1};
        desired = glm::normalize(desired + sv * 1.8f);
        tip     = bot.position + desired * probe;
    }
    return desired;
}

glm::vec3 BotManager::randomPoint(const glm::vec3& near, float radius) {
    std::uniform_real_distribution<float> d(-radius, radius);
    glm::vec3 pt = near + glm::vec3(d(rng_), 0.0f, d(rng_));
    pt.y = 0.0f;
    glm::vec2 ptXZ(pt.x, pt.z);
    float maxDist = zoneRadius_ * 0.85f;
    float dist    = glm::length(ptXZ - zoneCenter_);
    if (dist > maxDist && dist > 0.01f) {
        ptXZ = zoneCenter_ + glm::normalize(ptXZ - zoneCenter_) * maxDist;
        pt.x = ptXZ.x; pt.z = ptXZ.y;
    }
    return pt;
}

// ── Render ───────────────────────────────────────────────────────────────────

void BotManager::renderBox(Shader& sh, const glm::vec3& center,
                            const glm::vec3& size, const glm::vec3& color) const {
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), center), size);
    sh.setMat4("uModel", m);
    sh.setVec3("uColor", color);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

// Rotated box (Y-axis rotation) for rounded chamfers and tilted limbs
static void renderBoxR(Shader& sh, const glm::vec3& center, const glm::vec3& size,
                        const glm::vec3& color, float yawDeg,
                        float xTiltDeg = 0.0f, float zTiltDeg = 0.0f) {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), center);
    if (yawDeg   != 0.0f) m = glm::rotate(m, glm::radians(yawDeg),   glm::vec3(0,1,0));
    if (xTiltDeg != 0.0f) m = glm::rotate(m, glm::radians(xTiltDeg), glm::vec3(1,0,0));
    if (zTiltDeg != 0.0f) m = glm::rotate(m, glm::radians(zTiltDeg), glm::vec3(0,0,1));
    m = glm::scale(m, size);
    sh.setMat4("uModel", m);
    sh.setVec3("uColor", color);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

// ── Significantly improved humanoid character ────────────────────────────────
// Each body part uses multiple boxes at slight angles to approximate roundness.
// Total: ~65 boxes per character.

void BotManager::renderHumanoid(Shader& sh, const glm::vec3& pos,
                                  const glm::vec3& facing, float walkPhase,
                                  const Skin& sk) const {
    glm::vec3 fwd = (glm::length(glm::vec2(facing.x, facing.z)) > 0.01f)
        ? glm::normalize(glm::vec3(facing.x, 0.0f, facing.z))
        : glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 rgt = glm::normalize(glm::cross(fwd, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up  = glm::vec3(0.0f, 1.0f, 0.0f);

    float sw  = sinf(walkPhase) * 0.24f;   // leg/arm swing
    float sw2 = sw * 0.55f;
    float swb = sinf(walkPhase + 0.3f) * 0.06f;  // body sway

    // Dark tone for shadows / details
    const glm::vec3 kDark {0.04f, 0.04f, 0.05f};
    // Slightly lighter skin highlight
    glm::vec3 skinHi  = glm::min(sk.skin  * 1.18f, glm::vec3(1.0f));
    glm::vec3 skinSh  = sk.skin  * 0.78f;
    glm::vec3 hairHi  = glm::min(sk.hair  * 1.15f, glm::vec3(1.0f));
    glm::vec3 torsoHi = glm::min(sk.torso * 1.12f, glm::vec3(1.0f));
    glm::vec3 torsoSh = sk.torso * 0.75f;
    glm::vec3 pantsHi = glm::min(sk.pants * 1.10f, glm::vec3(1.0f));
    glm::vec3 pantsSh = sk.pants * 0.72f;

    float yaw = glm::degrees(atan2f(fwd.x, fwd.z));

    // ── FEET / BOOTS ─────────────────────────────────────────────────────────
    // Soles
    renderBox(sh, pos - rgt*0.20f + glm::vec3(0,0.035f,0) + fwd*sw,   {0.26f,0.07f,0.32f}, kDark);
    renderBox(sh, pos + rgt*0.20f + glm::vec3(0,0.035f,0) - fwd*sw,   {0.26f,0.07f,0.32f}, kDark);
    // Boot body
    renderBox(sh, pos - rgt*0.20f + glm::vec3(0,0.115f,0) + fwd*sw,   {0.23f,0.16f,0.28f}, sk.boots);
    renderBox(sh, pos + rgt*0.20f + glm::vec3(0,0.115f,0) - fwd*sw,   {0.23f,0.16f,0.28f}, sk.boots);
    // Boot tongue/top
    renderBox(sh, pos - rgt*0.20f + glm::vec3(0,0.20f, 0) + fwd*(sw + 0.04f), {0.20f,0.06f,0.22f}, sk.boots*0.85f);
    renderBox(sh, pos + rgt*0.20f + glm::vec3(0,0.20f, 0) - fwd*(sw - 0.04f), {0.20f,0.06f,0.22f}, sk.boots*0.85f);
    // Lace detail
    renderBox(sh, pos - rgt*0.20f + fwd*(sw+0.14f) + glm::vec3(0,0.14f,0), {0.08f,0.09f,0.04f}, sk.accent);
    renderBox(sh, pos + rgt*0.20f - fwd*(sw-0.14f) + glm::vec3(0,0.14f,0), {0.08f,0.09f,0.04f}, sk.accent);

    // ── LOWER LEGS ───────────────────────────────────────────────────────────
    // Shin front
    renderBox(sh, pos - rgt*0.20f + glm::vec3(0,0.50f,0) + fwd*sw,    {0.20f,0.52f,0.20f}, sk.pants);
    renderBox(sh, pos + rgt*0.20f + glm::vec3(0,0.50f,0) - fwd*sw,    {0.20f,0.52f,0.20f}, sk.pants);
    // Shin side chamfer
    renderBoxR(sh, pos - rgt*0.20f + glm::vec3(0,0.50f,0)+fwd*sw, {0.06f,0.48f,0.20f}, pantsSh, yaw+30.0f);
    renderBoxR(sh, pos + rgt*0.20f + glm::vec3(0,0.50f,0)-fwd*sw, {0.06f,0.48f,0.20f}, pantsSh, yaw-30.0f);
    // Calf muscle (rear)
    renderBox(sh, pos - rgt*0.20f - fwd*0.05f + glm::vec3(0,0.44f,0)+fwd*sw, {0.18f,0.32f,0.06f}, pantsHi);
    renderBox(sh, pos + rgt*0.20f - fwd*0.05f + glm::vec3(0,0.44f,0)-fwd*sw, {0.18f,0.32f,0.06f}, pantsHi);

    // ── THIGHS ────────────────────────────────────────────────────────────────
    renderBox(sh, pos - rgt*0.21f + glm::vec3(0,0.91f,0) + fwd*sw2,   {0.24f,0.46f,0.24f}, pantsHi);
    renderBox(sh, pos + rgt*0.21f + glm::vec3(0,0.91f,0) - fwd*sw2,   {0.24f,0.46f,0.24f}, pantsHi);
    // Quad front
    renderBox(sh, pos - rgt*0.21f + fwd*0.08f + glm::vec3(0,0.91f,0)+fwd*sw2, {0.20f,0.42f,0.08f}, sk.pants);
    renderBox(sh, pos + rgt*0.21f + fwd*0.08f + glm::vec3(0,0.91f,0)-fwd*sw2, {0.20f,0.42f,0.08f}, sk.pants);
    // Inner seam
    renderBox(sh, pos - rgt*0.10f + glm::vec3(0,0.88f,0), {0.06f,0.42f,0.20f}, pantsSh);
    renderBox(sh, pos + rgt*0.10f + glm::vec3(0,0.88f,0), {0.06f,0.42f,0.20f}, pantsSh);

    // ── BELT / HIP ────────────────────────────────────────────────────────────
    renderBox(sh, pos + glm::vec3(0,1.17f,0) + rgt*swb, {0.78f,0.10f,0.50f}, sk.accent);
    renderBox(sh, pos + fwd*0.26f + glm::vec3(0,1.17f,0), {0.16f,0.10f,0.06f}, sk.accent*0.7f);  // belt buckle

    // ── TORSO ─────────────────────────────────────────────────────────────────
    // Core body — V-taper shape
    renderBox(sh, pos + glm::vec3(0,1.52f,0) + rgt*swb, {0.74f,0.62f,0.48f}, sk.torso);
    // Chest wider section
    renderBox(sh, pos + glm::vec3(0,1.72f,0) + rgt*swb, {0.76f,0.24f,0.46f}, torsoHi);
    // Front panel / cloth folds
    renderBox(sh, pos + fwd*0.24f + glm::vec3(0,1.55f,0), {0.38f,0.52f,0.08f}, sk.torso * 0.90f);
    // Side panels
    renderBox(sh, pos - rgt*0.36f + glm::vec3(0,1.52f,0), {0.08f,0.58f,0.42f}, torsoSh);
    renderBox(sh, pos + rgt*0.36f + glm::vec3(0,1.52f,0), {0.08f,0.58f,0.42f}, torsoSh);
    // Back muscle definition
    renderBox(sh, pos - fwd*0.24f + glm::vec3(0,1.62f,0), {0.54f,0.40f,0.08f}, torsoSh);
    // Collar / neck base
    renderBox(sh, pos + glm::vec3(0,1.88f,0), {0.38f,0.10f,0.32f}, sk.torso * 1.05f);

    // ── SHOULDER CAPS ────────────────────────────────────────────────────────
    renderBox(sh, pos - rgt*0.46f + glm::vec3(0,1.82f,0), {0.24f,0.18f,0.26f}, torsoHi);
    renderBox(sh, pos + rgt*0.46f + glm::vec3(0,1.82f,0), {0.24f,0.18f,0.26f}, torsoHi);
    // Shoulder round chamfers
    renderBoxR(sh, pos - rgt*0.44f + up*1.82f, {0.08f,0.14f,0.24f}, torsoHi, yaw+35.0f,  0.0f, -25.0f);
    renderBoxR(sh, pos + rgt*0.44f + up*1.82f, {0.08f,0.14f,0.24f}, torsoHi, yaw-35.0f,  0.0f,  25.0f);

    // ── UPPER ARMS ───────────────────────────────────────────────────────────
    // Bicep
    renderBox(sh, pos - rgt*0.56f + glm::vec3(0,1.54f,0) - fwd*sw*0.7f, {0.20f,0.42f,0.20f}, sk.torso);
    renderBox(sh, pos + rgt*0.56f + glm::vec3(0,1.54f,0) + fwd*sw*0.7f, {0.20f,0.42f,0.20f}, sk.torso);
    // Bicep peak
    renderBox(sh, pos - rgt*0.56f + fwd*0.07f + glm::vec3(0,1.57f,0)-fwd*sw*0.7f, {0.18f,0.28f,0.07f}, torsoHi);
    renderBox(sh, pos + rgt*0.56f + fwd*0.07f + glm::vec3(0,1.57f,0)+fwd*sw*0.7f, {0.18f,0.28f,0.07f}, torsoHi);
    // Tricep (rear)
    renderBox(sh, pos - rgt*0.56f - fwd*0.06f + glm::vec3(0,1.48f,0)-fwd*sw*0.7f, {0.18f,0.32f,0.06f}, torsoSh);
    renderBox(sh, pos + rgt*0.56f - fwd*0.06f + glm::vec3(0,1.48f,0)+fwd*sw*0.7f, {0.18f,0.32f,0.06f}, torsoSh);

    // ── FOREARMS ─────────────────────────────────────────────────────────────
    glm::vec3 faCol  = (sk.style == SkinStyle::Suit || sk.style == SkinStyle::Robot)
                       ? sk.accent : sk.torso * 0.88f;
    glm::vec3 faColR = sk.torso * 0.88f;
    renderBox(sh, pos - rgt*0.55f + glm::vec3(0,1.10f,0) - fwd*sw*0.7f, {0.18f,0.40f,0.18f}, faCol);
    renderBox(sh, pos + rgt*0.55f + glm::vec3(0,1.10f,0) + fwd*sw*0.7f, {0.18f,0.40f,0.18f}, faColR);
    // Wrist taper
    renderBox(sh, pos - rgt*0.55f + glm::vec3(0,0.88f,0) - fwd*sw*0.7f, {0.15f,0.14f,0.15f}, faCol*0.9f);
    renderBox(sh, pos + rgt*0.55f + glm::vec3(0,0.88f,0) + fwd*sw*0.7f, {0.15f,0.14f,0.15f}, faColR*0.9f);

    // ── HANDS ─────────────────────────────────────────────────────────────────
    renderBox(sh, pos - rgt*0.55f + glm::vec3(0,0.75f,0) - fwd*sw*0.7f, {0.18f,0.18f,0.16f}, sk.lhand);
    renderBox(sh, pos + rgt*0.55f + glm::vec3(0,0.75f,0) + fwd*sw*0.7f, {0.18f,0.18f,0.16f}, sk.skin);
    // Fingers (front of hand)
    renderBox(sh, pos - rgt*0.55f + fwd*0.09f + glm::vec3(0,0.72f,0)-fwd*sw*0.7f, {0.14f,0.12f,0.06f}, sk.lhand*0.92f);
    renderBox(sh, pos + rgt*0.55f + fwd*0.09f + glm::vec3(0,0.72f,0)+fwd*sw*0.7f, {0.14f,0.12f,0.06f}, sk.skin*0.92f);
    // Thumb nub
    renderBox(sh, pos - rgt*0.47f + glm::vec3(0,0.76f,0) - fwd*sw*0.7f, {0.06f,0.10f,0.08f}, sk.lhand*0.88f);
    renderBox(sh, pos + rgt*0.63f + glm::vec3(0,0.76f,0) + fwd*sw*0.7f, {0.06f,0.10f,0.08f}, sk.skin*0.88f);

    // ── NECK ─────────────────────────────────────────────────────────────────
    renderBox(sh, pos + glm::vec3(0,1.96f,0), {0.18f,0.18f,0.18f}, skinHi);
    // Neck sides
    renderBox(sh, pos - rgt*0.08f + glm::vec3(0,1.96f,0), {0.06f,0.16f,0.16f}, skinSh);
    renderBox(sh, pos + rgt*0.08f + glm::vec3(0,1.96f,0), {0.06f,0.16f,0.16f}, skinSh);

    // ── HEAD ─────────────────────────────────────────────────────────────────
    // Skull - main volume (slightly elongated)
    renderBox(sh, pos + glm::vec3(0,2.18f,0), {0.38f,0.42f,0.36f}, sk.skin);
    // Side chamfers (simulate roundness)
    renderBoxR(sh, pos - rgt*0.17f + glm::vec3(0,2.18f,0), {0.07f,0.36f,0.32f}, skinSh, yaw+45.0f);
    renderBoxR(sh, pos + rgt*0.17f + glm::vec3(0,2.18f,0), {0.07f,0.36f,0.32f}, skinSh, yaw-45.0f);
    // Forehead
    renderBox(sh, pos + fwd*0.14f + glm::vec3(0,2.34f,0), {0.34f,0.12f,0.14f}, skinHi);
    // Brow ridge
    renderBox(sh, pos + fwd*0.18f + glm::vec3(0,2.24f,0), {0.32f,0.07f,0.10f}, skinSh);
    // Cheekbones
    renderBox(sh, pos + fwd*0.16f - rgt*0.14f + glm::vec3(0,2.12f,0), {0.09f,0.08f,0.08f}, skinHi);
    renderBox(sh, pos + fwd*0.16f + rgt*0.14f + glm::vec3(0,2.12f,0), {0.09f,0.08f,0.08f}, skinHi);
    // Nose bridge
    renderBox(sh, pos + fwd*0.20f + glm::vec3(0,2.16f,0), {0.06f,0.12f,0.06f}, skinSh*0.95f);
    // Nose tip
    renderBox(sh, pos + fwd*0.22f + glm::vec3(0,2.08f,0), {0.08f,0.06f,0.08f}, skinSh);
    // Jaw / lower face
    renderBox(sh, pos + fwd*0.10f + glm::vec3(0,2.01f,0), {0.28f,0.10f,0.14f}, skinSh);
    // Ears
    renderBox(sh, pos - rgt*0.20f + glm::vec3(0,2.17f,0), {0.05f,0.10f,0.08f}, skinSh);
    renderBox(sh, pos + rgt*0.20f + glm::vec3(0,2.17f,0), {0.05f,0.10f,0.08f}, skinSh);
    // Chin
    renderBox(sh, pos + fwd*0.14f + glm::vec3(0,1.98f,0), {0.16f,0.07f,0.09f}, skinSh*0.9f);

    // ── EYES ─────────────────────────────────────────────────────────────────
    renderBox(sh, pos + fwd*0.21f - rgt*0.10f + glm::vec3(0,2.21f,0), {0.10f,0.068f,0.04f}, kDark);
    renderBox(sh, pos + fwd*0.21f + rgt*0.10f + glm::vec3(0,2.21f,0), {0.10f,0.068f,0.04f}, kDark);
    // Iris (slight colour)
    glm::vec3 irisCol = glm::mix(sk.detail, glm::vec3(0.15f,0.28f,0.55f), 0.5f);
    renderBox(sh, pos + fwd*0.225f - rgt*0.10f + glm::vec3(0,2.210f,0), {0.058f,0.050f,0.04f}, irisCol);
    renderBox(sh, pos + fwd*0.225f + rgt*0.10f + glm::vec3(0,2.210f,0), {0.058f,0.050f,0.04f}, irisCol);
    // Eyebrows
    renderBox(sh, pos + fwd*0.19f - rgt*0.10f + glm::vec3(0,2.27f,0), {0.12f,0.030f,0.05f}, sk.hair*0.8f);
    renderBox(sh, pos + fwd*0.19f + rgt*0.10f + glm::vec3(0,2.27f,0), {0.12f,0.030f,0.05f}, sk.hair*0.8f);
    // Mouth line
    renderBox(sh, pos + fwd*0.21f + glm::vec3(0,2.04f,0), {0.14f,0.025f,0.04f}, skinSh*0.7f);

    // ── HAIR ─────────────────────────────────────────────────────────────────
    // Crown
    renderBox(sh, pos + glm::vec3(0,2.47f,0),            {0.40f,0.15f,0.38f}, sk.hair);
    renderBox(sh, pos - fwd*0.06f + glm::vec3(0,2.40f,0),{0.38f,0.10f,0.16f}, sk.hair);
    // Side hair
    renderBox(sh, pos - rgt*0.20f + glm::vec3(0,2.30f,0),{0.05f,0.22f,0.32f}, hairHi);
    renderBox(sh, pos + rgt*0.20f + glm::vec3(0,2.30f,0),{0.05f,0.22f,0.32f}, hairHi);
    // Back hairline
    renderBox(sh, pos - fwd*0.18f + glm::vec3(0,2.24f,0),{0.34f,0.28f,0.06f}, sk.hair*0.88f);

    // ── STYLE-SPECIFIC ACCESSORIES ───────────────────────────────────────────
    switch (sk.style) {
    case SkinStyle::Suit:
        renderBox(sh, pos + fwd*0.26f + glm::vec3(0,1.40f,0), {0.06f,0.36f,0.04f}, sk.accent);
        renderBox(sh, pos + fwd*0.25f - rgt*0.20f + glm::vec3(0,1.62f,0), {0.10f,0.30f,0.05f}, torsoHi);
        renderBox(sh, pos + fwd*0.25f + rgt*0.20f + glm::vec3(0,1.62f,0), {0.10f,0.30f,0.05f}, torsoHi);
        break;
    case SkinStyle::Military:
        renderBox(sh, pos + glm::vec3(0,2.52f,0),              {0.46f,0.16f,0.46f}, sk.hair);
        renderBox(sh, pos + fwd*0.24f + glm::vec3(0,2.42f,0),  {0.30f,0.08f,0.06f}, sk.accent);
        renderBox(sh, pos - rgt*0.22f + fwd*0.12f + glm::vec3(0,0.72f,0), {0.24f,0.12f,0.08f}, sk.accent);
        renderBox(sh, pos + rgt*0.22f + fwd*0.12f + glm::vec3(0,0.72f,0), {0.24f,0.12f,0.08f}, sk.accent);
        break;
    case SkinStyle::SciFi:
        renderBox(sh, pos + fwd*0.24f + glm::vec3(0,2.18f,0),  {0.32f,0.07f,0.05f}, sk.accent);
        renderBox(sh, pos - rgt*0.55f + fwd*0.12f + glm::vec3(0,1.10f,0), {0.22f,0.12f,0.06f}, sk.accent);
        renderBox(sh, pos + rgt*0.55f + fwd*0.12f + glm::vec3(0,1.10f,0), {0.22f,0.12f,0.06f}, sk.accent);
        // Chest LED strip
        renderBox(sh, pos + fwd*0.26f + glm::vec3(0,1.66f,0),  {0.30f,0.05f,0.04f}, sk.accent * 1.3f);
        break;
    case SkinStyle::Casual:
        renderBox(sh, pos + glm::vec3(0,2.52f,0),              {0.40f,0.20f,0.40f}, sk.hair);
        renderBox(sh, pos + fwd*0.22f + glm::vec3(0,2.40f,0),  {0.38f,0.07f,0.18f}, sk.hair * 0.75f);
        break;
    case SkinStyle::Armor:
        renderBox(sh, pos - rgt*0.58f + glm::vec3(0,1.84f,0),  {0.28f,0.20f,0.28f}, torsoHi);
        renderBox(sh, pos + rgt*0.58f + glm::vec3(0,1.84f,0),  {0.28f,0.20f,0.28f}, torsoHi);
        renderBox(sh, pos - rgt*0.22f + fwd*0.14f + glm::vec3(0,0.73f,0), {0.26f,0.14f,0.10f}, torsoHi);
        renderBox(sh, pos + rgt*0.22f + fwd*0.14f + glm::vec3(0,0.73f,0), {0.26f,0.14f,0.10f}, torsoHi);
        // Chest plate
        renderBox(sh, pos + fwd*0.28f + glm::vec3(0,1.55f,0),  {0.44f,0.48f,0.08f}, torsoHi);
        break;
    case SkinStyle::Ninja:
        renderBox(sh, pos + fwd*0.22f + glm::vec3(0,2.07f,0),  {0.38f,0.09f,0.09f}, sk.torso);
        renderBox(sh, pos - fwd*0.28f + glm::vec3(0,1.62f,0),  {0.06f,0.22f,0.04f}, sk.accent);
        renderBox(sh, pos - fwd*0.36f + glm::vec3(0,2.18f,0),  {0.38f,0.24f,0.06f}, sk.hair * 0.8f);  // headband
        break;
    case SkinStyle::Cowboy:
        renderBox(sh, pos + glm::vec3(0,2.57f,0),              {0.34f,0.28f,0.34f}, sk.hair);
        renderBox(sh, pos + glm::vec3(0,2.42f,0),              {0.62f,0.05f,0.58f}, sk.hair * 0.82f);
        renderBox(sh, pos + fwd*0.22f + glm::vec3(0,2.06f,0),  {0.26f,0.08f,0.06f}, sk.accent);
        break;
    case SkinStyle::Robot:
        renderBox(sh, pos + fwd*0.24f + glm::vec3(0,2.18f,0),  {0.36f,0.09f,0.05f}, sk.accent);
        renderBox(sh, pos - rgt*0.10f + glm::vec3(0,2.58f,0),  {0.05f,0.20f,0.05f}, sk.accent);
        renderBox(sh, pos + fwd*0.28f + glm::vec3(0,1.66f,0),  {0.10f,0.08f,0.04f}, sk.accent * 1.4f);
        // Panel seam lines
        renderBox(sh, pos + fwd*0.27f + glm::vec3(0,1.52f,0),  {0.02f,0.52f,0.08f}, kDark);
        break;
    case SkinStyle::Medic:
        renderBox(sh, pos + fwd*0.28f + glm::vec3(0,1.58f,0),  {0.22f,0.06f,0.04f}, sk.detail);
        renderBox(sh, pos + fwd*0.28f + glm::vec3(0,1.58f,0),  {0.06f,0.22f,0.04f}, sk.detail);
        renderBox(sh, pos - rgt*0.44f + glm::vec3(0,1.22f,0),  {0.10f,0.16f,0.12f}, torsoHi);
        break;
    case SkinStyle::Raider:
        renderBox(sh, pos - rgt*0.60f + glm::vec3(0,1.90f,0),  {0.06f,0.18f,0.06f}, sk.accent);
        renderBox(sh, pos + rgt*0.60f + glm::vec3(0,1.90f,0),  {0.06f,0.18f,0.06f}, sk.accent);
        renderBox(sh, pos + fwd*0.22f + glm::vec3(0,2.20f,0),  {0.20f,0.07f,0.06f}, sk.accent * 0.55f);
        break;
    }
}

void BotManager::render(Shader& shader, const glm::vec3& lightDir, const glm::vec3& viewPos) {
    shader.use();
    shader.setVec3 ("uLightDir", lightDir);
    shader.setVec3 ("uViewPos",  viewPos);
    shader.setFloat("uAlpha",    1.0f);

    glBindVertexArray(cubeVAO_);

    for (const Bot& bot : bots_) {
        if (bot.isEliminated()) continue;

        if (bot.state == BotState::Dead || bot.state == BotState::Respawning) {
            if (bot.state == BotState::Dead) {
                // Fallen body flat on ground
                shader.setFloat("uAlpha", 0.7f);
                renderBox(shader, bot.position + glm::vec3(0,0.10f,0),
                          glm::vec3(1.2f, 0.22f, 0.55f), glm::vec3(0.18f,0.12f,0.12f));
                shader.setFloat("uAlpha", 1.0f);
            }
            continue;
        }

        renderHumanoid(shader, bot.position, bot.facing, bot.walkPhase,
                       kSkins[bot.skinIdx % kSkinCount]);
    }

    glBindVertexArray(0);
}

void BotManager::renderPlayerCharacter(Shader& sh, const glm::vec3& lightDir,
                                        const glm::vec3& viewPos,
                                        const glm::vec3& pos, const glm::vec3& facing,
                                        float walkPhase, int skinIdx) {
    sh.use();
    sh.setVec3 ("uLightDir", lightDir);
    sh.setVec3 ("uViewPos",  viewPos);
    sh.setFloat("uAlpha",    1.0f);
    glBindVertexArray(cubeVAO_);
    renderHumanoid(sh, pos, facing, walkPhase, kSkins[skinIdx % kSkinCount]);
    glBindVertexArray(0);
}
