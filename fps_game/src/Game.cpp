#include "Game.h"
#include "SaveData.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

#ifndef SHADER_DIR
#define SHADER_DIR "shaders"
#endif

static const glm::vec3 kLightDir = glm::normalize(glm::vec3(-0.4f, -1.0f, -0.3f));

// ── Arena constants ────────────────────────────────────────────────────────────
static const glm::vec3 kArenaSpawns[8] = {
    {  0.0f, 0.0f, -20.0f},   // N  (player default)
    { 14.1f, 0.0f, -14.1f},   // NE
    { 20.0f, 0.0f,   0.0f},   // E
    { 14.1f, 0.0f,  14.1f},   // SE
    {  0.0f, 0.0f,  20.0f},   // S
    {-14.1f, 0.0f,  14.1f},   // SW
    {-20.0f, 0.0f,   0.0f},   // W
    {-14.1f, 0.0f, -14.1f},   // NW
};
static const glm::vec3 kArenaWeaponSpawns[8] = {
    {  8.0f, 0.3f,  0.0f},
    { -8.0f, 0.3f,  0.0f},
    {  0.0f, 0.3f,  8.0f},
    {  0.0f, 0.3f, -8.0f},
    {  5.6f, 0.3f,  5.6f},
    { -5.6f, 0.3f,  5.6f},
    {  5.6f, 0.3f, -5.6f},
    { -5.6f, 0.3f, -5.6f},
};

// ── Glider skin system ────────────────────────────────────────────────────────

struct GliderSkin {
    std::string name;
    glm::vec3   canopy;
    glm::vec3   stripe;
    glm::vec3   frame;
};

static std::vector<GliderSkin> buildGliderList() {
    static const char* kStyles[10] = {
        "VIPER","STORM","NOVA","PULSE","APEX",
        "NEXUS","ECHO","DRIFT","RAZOR","FLUX"
    };
    struct Hue { const char* name; glm::vec3 canopy, stripe, frame; };
    static const Hue kHues[15] = {
        {"CRIMSON",  {0.85f,0.12f,0.15f}, {1.00f,0.45f,0.10f}, {0.25f,0.05f,0.05f}},
        {"COBALT",   {0.10f,0.28f,0.90f}, {0.30f,0.72f,1.00f}, {0.05f,0.08f,0.30f}},
        {"JADE",     {0.10f,0.72f,0.35f}, {0.55f,1.00f,0.45f}, {0.03f,0.22f,0.12f}},
        {"GOLD",     {0.95f,0.78f,0.05f}, {1.00f,0.95f,0.50f}, {0.30f,0.22f,0.00f}},
        {"VIOLET",   {0.55f,0.10f,0.90f}, {0.88f,0.45f,1.00f}, {0.18f,0.03f,0.30f}},
        {"EMBER",    {0.95f,0.40f,0.05f}, {1.00f,0.70f,0.25f}, {0.30f,0.12f,0.00f}},
        {"CYAN",     {0.05f,0.82f,0.92f}, {0.40f,1.00f,1.00f}, {0.00f,0.25f,0.30f}},
        {"OBSIDIAN", {0.08f,0.08f,0.10f}, {0.40f,0.42f,0.50f}, {0.02f,0.02f,0.04f}},
        {"ROSE",     {0.95f,0.38f,0.62f}, {1.00f,0.72f,0.85f}, {0.30f,0.10f,0.18f}},
        {"ARCTIC",   {0.80f,0.92f,1.00f}, {1.00f,1.00f,1.00f}, {0.30f,0.40f,0.50f}},
        {"DUSK",     {0.45f,0.22f,0.55f}, {0.80f,0.50f,0.92f}, {0.15f,0.06f,0.18f}},
        {"COPPER",   {0.78f,0.48f,0.22f}, {1.00f,0.72f,0.40f}, {0.24f,0.14f,0.04f}},
        {"LIME",     {0.52f,0.92f,0.10f}, {0.80f,1.00f,0.45f}, {0.15f,0.28f,0.02f}},
        {"IVORY",    {0.95f,0.92f,0.82f}, {1.00f,1.00f,0.95f}, {0.35f,0.32f,0.25f}},
        {"ONYX",     {0.15f,0.15f,0.18f}, {0.55f,0.55f,0.60f}, {0.05f,0.05f,0.06f}},
    };
    std::vector<GliderSkin> list;
    list.reserve(150);
    for (int h = 0; h < 15; ++h)
        for (int s = 0; s < 10; ++s)
            list.push_back({std::string(kHues[h].name) + " " + kStyles[s],
                            kHues[h].canopy, kHues[h].stripe, kHues[h].frame});
    return list;
}

static const std::vector<GliderSkin>& getGliders() {
    static auto s = buildGliderList();
    return s;
}

// ── GLFW callbacks ────────────────────────────────────────────────────────────

static void cb_glfw_error(int code, const char* desc) {
    std::cerr << "[GLFW error " << code << "] " << desc << "\n";
}

static Game* gActive = nullptr;
static void cb_mouse    (GLFWwindow*, double x, double y)             { if(gActive) gActive->onMouseMove(x,y);         }
static void cb_key      (GLFWwindow*, int k, int, int action, int)    { if(gActive) gActive->onKey(k, action);        }
static void cb_mouse_btn(GLFWwindow*, int btn, int action, int)       { if(gActive) gActive->onMouseButton(btn,action);}
static void cb_resize   (GLFWwindow*, int w, int h)                   { if(gActive) gActive->onResize(w, h);          }

// ── Init / shutdown ───────────────────────────────────────────────────────────

bool Game::init(int w, int h, const char* title) {
    width_  = w;
    height_ = h;

    glfwSetErrorCallback(cb_glfw_error);
    if (!glfwInit()) { std::cerr << "glfwInit failed\n"; return false; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_SAMPLES, 0);

    window_ = glfwCreateWindow(width_, height_, title, nullptr, nullptr);
    if (!window_) { std::cerr << "glfwCreateWindow failed\n"; glfwTerminate(); return false; }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (GLenum e = glewInit(); e != GLEW_OK) {
        std::cerr << "glewInit: " << glewGetErrorString(e) << "\n";
        return false;
    }

    gActive = this;
    glfwSetCursorPosCallback      (window_, cb_mouse);
    glfwSetKeyCallback            (window_, cb_key);
    glfwSetMouseButtonCallback    (window_, cb_mouse_btn);
    glfwSetFramebufferSizeCallback(window_, cb_resize);
    // Game starts in Menu — use normal cursor so user can click
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    cursorLocked_ = false;
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.55f, 0.75f, 0.95f, 1.0f);

    const std::string sd = SHADER_DIR;
    if (!worldShader_.loadFromFiles(sd + "/world.vert", sd + "/world.frag")) return false;
    if (!uiShader_  .loadFromFiles (sd + "/ui.vert",    sd + "/ui.frag"))    return false;
    if (!font_      .init(sd))                                                return false;
    if (!effects_   .init(sd))                                                return false;

    world_.init();
    ui_.init();
    zone_.init();
    botManager_.init(29, world_);
    dropManager_.init(42u);
    pickupManager_.init(1337u);

    player_.position = glm::vec3(0.0f);
    player_.health   = 100.0f;
    player_.shield   = 100.0f;
    player_.initWeapons();

    camera_.fov = settings_.fov;

    // Load persistent save data
    if (!loadGame(saveData_)) {
        saveData_.ownedSkins = {0};
        saveData_.selectedSkin = 0;
    }
    playerSkinIdx_ = saveData_.selectedSkin;
    gliderIdx_     = saveData_.gliderIdx;
    lockerSel_     = 0;   // will show first owned skin

    return true;
}

void Game::shutdown() {
    if (window_) { glfwDestroyWindow(window_); window_ = nullptr; }
    glfwTerminate();
    gActive = nullptr;
}

// ── Respawn helpers ───────────────────────────────────────────────────────────

void Game::respawnPlayer() {
    player_.health          = 100.0f;
    player_.shield          = 100.0f;
    // NOTE: do NOT reset position/velocity here — player lands wherever they drop
    player_.initWeapons();   // clears weapons, activates pickaxe, zeroes materials
    player_.medkits         = 0;
    player_.bandages        = 0;
    player_.shieldPotions   = 0;
    player_.miniShields     = 0;
    player_.dashCooldown    = 0.0f;
    player_.dashing         = false;
    player_.grappleActive   = false;
    player_.grappleCooldown = 0.0f;
    playerDead_             = false;
    victoryAchieved_        = false;
    victoryTimer_           = 0.0f;
    respawnTimer_           = 0.0f;
}

// respawnViaBus removed — 1 life only

void Game::startBusFlight(bool isRespawn) {
    state_          = GameState::BusFlight;
    busT_           = 0.0f;
    busPhase_       = 0;
    busIsRespawn_   = isRespawn;
    gliderDeployAnim_ = 0.0f;

    // Random route: start on one edge of the land, fly to the opposite edge
    float angle = std::uniform_real_distribution<float>(0.0f, 6.28318f)(rng_);
    const float kEdge = 230.0f;
    const float kBusY = 120.0f;
    busStart_ = glm::vec3(-cosf(angle) * kEdge, kBusY, -sinf(angle) * kEdge);
    busEnd_   = glm::vec3( cosf(angle) * kEdge, kBusY,  sinf(angle) * kEdge);

    player_.position  = busStart_ + glm::vec3(0.0f, -1.5f, 0.0f);
    player_.velocity  = glm::vec3(0.0f);
    player_.onGround  = false;
    // Face the direction of travel
    camera_.yaw       = glm::degrees(atan2f(busEnd_.z - busStart_.z,
                                             busEnd_.x - busStart_.x)) - 90.0f;
    camera_.pitch     = -10.0f;
    firstMouse_       = true;
}

void Game::adjustSetting(int delta) {
    switch (settingIdx_) {
    case 0: settings_.sensitivity = std::clamp(settings_.sensitivity + delta * 0.01f, 0.02f, 0.50f); break;
    case 1: settings_.fov = std::clamp(settings_.fov + delta * 5.0f, 50.0f, 110.0f); camera_.fov = settings_.fov; break;
    case 2: if (delta != 0) settings_.showFPS = !settings_.showFPS; break;
    }
}

// ── GLFW callbacks ────────────────────────────────────────────────────────────

void Game::onMouseMove(double x, double y) {
    // Always track NDC position for menu hover/click
    menuMouseNX_ = float(x) / float(std::max(width_,  1)) * 2.0f - 1.0f;
    menuMouseNY_ = 1.0f - float(y) / float(std::max(height_, 1)) * 2.0f;

    if (state_ == GameState::Menu) return;   // don't rotate camera in menu

    if (firstMouse_) { lastX_ = x; lastY_ = y; firstMouse_ = false; return; }
    camera_.processMouse(float(x - lastX_), float(y - lastY_), settings_.sensitivity);
    lastX_ = x; lastY_ = y;
}

void Game::onKey(int key, int action) {
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
        if (state_ == GameState::Menu && menuPage_ == MenuPage::Settings)
            menuPage_ = MenuPage::Main;
        else if (state_ == GameState::Menu && menuPage_ == MenuPage::Difficulty)
            menuPage_ = MenuPage::Main;
        else if (state_ == GameState::Menu && menuPage_ == MenuPage::Locker)
            menuPage_ = MenuPage::Main;
        else if (state_ == GameState::Menu && menuPage_ == MenuPage::Shop)
            menuPage_ = MenuPage::Main;
        else if (state_ == GameState::Menu && menuPage_ == MenuPage::Main)
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
        else if (state_ == GameState::Playing && isArena_) {
            awardAndSave();
            state_   = GameState::Menu;
            menuPage_= MenuPage::Main;
            isArena_ = false;
        }
        else
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_ENTER && state_ == GameState::Menu) {
        if (menuPage_ == MenuPage::Main)
            startBusFlight(false);
    }
}

void Game::onResize(int w, int h) {
    width_ = w;  height_ = h;
    glViewport(0, 0, w, h);
}

// ── Mouse button (menu only) ──────────────────────────────────────────────────

void Game::onMouseButton(int button, int action) {
    if (state_ != GameState::Menu) return;
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;

    float mx = menuMouseNX_, my = menuMouseNY_;
    // Returns true if (mx,my) is inside box (x0, top-y, width, height)
    auto hit = [&](float x0, float topY, float w, float h) {
        return mx >= x0 && mx <= x0+w && my <= topY && my >= topY-h;
    };

    // ── Top tab bar ───────────────────────────────────────────────────────────
    const float TX = -0.30f, TW_TAB = 0.15f, TGAP = 0.018f;
    static const MenuPage kTabPages[4] = {
        MenuPage::Main, MenuPage::Locker, MenuPage::Shop, MenuPage::Settings };
    for (int i = 0; i < 4; ++i) {
        float tabX = TX + i*(TW_TAB+TGAP);
        if (hit(tabX-0.01f, 1.0f, TW_TAB+0.02f, 0.155f)) {
            menuPage_ = kTabPages[i];
            return;
        }
    }

    // ── Difficulty / Settings: back button ───────────────────────────────────
    if (menuPage_ == MenuPage::Difficulty || menuPage_ == MenuPage::Settings) {
        if (hit(-0.20f, -0.42f, 0.40f, 0.08f)) { menuPage_ = MenuPage::Main; return; }
    }
    if (menuPage_ == MenuPage::Difficulty) {
        const float rowY0=0.42f, rowGap=0.20f;
        const float CHN=0.046f;
        for (int i=0;i<6;++i) {
            float ry = rowY0 - i*rowGap;
            if (hit(-0.67f, ry+CHN*0.6f, 1.34f, CHN+0.018f)) {
                settingIdx_ = i;
                difficulty_ = static_cast<Difficulty>(i);
                menuPage_ = MenuPage::Main;
                return;
            }
        }
    }

    // ── PLAY page ─────────────────────────────────────────────────────────────
    if (menuPage_ == MenuPage::Main) {
        // PLAY button
        if (hit(-0.30f, -0.80f, 0.60f, 0.16f)) { startBusFlight(false); return; }
        // Arena button
        if (hit(-0.55f, -0.72f, 1.10f, 0.055f)) { startArena(); return; }
        // Nav arrows for skin selection
        if (hit(-0.96f, -0.64f, 0.12f, 0.10f)) {  // left arrow
            playerSkinIdx_ = std::max(0, playerSkinIdx_-1);
            saveData_.selectedSkin = playerSkinIdx_;
            saveGame(saveData_);
            return;
        }
        if (hit(-0.18f, -0.64f, 0.12f, 0.10f)) {  // right arrow
            playerSkinIdx_ = std::min(kSkinCount-1, playerSkinIdx_+1);
            saveData_.selectedSkin = playerSkinIdx_;
            saveGame(saveData_);
            return;
        }
    }

    // ── Locker page ───────────────────────────────────────────────────────────
    if (menuPage_ == MenuPage::Locker && !lockerGlider_) {
        const int kCols = 4;
        const float TW_SK=0.185f, TH_SK=0.180f, TGAP_SK=0.016f;
        const float GX0=-0.02f, GY0=0.72f;
        int owned = (int)saveData_.ownedSkins.size();
        for (int i=0; i<owned; ++i) {
            int col=i%kCols, row=i/kCols;
            float sx=GX0+col*(TW_SK+TGAP_SK);
            float sy=GY0-0.08f-row*(TH_SK+TGAP_SK);
            if (hit(sx, sy, TW_SK, TH_SK)) {
                lockerSel_ = i;
                playerSkinIdx_ = saveData_.ownedSkins[i];
                saveData_.selectedSkin = playerSkinIdx_;
                saveGame(saveData_);
                return;
            }
        }
        // [G] gliders button region
        if (hit(-0.02f, 0.68f, 0.28f, 0.04f)) { lockerGlider_ = true; return; }
    }
    if (menuPage_ == MenuPage::Locker && lockerGlider_) {
        // Glider grid cells
        const float SW=0.115f, SH=0.095f, GAPX=0.010f, GAPY=0.010f;
        const float SX=SW+GAPX, SY=SH+GAPY;
        for (int i=0; i<150; ++i) {
            int col=i%15, row=i/15;
            float sx=-0.90f+col*SX, sy=0.76f-row*SY;
            if (hit(sx, sy, SW, SH)) { gliderIdx_ = i; saveData_.gliderIdx=i; saveGame(saveData_); return; }
        }
        // Back button
        if (hit(-0.20f, -0.87f, 0.40f, 0.06f)) { lockerGlider_ = false; return; }
    }

    // ── Shop page ─────────────────────────────────────────────────────────────
    if (menuPage_ == MenuPage::Shop) {
        std::vector<int> forSale;
        for (int i=0; i<kSkinCount; ++i)
            if (!saveData_.ownsSkin(i)) forSale.push_back(i);
        const int total = (int)forSale.size();
        const int kCols=4, kRows=3;
        const float TW_SH=0.220f, TH_SH=0.200f, TGAP_SH=0.014f;
        const float GX0=0.02f, GY0=0.72f;
        for (int vi=0; vi<kRows; ++vi) {
            int globalRow=shopScroll_+vi;
            for (int col=0; col<kCols; ++col) {
                int idx=globalRow*kCols+col;
                if (idx>=total) break;
                float sx=GX0+col*(TW_SH+TGAP_SH);
                float sy=GY0-vi*(TH_SH+TGAP_SH);
                if (hit(sx, sy, TW_SH, TH_SH)) { shopSel_=idx; return; }
            }
        }
        // Buy button
        if (total>0 && shopSel_<total) {
            if (hit(-0.30f, -0.78f, 0.60f, 0.10f)) {
                int skinIdx=forSale[shopSel_];
                int price=skinPrice(skinIdx);
                if (saveData_.dbucks>=price) {
                    saveData_.dbucks -= price;
                    saveData_.addSkin(skinIdx);
                    // Show it in locker immediately
                    playerSkinIdx_ = skinIdx;
                    saveData_.selectedSkin = skinIdx;
                    saveGame(saveData_);
                    menuPage_ = MenuPage::Locker;
                }
                return;
            }
        }
        // Scroll up/down buttons
        if (hit(0.85f, 0.32f, 0.12f, 0.08f) && shopScroll_ > 0) { --shopScroll_; return; }
        if (hit(0.85f, 0.22f, 0.12f, 0.08f) && (shopScroll_+kRows)*kCols < total) { ++shopScroll_; return; }
    }
}

// ── Save / award ──────────────────────────────────────────────────────────────

void Game::awardAndSave() {
    saveData_.dbucks      += gameKills_ * 100;
    saveData_.totalKills  += gameKills_;
    saveData_.selectedSkin = playerSkinIdx_;
    saveData_.gliderIdx    = gliderIdx_;
    gameKills_ = 0;
    arenaPlayerScore_ = 0;
    saveGame(saveData_);
}

// ── Arena ─────────────────────────────────────────────────────────────────────

void Game::startArena() {
    state_   = GameState::Playing;
    isArena_ = true;

    // Place player at north spawn
    player_.position = kArenaSpawns[0];
    player_.velocity = glm::vec3(0.0f);
    player_.onGround = true;
    playerDead_      = false;
    respawnTimer_    = 0.0f;
    respawnPlayer();

    camera_.yaw   = 180.0f;   // face south toward action
    camera_.pitch = 0.0f;
    firstMouse_   = true;

    // Reset bots and scatter them around arena spawns
    botManager_.reset(world_);
    {
        auto& bots = botManager_.botsRef();
        for (int i = 0; i < (int)bots.size(); ++i) {
            int si = (i % 7) + 1;           // spawns 1-7 (leave 0 for player)
            bots[i].position    = kArenaSpawns[si];
            bots[i].kills       = 0;
            bots[i].lives       = 99;        // effectively infinite
            bots[i].state       = BotState::Patrol;
            bots[i].respawnTimer= 0.0f;
        }
    }

    dropManager_.reset();
    pickupManager_ = PickupManager{};

    arenaPlayerScore_ = 0;
    arenaWeaponTimer_ = 5.0f;    // first weapon drop after 5 s
    arenaTimeLeft_    = kArenaTime;
    gameTime_         = 0.0f;
    victoryAchieved_  = false;

    zone_.reset();
    zone_.setPaused(true);

    playerInWater_ = false;
    playerInLake_  = false;
    mapOpen_       = false;
    zoneWarnTimer_ = 0.0f;
    crosshairSpread_ = 0.0f;
}

// ── Bus flight update ─────────────────────────────────────────────────────────

void Game::updateBusFlight(float dt) {
    busT_ = std::min(busT_ + dt / kBusDuration, 1.0f);
    glm::vec3 busCurPos = glm::mix(busStart_, busEnd_, busT_);

    if (busPhase_ == 0) {
        player_.position = busCurPos + glm::vec3(0.0f, -1.5f, 0.0f);
        player_.velocity = glm::vec3(0.0f);

        bool spaceDown = glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS;
        if ((spaceDown && !spaceWasDown_) || busT_ >= 1.0f) {
            busPhase_        = 1;
            player_.position = busCurPos + glm::vec3(0.0f, -1.5f, 0.0f);
            player_.velocity = glm::vec3(18.0f, -2.0f, 0.0f);
            player_.onGround = false;
            firstMouse_      = true;
        }
        spaceWasDown_ = spaceDown;
        return;
    }

    if (busPhase_ == 1) {
        player_.velocity.y -= 22.0f * dt;
        player_.velocity.y  = std::max(player_.velocity.y, -42.0f);

        glm::vec3 fwd = camera_.forwardFlat();
        glm::vec3 rgt = camera_.rightFlat();
        const float kDiveSteer = 14.0f;
        glm::vec3 wish(0.0f);
        if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) wish += fwd;
        if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) wish -= fwd;
        if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) wish += rgt;
        if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) wish -= rgt;
        if (glm::length(wish) > 0.001f) {
            wish = glm::normalize(wish);
            player_.velocity.x = wish.x * kDiveSteer;
            player_.velocity.z = wish.z * kDiveSteer;
        }

        playerFacing_ = camera_.forwardFlat();

        bool spaceDown = glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS;
        if ((spaceDown && !spaceWasDown_) || player_.position.y < 30.0f) {
            busPhase_         = 2;
            gliderDeployAnim_ = 0.0f;
            gliderHeading_    = glm::radians(camera_.yaw + 90.0f);
            gliderBank_       = 0.0f;
            gliderYawRate_    = 0.0f;
        }
        spaceWasDown_ = spaceDown;
    }

    if (busPhase_ == 2) {
        // Animate glider open
        gliderDeployAnim_ = std::min(gliderDeployAnim_ + dt * 3.0f, 1.0f);

        // Track glider heading from camera yaw (smooth follow)
        float targetYaw = glm::radians(camera_.yaw + 90.0f);  // yaw=-90 → forward
        float dyaw = targetYaw - gliderHeading_;
        // Wrap to [-pi, pi]
        while (dyaw >  3.14159f) dyaw -= 6.28318f;
        while (dyaw < -3.14159f) dyaw += 6.28318f;
        float yawRate   = dyaw / std::max(dt, 0.001f);
        gliderYawRate_  = gliderYawRate_ * 0.85f + yawRate * 0.15f;
        gliderHeading_ += dyaw * std::min(1.0f, dt * 5.0f);
        // Bank angle: negative yaw rate = bank right, positive = bank left
        float targetBank = std::max(-0.55f, std::min(0.55f, -gliderYawRate_ * 0.018f));
        gliderBank_ = gliderBank_ * 0.88f + targetBank * 0.12f;

        float targetY = -5.0f;
        player_.velocity.y += (targetY - player_.velocity.y) * 6.0f * dt;

        glm::vec3 fwd = camera_.forwardFlat();
        glm::vec3 rgt = camera_.rightFlat();
        const float kGlideSpeed = 10.0f;
        glm::vec3 wish(0.0f);
        if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) wish += fwd;
        if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) wish -= fwd;
        if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) wish += rgt;
        if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) wish -= rgt;
        if (glm::length(wish) > 0.001f) {
            wish = glm::normalize(wish);
            player_.velocity.x = wish.x * kGlideSpeed;
            player_.velocity.z = wish.z * kGlideSpeed;
        } else {
            player_.velocity.x *= (1.0f - 4.0f * dt);
            player_.velocity.z *= (1.0f - 4.0f * dt);
        }
    }

    player_.position += player_.velocity * dt;

    if (player_.position.y <= 0.0f) {
        player_.position.y = 0.0f;
        player_.velocity   = glm::vec3(0.0f);
        player_.onGround   = true;
        state_             = GameState::Playing;
        gliderDeployAnim_  = 0.0f;
        if (!busIsRespawn_) {
            // First drop: full fresh game
            zone_.reset();
            zone_.setPaused(true);
            zoneStartDelay_ = 20.0f;
            botManager_.reset(world_);
            dropManager_.reset();
            dropManager_.spawnFloorLoot();
            gameTime_       = 0.0f;
            victoryAchieved_= false;
            victoryTimer_   = 0.0f;
        }
        respawnPlayer();
        zoneWarnTimer_ = 0.0f;
    }
}

// ── Update ────────────────────────────────────────────────────────────────────

void Game::update(float dt) {
    fps_ = fps_ * 0.92f + (dt > 0.0001f ? 1.0f / dt : 9999.0f) * 0.08f;

    // ── Cursor mode: normal in menu, locked in gameplay ───────────────────────
    {
        bool needLocked = (state_ != GameState::Menu);
        if (needLocked != cursorLocked_) {
            cursorLocked_ = needLocked;
            if (needLocked) {
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                if (glfwRawMouseMotionSupported())
                    glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
                firstMouse_ = true;
            } else {
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                if (glfwRawMouseMotionSupported())
                    glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
            }
        }
    }

    // ── Menu ──────────────────────────────────────────────────────────────────
    if (state_ == GameState::Menu) {
        gameTime_ += dt;

        bool enterDown = glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_PRESS;
        bool upDown    = glfwGetKey(window_, GLFW_KEY_UP)    == GLFW_PRESS;
        bool downDown  = glfwGetKey(window_, GLFW_KEY_DOWN)  == GLFW_PRESS;
        bool leftDown  = glfwGetKey(window_, GLFW_KEY_LEFT)  == GLFW_PRESS;
        bool rightDown = glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS;
        bool lDown     = glfwGetKey(window_, GLFW_KEY_L)     == GLFW_PRESS;
        bool hDown     = glfwGetKey(window_, GLFW_KEY_H)     == GLFW_PRESS;
        bool sDown     = glfwGetKey(window_, GLFW_KEY_S)     == GLFW_PRESS;
        bool aDown     = glfwGetKey(window_, GLFW_KEY_A)     == GLFW_PRESS;
        bool dDown     = glfwGetKey(window_, GLFW_KEY_D)     == GLFW_PRESS;
        bool gDown     = glfwGetKey(window_, GLFW_KEY_G)     == GLFW_PRESS;

        // ── Global tab shortcuts ──────────────────────────────────────────────
        if (lDown && !lWasDown_ && menuPage_ != MenuPage::Locker) {
            menuPage_   = MenuPage::Locker;
            lockerSel_  = 0;
            lockerGlider_ = false;
        }
        if (hDown && !hWasDown_ && menuPage_ != MenuPage::Shop) {
            menuPage_  = MenuPage::Shop;
            shopSel_   = 0;
            shopScroll_= 0;
        }

        if (menuPage_ == MenuPage::Main) {
            if (enterDown && !enterWasDown_) startBusFlight(false);
            if (aDown && !aWasDown_) startArena();
            if (sDown && !sWasDown_) menuPage_ = MenuPage::Settings;
            if (dDown && !diffWasDown_) {
                settingIdx_ = static_cast<int>(difficulty_);
                menuPage_ = MenuPage::Difficulty;
            }

        } else if (menuPage_ == MenuPage::Locker) {
            if (gDown && !gMenuWasDown_) lockerGlider_ = !lockerGlider_;
            if (!lockerGlider_) {
                // Skin grid navigation
                int owned = static_cast<int>(saveData_.ownedSkins.size());
                if (leftDown  && !leftWasDown_) lockerSel_ = std::max(0, lockerSel_ - 1);
                if (rightDown && !rightWasDown_) lockerSel_ = std::min(owned - 1, lockerSel_ + 1);
                if (upDown    && !upWasDown_)    lockerSel_ = std::max(0, lockerSel_ - 5);
                if (downDown  && !downWasDown_)  lockerSel_ = std::min(owned - 1, lockerSel_ + 5);
                if (enterDown && !enterWasDown_ && owned > 0) {
                    playerSkinIdx_          = saveData_.ownedSkins[lockerSel_];
                    saveData_.selectedSkin  = playerSkinIdx_;
                    saveGame(saveData_);
                }
            } else {
                // Glider grid (15 cols x 10 rows)
                if (leftDown  && !leftWasDown_)  gliderIdx_ = (gliderIdx_ + 150 - 1) % 150;
                if (rightDown && !rightWasDown_)  gliderIdx_ = (gliderIdx_ + 1) % 150;
                if (upDown    && !upWasDown_)     gliderIdx_ = (gliderIdx_ + 150 - 15) % 150;
                if (downDown  && !downWasDown_)   gliderIdx_ = (gliderIdx_ + 15) % 150;
                if (enterDown && !enterWasDown_) {
                    saveData_.gliderIdx = gliderIdx_;
                    saveGame(saveData_);
                }
            }

        } else if (menuPage_ == MenuPage::Shop) {
            // Build un-owned list
            std::vector<int> forSale;
            for (int i = 0; i < kSkinCount; ++i)
                if (!saveData_.ownsSkin(i)) forSale.push_back(i);
            int total = static_cast<int>(forSale.size());
            if (leftDown  && !leftWasDown_) shopSel_ = std::max(0, shopSel_ - 1);
            if (rightDown && !rightWasDown_) shopSel_ = std::min(total - 1, shopSel_ + 1);
            if (upDown    && !upWasDown_)   shopSel_ = std::max(0, shopSel_ - 4);
            if (downDown  && !downWasDown_) shopSel_ = std::min(total - 1, shopSel_ + 4);
            // Keep scroll in sync
            const int kRows = 3, kCols = 4;
            int selRow = shopSel_ / kCols;
            if (selRow < shopScroll_) shopScroll_ = selRow;
            if (selRow >= shopScroll_ + kRows) shopScroll_ = selRow - kRows + 1;
            // Buy
            if (enterDown && !enterWasDown_ && total > 0) {
                int idx   = forSale[shopSel_];
                int price = skinPrice(idx);
                if (saveData_.dbucks >= price) {
                    saveData_.dbucks -= price;
                    saveData_.addSkin(idx);
                    saveGame(saveData_);
                    // Remove from sale list — shift selection if needed
                    if (shopSel_ >= static_cast<int>(forSale.size()) - 1)
                        shopSel_ = std::max(0, shopSel_ - 1);
                }
            }

        } else if (menuPage_ == MenuPage::Difficulty) {
            if (upDown   && !upWasDown_)   settingIdx_ = std::max(0, settingIdx_ - 1);
            if (downDown && !downWasDown_) settingIdx_ = std::min(5, settingIdx_ + 1);
            difficulty_ = static_cast<Difficulty>(settingIdx_);
            if (enterDown && !enterWasDown_) menuPage_ = MenuPage::Main;

        } else {
            // Settings page
            if (upDown   && !upWasDown_)   settingIdx_ = std::max(0, settingIdx_ - 1);
            if (downDown && !downWasDown_) settingIdx_ = std::min(2, settingIdx_ + 1);
            if (leftDown && !leftWasDown_)  adjustSetting(-1);
            if (rightDown && !rightWasDown_) adjustSetting(+1);
        }

        enterWasDown_ = enterDown;
        sWasDown_     = sDown;
        aWasDown_     = aDown;
        diffWasDown_  = dDown;
        lWasDown_     = lDown;
        hWasDown_     = hDown;
        gMenuWasDown_ = gDown;
        upWasDown_    = upDown;
        downWasDown_  = downDown;
        leftWasDown_  = leftDown;
        rightWasDown_ = rightDown;
        return;
    }

    // ── Bus flight ────────────────────────────────────────────────────────────
    if (state_ == GameState::BusFlight) {
        gameTime_ += dt;
        updateBusFlight(dt);
        return;
    }

    // ── Playing ───────────────────────────────────────────────────────────────
    gameTime_ += dt;
    effects_.update(dt);

    crosshairSpread_ -= dt * 5.0f;
    if (crosshairSpread_ < 0.0f) crosshairSpread_ = 0.0f;

    zone_.update(dt);
    dropManager_.update(dt);
    pickupManager_.update(dt);

    // ── Poll bot loot drops ──
    for (const auto& dr : botManager_.pendingDrops())
        dropManager_.spawnFromBot(dr.pos, dr.weaponType);
    botManager_.clearDrops();

    // ── Victory check (skipped in arena — it never ends by elimination) ──────
    if (!isArena_) {
        if (!playerDead_ && !victoryAchieved_ && botManager_.aliveCount() == 0) {
            victoryAchieved_ = true;
            victoryTimer_    = 0.0f;
        }
        if (victoryAchieved_) {
            victoryTimer_ += dt;
            if (victoryTimer_ > 12.0f) {
                awardAndSave();
                state_    = GameState::Menu;
                menuPage_ = MenuPage::Main;
            }
        }
    }

    // ── Weapon slots 1-5 ──
    static const int kSlotKeys[5] = {
        GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5
    };
    for (int i = 0; i < 5; ++i) {
        bool down = glfwGetKey(window_, kSlotKeys[i]) == GLFW_PRESS;
        if (down && !slot_[i]) player_.selectWeapon(i);
        slot_[i] = down;
    }

    // ── Pickaxe equip (P) ──
    bool pDown = glfwGetKey(window_, GLFW_KEY_X) == GLFW_PRESS;
    if (pDown && !pWasDown_) {
        player_.pickaxeActive = true;
        player_.weapons[player_.currentWeapon].reloading = false;
    }
    pWasDown_ = pDown;

    // ── Camera toggle (V) ──
    bool vDown = glfwGetKey(window_, GLFW_KEY_V) == GLFW_PRESS;
    if (vDown && !vWasDown_) camera_.toggleMode();
    vWasDown_ = vDown;

    // ── Map toggle (M) ──
    bool mDown = glfwGetKey(window_, GLFW_KEY_M) == GLFW_PRESS;
    if (mDown && !mWasDown_) mapOpen_ = !mapOpen_;
    mWasDown_ = mDown;

    // ── Dead state ──────────────────────────────────────────────────────────
    if (playerDead_) {
        respawnTimer_ += dt;

        if (isArena_ && respawnTimer_ >= 5.0f) {
            // Auto-respawn in arena
            player_.position = kArenaSpawns[0];
            player_.velocity = glm::vec3(0.0f);
            player_.onGround = true;
            respawnPlayer();
            playerDead_   = false;
            respawnTimer_ = 0.0f;
        } else if (!isArena_) {
            bool rDown = glfwGetKey(window_, GLFW_KEY_R) == GLFW_PRESS;
            if (rDown && !rWasDown_) {
                awardAndSave();
                state_    = GameState::Menu;
                menuPage_ = MenuPage::Main;
                botManager_.reset(world_);
                dropManager_.reset();
                victoryAchieved_ = false;
                isArena_ = false;
            }
            rWasDown_ = rDown;
        }

        botManager_.update(dt, player_, world_, difficulty_,
                           zone_.center(), zone_.radius(), zone_.damagePerSec(), dropManager_);
        return;
    }

    // ── Zone start delay countdown ──
    if (zoneStartDelay_ > 0.0f) {
        zoneStartDelay_ -= dt;
        if (zoneStartDelay_ <= 0.0f) {
            zoneStartDelay_ = 0.0f;
            zone_.setPaused(false);
        }
    }

    // ── Arena mode: weapon spawns, bot infinite-respawn, time limit ──────────
    if (isArena_) {
        arenaTimeLeft_ -= dt;
        if (arenaTimeLeft_ <= 0.0f) {
            awardAndSave();
            state_    = GameState::Menu;
            menuPage_ = MenuPage::Main;
            isArena_  = false;
            return;
        }
        arenaWeaponTimer_ -= dt;
        if (arenaWeaponTimer_ <= 0.0f) {
            arenaWeaponTimer_ = kArenaWeaponInterval;
            for (int i = 0; i < 8; ++i) {
                bool occupied = false;
                for (int j = 0; j < dropManager_.itemCount(); ++j) {
                    if (glm::length(dropManager_.item(j).pos - kArenaWeaponSpawns[i]) < 3.0f) {
                        occupied = true;
                        break;
                    }
                }
                if (!occupied) {
                    static const WeaponType kWeapons[5] = {
                        WeaponType::Pistol, WeaponType::Rifle, WeaponType::Shotgun,
                        WeaponType::Sniper, WeaponType::SMG
                    };
                    dropManager_.spawnFromBot(kArenaWeaponSpawns[i],
                        kWeapons[std::uniform_int_distribution<int>(0,4)(rng_)]);
                }
            }
        }
        // Keep bots alive: re-queue permanently-dead bots for respawn
        for (Bot& b : botManager_.botsRef()) {
            if (b.state == BotState::Dead) {
                b.lives        = 1;
                b.state        = BotState::Respawning;
                b.respawnTimer = 4.0f;
                b.weaponType   = WeaponType::Pistol;  // punishment: restart with pistol
            }
            // Teleport respawning bot to a random arena spawn when timer expires
            if (b.state == BotState::Respawning && b.respawnTimer <= 0.0f) {
                int si = std::uniform_int_distribution<int>(1, 7)(rng_);
                b.position    = kArenaSpawns[si];
                b.health      = b.maxHealth;
                b.seekingLoot = false;
                b.lootItemIdx = -1;
            }
        }
    }

    // ── Zone damage (skipped in arena) ───────────────────────────────────────
    if (!isArena_ && !zone_.isInside(player_.position)) {
        effects_.triggerDamageFlash();
        player_.takeDamage(zone_.damagePerSec() * dt);
        zoneWarnTimer_ -= dt;
        if (zoneWarnTimer_ <= 0.0f) {
            zoneWarnTimer_ = 3.0f;
        }
    } else if (!isArena_) {
        zoneWarnTimer_ = 0.0f;
    }

    // ── Aim Down Sights (RMB) ──
    isAiming_ = (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                && !player_.pickaxeActive;

    // ── Movement ──
    {
        float savedSpeed = player_.moveSpeed;
        float savedSprint = player_.sprintMul;
        if (isAiming_) {
            player_.moveSpeed *= 0.45f;
            player_.sprintMul  = 1.0f;   // no sprint while aiming
        }
        player_.handleInput(window_, camera_, dt);
        player_.moveSpeed = savedSpeed;
        player_.sprintMul = savedSprint;
    }
    player_.update(dt);
    if (world_.resolvePlayerBuilding(player_.position, player_.radius, player_.height)) {
        player_.onGround = true;
        if (player_.velocity.y < 0.0f) player_.velocity.y = 0.0f;
    }

    // ── Water detection ───────────────────────────────────────────────────────
    {
        const float kLandEdge = 248.0f;
        bool inOcean = (fabsf(player_.position.x) > kLandEdge ||
                        fabsf(player_.position.z) > kLandEdge) && !playerDead_;
        bool inLake  = !inOcean && world_.isInlandWater(player_.position) && !playerDead_;
        playerInWater_ = inOcean;
        playerInLake_  = inLake;
        if (playerInWater_) {
            player_.takeDamage(18.0f * dt);
            player_.velocity.x *= std::max(0.0f, 1.0f - 7.0f * dt);
            player_.velocity.z *= std::max(0.0f, 1.0f - 7.0f * dt);
        }
        if (playerInLake_) {
            // Swimming: no damage, just gentle drag
            player_.velocity.x *= std::max(0.0f, 1.0f - 5.0f * dt);
            player_.velocity.z *= std::max(0.0f, 1.0f - 5.0f * dt);
        }
    }

    playerFacing_ = camera_.forwardFlat();
    float speedXZ = glm::length(glm::vec2(player_.velocity.x, player_.velocity.z));
    if (speedXZ > 0.5f) playerWalkPhase_ += speedXZ * dt * 1.6f;
    if (speedXZ > 2.0f) crosshairSpread_ = std::min(crosshairSpread_ + speedXZ * dt * 0.12f, 1.2f);

    {
        // ── Combat / pickaxe mode ──────────────────────────────────────────────

        // Tab = cycle weapon (skip empty)
        bool tabDown = glfwGetKey(window_, GLFW_KEY_TAB) == GLFW_PRESS;
        if (tabDown && !tabWasDown_ && !inventoryOpen_) player_.cycleWeapon(1);
        tabWasDown_ = tabDown;

        // I = inventory/drop menu
        bool iDown = glfwGetKey(window_, GLFW_KEY_I) == GLFW_PRESS;
        if (iDown && !iWasDown_) {
            inventoryOpen_ = !inventoryOpen_;
            inventorySel_  = 0;
        }
        iWasDown_ = iDown;

        // Inventory navigation and drop
        if (inventoryOpen_) {
            bool upDown    = glfwGetKey(window_, GLFW_KEY_UP)    == GLFW_PRESS;
            bool downDown  = glfwGetKey(window_, GLFW_KEY_DOWN)  == GLFW_PRESS;
            bool dDown     = glfwGetKey(window_, GLFW_KEY_D)     == GLFW_PRESS;
            bool escDown   = glfwGetKey(window_, GLFW_KEY_ESCAPE)== GLFW_PRESS;
            if (upDown   && !upWasDown_)   inventorySel_ = (inventorySel_ + 4) % 5;
            if (downDown && !downWasDown_) inventorySel_ = (inventorySel_ + 1) % 5;
            upWasDown_   = upDown;
            downWasDown_ = downDown;
            if (dDown && !dWasDown_) {
                // Drop selected item at player feet
                glm::vec3 dropPos = player_.position + glm::vec3(0.5f, 0.25f, 0.5f);
                if (inventorySel_ < 5) {
                    int slot = inventorySel_;
                    if (player_.hasWeapon[slot]) {
                        DroppedItem di;
                        di.pos       = dropPos;
                        di.hasWeapon = true;
                        di.weaponType= player_.weapons[slot].type;
                        di.nameIdx   = player_.weapons[slot].nameIdx;
                        di.ammo      = player_.weapons[slot].ammo;
                        di.reserve   = player_.weapons[slot].reserve;
                        di.bobTimer  = 0.0f;
                        dropManager_.spawnDroppedItem(di);
                        player_.hasWeapon[slot] = false;
                        player_.weapons[slot]   = Weapon{};
                        if (player_.currentWeapon == slot) player_.cycleWeapon(1);
                    }
                }
            }
            dWasDown_ = dDown;
            if (escDown) inventoryOpen_ = false;
        }

        // R = reload (only when holding weapon, not in inventory)
        bool rDown = glfwGetKey(window_, GLFW_KEY_R) == GLFW_PRESS;
        if (rDown && !rWasDown_ && !player_.pickaxeActive && !inventoryOpen_)
            player_.weapon().startReload();
        rWasDown_ = rDown;

        // Q = dash
        bool qDown = glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS;
        if (qDown && !qWasDown_) {
            glm::vec3 wishDir(0.0f);
            if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) wishDir += camera_.forwardFlat();
            if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) wishDir -= camera_.forwardFlat();
            if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) wishDir += camera_.rightFlat();
            if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) wishDir -= camera_.rightFlat();
            if (glm::length(wishDir) < 0.001f) wishDir = camera_.forwardFlat();
            player_.activateDash(glm::normalize(wishDir));
        }
        qWasDown_ = qDown;

        // E tap = interact (pickup item or open chest); E hold = grapple if nothing nearby
        bool eDown = glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS;
        if (eDown && !eWasDown_) {
            // Door interaction takes priority
            if (world_.tryToggleDoor(player_.position) >= 0) {
                // door toggled — no further interaction
            } else
            // Try pickup (weapons only via E)
            if (int itemIdx = dropManager_.tryPickup(player_.position); itemIdx >= 0) {
                DroppedItem it = dropManager_.item(itemIdx);
                dropManager_.removeItem(itemIdx);
                if (it.hasWeapon) {
                    player_.giveWeapon(it.weaponType, it.ammo, it.reserve, it.nameIdx);
                } else if (it.reserve > 0) {
                    int slot = static_cast<int>(it.weaponType);
                    if (slot >= 0 && slot < 5)
                        player_.weapons[slot].reserve = std::min(player_.weapons[slot].reserve + it.reserve, 300);
                }
            } else
            // Try chest
            if (int ci = dropManager_.tryOpenChest(player_.position); ci >= 0) {
                const Chest& ch = dropManager_.chest(ci);
                dropManager_.spawnFromChest(ch.pos, ch.isBlue);
            } else {
                // Grapple as last resort
                glm::vec3 origin2 = camera_.position(player_);
                glm::vec3 fwd2    = camera_.forward();
                glm::vec3 hitPt2;
                float t2 = world_.raycast(origin2, fwd2, Player::kGrappleRange, &hitPt2);
                if (t2 < Player::kGrappleRange) player_.activateGrapple(hitPt2);
            }
        }
        if (!eDown && player_.grappleActive) player_.cancelGrapple();
        eWasDown_ = eDown;

        // G = heal
        bool gDown = glfwGetKey(window_, GLFW_KEY_G) == GLFW_PRESS;
        if (gDown && !gWasDown_) player_.useHeal();
        gWasDown_ = gDown;

        // LMB / F: pickaxe swing OR shoot
        bool fireDown = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS
                     || glfwGetKey(window_, GLFW_KEY_F) == GLFW_PRESS;

        if (player_.pickaxeActive) {
            // ── Pickaxe swing (melee only, 2.5 m range, 10 damage) ──
            if (fireDown && player_.pickaxeCooldown <= 0.0f) {
                player_.pickaxeCooldown  = 0.55f;
                player_.pickaxeSwingAnim = 1.0f;
                // Melee: only hit bots within 2.5 m
                static constexpr float kMeleeRange = 2.5f;
                glm::vec3 origin = camera_.position(player_);
                glm::vec3 fwd    = camera_.forward();
                ShotResult r = botManager_.playerShot(origin, fwd, 10.0f);
                if (r.hit) {
                    float hitDist = glm::length(r.hitPos - origin);
                    if (hitDist <= kMeleeRange) {
                        effects_.spawnDamageNum(r.hitPos + glm::vec3(0, 0.3f, 0), r.damage, false);
                        effects_.triggerHitMarker();
                        if (r.killed) effects_.triggerKill();
                    }
                }
            }
        } else {
            // ── Shoot ──
            Weapon& w = player_.weapon();
            if (fireDown && w.canFire()) {
                const WeaponStats& ws = getWeaponStats(w.type);
                glm::vec3 origin = camera_.position(player_);
                glm::vec3 fwd    = camera_.forward();

                glm::vec3 up2  = (fabsf(fwd.y) < 0.9f) ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
                glm::vec3 rgt2 = glm::normalize(glm::cross(fwd, up2));
                glm::vec3 upv  = glm::cross(rgt2, fwd);
                std::uniform_real_distribution<float> sD(-ws.spreadRad, ws.spreadRad);

                effects_.triggerMuzzleFlash();
                crosshairSpread_ = std::min(crosshairSpread_ + 0.35f, 1.5f);

                bool anyHit = false, anyKill = false;
                for (int pel = 0; pel < ws.pelletsPerShot; ++pel) {
                    glm::vec3 dir = glm::normalize(fwd + rgt2 * sD(rng_) + upv * sD(rng_));
                    // Sniper bullet drop: full damage ≤80 m, falls to 35% at max range
                    float pelDmg = ws.damage;
                    if (w.type == WeaponType::Sniper) {
                        glm::vec3 hitPtSnipe;
                        float d = world_.raycast(origin, dir, ws.range, &hitPtSnipe);
                        if (d < ws.range) {
                            const float kFullRange = 80.0f;
                            if (d > kFullRange) {
                                float t = std::min(1.0f, (d - kFullRange) / (ws.range - kFullRange));
                                pelDmg *= (1.0f - t * 0.65f);   // 35% damage at max range
                            }
                        }
                    }
                    ShotResult r  = botManager_.playerShot(origin, dir, pelDmg);
                    glm::vec3 tracerEnd = r.hit ? r.hitPos : (origin + dir * 80.0f);
                    effects_.spawnTracer(origin + fwd * 0.5f, tracerEnd);
                    if (r.hit) {
                        anyHit = true;
                        effects_.spawnDamageNum(r.hitPos + glm::vec3(0, 0.4f, 0), r.damage, r.headshot);
                        if (r.killed) anyKill = true;
                    }
                }
                if (anyHit)  effects_.triggerHitMarker();
                if (anyKill) {
                    effects_.triggerKill();
                    ++gameKills_;
                    if (isArena_) ++arenaPlayerScore_;
                }
                --w.ammo;
                w.fireTimer = ws.fireInterval;
                if (w.ammo == 0) w.startReload();
            }
        }
    }

    // ── Death check ──
    if (player_.health <= 0.0f && !playerDead_) {
        playerDead_  = true;
        respawnTimer_= 0.0f;
    }

    // ── Bot damage to player ──
    float hBefore = player_.health + player_.shield;
    botManager_.update(dt, player_, world_, difficulty_,
                       zone_.center(), zone_.radius(), zone_.damagePerSec(), dropManager_);
    float hAfter = player_.health + player_.shield;
    if (hAfter < hBefore - 0.5f) effects_.triggerDamageFlash();

    // ── Auto-collect ammo packs from floor ──
    for (int ii = dropManager_.itemCount() - 1; ii >= 0; --ii) {
        const DroppedItem& it = dropManager_.item(ii);
        if (it.hasWeapon) continue;
        if (glm::length(it.pos - player_.position) > 1.8f) continue;
        if (it.reserve > 0) {
            int slot = static_cast<int>(it.weaponType);
            if (slot >= 0 && slot < 5)
                player_.weapons[slot].reserve = std::min(player_.weapons[slot].reserve + it.reserve, 300);
        }
        dropManager_.removeItem(ii);
    }

    // ── Auto-collect health/shield items (pickupManager) ──
    int collected = pickupManager_.tryCollect(player_.position);
    if (collected >= 0) {
        auto pk = static_cast<PickupKind>(collected);
        switch (pk) {
            case PickupKind::Medkit:       if (player_.medkits       < 5)  ++player_.medkits;       break;
            case PickupKind::Bandage:      if (player_.bandages      < 9)  ++player_.bandages;      break;
            case PickupKind::ShieldPotion: if (player_.shieldPotions < 3)  ++player_.shieldPotions; break;
            case PickupKind::MiniShield:   if (player_.miniShields   < 5)  ++player_.miniShields;   break;
            default: break;
        }
    }
}

// ── Bus model render ──────────────────────────────────────────────────────────

void Game::renderBusModel(const glm::mat4& proj, const glm::mat4& view) {
    glm::vec3 bp = glm::mix(busStart_, busEnd_, busT_);

    worldShader_.use();
    worldShader_.setMat4 ("uView",     view);
    worldShader_.setMat4 ("uProj",     proj);
    worldShader_.setVec3 ("uLightDir", kLightDir);
    worldShader_.setFloat("uAlpha",    1.0f);
    worldShader_.setVec3 ("uViewPos",  bp + glm::vec3(-18, 10, 0));

    world_.beginBoxRender();
    world_.drawBox(worldShader_, bp,                              {14.0f, 3.2f, 4.0f},  {0.95f, 0.80f, 0.10f});
    world_.drawBox(worldShader_, bp + glm::vec3(0, 1.85f, 0),    {14.2f, 0.45f, 4.3f}, {0.80f, 0.66f, 0.08f});
    world_.drawBox(worldShader_, bp + glm::vec3(0,-1.62f, 0),    {13.0f, 0.40f, 3.8f}, {0.30f, 0.28f, 0.24f});
    world_.drawBox(worldShader_, bp + glm::vec3(-6.85f, 0.15f, 0), {0.30f, 1.80f, 3.5f}, {0.40f, 0.72f, 0.95f});
    world_.drawBox(worldShader_, bp + glm::vec3( 6.85f, 0.15f, 0), {0.30f, 1.80f, 3.5f}, {0.40f, 0.72f, 0.95f});
    world_.drawBox(worldShader_, bp + glm::vec3(0, 0.30f,-2.06f), {10.0f, 1.40f, 0.28f}, {0.40f, 0.72f, 0.95f});
    world_.drawBox(worldShader_, bp + glm::vec3(0, 0.30f, 2.06f), {10.0f, 1.40f, 0.28f}, {0.40f, 0.72f, 0.95f});
    world_.drawBox(worldShader_, bp + glm::vec3(-4, 6.0f, 0),    {3.5f, 4.8f, 3.5f},   {0.50f, 0.72f, 1.00f});
    world_.drawBox(worldShader_, bp + glm::vec3( 4, 6.0f, 0),    {3.5f, 4.8f, 3.5f},   {0.50f, 0.72f, 1.00f});
    world_.drawBox(worldShader_, bp + glm::vec3(-4, 3.2f, 0),    {0.22f, 2.2f, 0.22f}, {0.70f, 0.58f, 0.30f});
    world_.drawBox(worldShader_, bp + glm::vec3( 4, 3.2f, 0),    {0.22f, 2.2f, 0.22f}, {0.70f, 0.58f, 0.30f});
    world_.drawBox(worldShader_, bp + glm::vec3( 7.2f,-0.5f, 0), {0.30f, 0.30f, 3.80f},{0.65f, 0.60f, 0.55f});
    world_.endBoxRender();
}

// ── Glider render (3D canopy during phase 2) ──────────────────────────────────

void Game::renderGlider(const glm::mat4& proj, const glm::mat4& view) {
    if (busPhase_ != 2 || gliderDeployAnim_ < 0.01f) return;

    const GliderSkin& gs = getGliders()[gliderIdx_];
    float a = gliderDeployAnim_;

    worldShader_.use();
    worldShader_.setMat4 ("uView",     view);
    worldShader_.setMat4 ("uProj",     proj);
    worldShader_.setVec3 ("uLightDir", kLightDir);
    worldShader_.setVec3 ("uViewPos",  camera_.position(player_));

    // Heading (yaw) + bank (roll) rotation
    float sinH = sinf(gliderHeading_), cosH = cosf(gliderHeading_);
    float sinB = sinf(gliderBank_),    cosB = cosf(gliderBank_);

    // Rotate local (lx,ly,lz) → world offset: bank first, then heading
    auto rot = [&](float lx, float ly, float lz) -> glm::vec3 {
        float bx = lx * cosB - ly * sinB;
        float by = lx * sinB + ly * cosB;
        float bz = lz;
        return { bx * cosH - bz * sinH, by, bx * sinH + bz * cosH };
    };

    float cw = 7.0f * a;   // wingspan
    float ch = 0.30f;      // canopy thickness
    float cd = 4.5f * a;   // canopy depth (front-to-back)
    glm::vec3 base = player_.position + glm::vec3(0.0f, 3.5f, 0.0f);

    // Draw an axis-aligned box centred at (base + rotated offset),
    // with sizes projected onto world axes for correct heading.
    auto gb = [&](float ox, float oy, float oz,
                  float sx, float sy, float sz, const glm::vec3& col) {
        glm::vec3 ctr = base + rot(ox, oy, oz);
        float rsx = fabsf(sx * cosH) + fabsf(sz * sinH);
        float rsz = fabsf(sx * sinH) + fabsf(sz * cosH);
        worldShader_.setFloat("uAlpha", 0.95f * a);
        worldShader_.setVec3 ("uColor", col);
        glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), ctr),
                                 glm::vec3(rsx, sy, rsz));
        worldShader_.setMat4("uModel", m);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    };

    // Draw a diagonal line (shroud / spar) from world point A to world point B.
    auto drawLine = [&](const glm::vec3& A, const glm::vec3& B,
                        float thickness, const glm::vec3& col) {
        glm::vec3 diff = B - A;
        float len = glm::length(diff);
        if (len < 0.01f) return;
        glm::vec3 dir = diff / len;
        glm::vec3 yUp(0.0f, 1.0f, 0.0f);
        glm::vec3 axis = glm::cross(yUp, dir);
        float sinA = glm::length(axis);
        float cosA = glm::dot(yUp, dir);
        glm::mat4 rot_mat(1.0f);
        if (sinA > 1e-5f)
            rot_mat = glm::rotate(glm::mat4(1.0f), atan2f(sinA, cosA), axis / sinA);
        glm::vec3 mid = (A + B) * 0.5f;
        glm::mat4 m = glm::translate(glm::mat4(1.0f), mid)
                    * rot_mat
                    * glm::scale(glm::mat4(1.0f), glm::vec3(thickness, len, thickness));
        worldShader_.setMat4 ("uModel", m);
        worldShader_.setVec3 ("uColor", col);
        worldShader_.setFloat("uAlpha", 0.78f * a);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    };

    glBindVertexArray(0);
    world_.beginBoxRender();

    // ── Canopy panels ────────────────────────────────────────────────────────
    // Main centre panel
    gb(0.0f,  0.0f, 0.0f,   cw,        ch,        cd,        gs.canopy);
    // Left stripe
    gb(-(cw * 0.38f), -0.06f, 0.0f,  cw * 0.22f, ch * 0.75f, cd * 0.90f, gs.stripe);
    // Right stripe
    gb( (cw * 0.38f), -0.06f, 0.0f,  cw * 0.22f, ch * 0.75f, cd * 0.90f, gs.stripe);
    // Wing tips
    gb(-(cw * 0.5f + 1.2f * a), -0.12f, 0.0f, 2.4f * a, ch * 0.60f, cd * 0.55f, gs.stripe);
    gb( (cw * 0.5f + 1.2f * a), -0.12f, 0.0f, 2.4f * a, ch * 0.60f, cd * 0.55f, gs.stripe);

    // ── Structural spars ──────────────────────────────────────────────────────
    gb(0.0f, ch * 0.40f, 0.0f,  cw * 0.05f, ch * 0.50f, cd,        gs.frame); // keel
    gb(0.0f, ch * 0.20f, 0.0f,  cw,         ch * 0.10f, cd * 0.05f, gs.frame); // cross brace

    // ── Shroud lines (4 lines: front-left, front-right, back-left, back-right) ─
    // Harness attach points at player shoulders
    glm::vec3 harnessL = player_.position + rot(-0.35f, 1.4f, 0.0f);
    glm::vec3 harnessR = player_.position + rot( 0.35f, 1.4f, 0.0f);
    // Canopy attachment corners
    glm::vec3 cfL = base + rot(-cw * 0.42f, -ch * 0.5f, -cd * 0.45f); // front-left
    glm::vec3 cfR = base + rot( cw * 0.42f, -ch * 0.5f, -cd * 0.45f); // front-right
    glm::vec3 cbL = base + rot(-cw * 0.42f, -ch * 0.5f,  cd * 0.45f); // back-left
    glm::vec3 cbR = base + rot( cw * 0.42f, -ch * 0.5f,  cd * 0.45f); // back-right

    drawLine(harnessL, cfL, 0.05f, gs.frame);
    drawLine(harnessR, cfR, 0.05f, gs.frame);
    drawLine(harnessL, cbL, 0.05f, gs.frame);
    drawLine(harnessR, cbR, 0.05f, gs.frame);

    worldShader_.setFloat("uAlpha", 1.0f);
    world_.endBoxRender();
}

// ── Bus flight render ─────────────────────────────────────────────────────────

void Game::renderBusFlight() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = height_ > 0 ? float(width_) / float(height_) : 16.0f/9.0f;
    glm::mat4 proj, view;
    glm::vec3 eye;

    // All bus phases use the player camera (mouse-look works throughout)
    proj = camera_.projection(aspect);
    view = camera_.view(player_);
    eye  = camera_.position(player_);

    glEnable(GL_DEPTH_TEST);
    worldShader_.use();
    worldShader_.setMat4("uView", view);
    worldShader_.setMat4("uProj", proj);
    world_.render(worldShader_, eye);
    botManager_.render(worldShader_, kLightDir, eye);
    renderBusModel(proj, view);

    if (busPhase_ > 0) {
        // Only render player body in TPS — in FPS the camera is inside the body
        if (camera_.mode == CameraMode::TPS)
            botManager_.renderPlayerCharacter(worldShader_, kLightDir, eye,
                                              player_.position, playerFacing_, 0.0f, playerSkinIdx_);
        renderGlider(proj, view);
        effects_.renderTracers(proj * view);
    }

    glDisable(GL_DEPTH_TEST);
    renderBusHUD();
}

void Game::renderBusHUD() {
    const float CWN = 0.026f, CHN = 0.046f;
    const float CWS = 0.020f, CHS = 0.036f;
    const float CWH = 0.032f, CHH = 0.056f;

    int alt = std::max(0, (int)player_.position.y);
    const GliderSkin& gs = getGliders()[gliderIdx_];

    if (busPhase_ == 0) {
        float pulse = 0.6f + 0.4f * sinf(gameTime_ * 3.5f);
        const std::string hint = "[  SPACE  ]  JUMP FROM BUS";
        font_.drawShadowed(hint, -font_.textWidth(hint, CWH)*0.5f, -0.72f,
                           CWH, CHH, {0.25f, 1.0f, 0.45f, pulse});
        int pct = int(busT_ * 100.0f);
        std::string prog = "BUS PATH  " + std::to_string(pct) + "%";
        font_.drawShadowed(prog, -font_.textWidth(prog, CWS)*0.5f, 0.88f,
                           CWS, CHS, {0.95f, 0.85f, 0.15f, 0.9f});
        // Glider name hint
        std::string gn = "GLIDER: " + gs.name;
        font_.drawShadowed(gn, -font_.textWidth(gn, CWS)*0.5f, -0.86f,
                           CWS, CHS, {gs.canopy.r, gs.canopy.g, gs.canopy.b, 0.9f});

    } else if (busPhase_ == 1) {
        std::string altStr = "ALT  " + std::to_string(alt) + " m";
        font_.drawShadowed(altStr, -font_.textWidth(altStr, CWN)*0.5f, 0.88f,
                           CWN, CHN, {1.0f, 0.85f, 0.15f, 0.95f});
        float pulse = 0.6f + 0.4f * sinf(gameTime_ * 4.0f);
        const std::string glide = "[  SPACE  ]  DEPLOY GLIDER";
        font_.drawShadowed(glide, -font_.textWidth(glide, CWH)*0.5f, -0.72f,
                           CWH, CHH, {0.30f, 0.85f, 1.0f, pulse});
        font_.drawShadowed("WASD STEER", -font_.textWidth("WASD STEER", CWS)*0.5f, -0.86f,
                           CWS, CHS, {0.9f, 0.9f, 0.9f, 0.7f});

    } else {
        std::string altStr = "ALT  " + std::to_string(alt) + " m   GLIDING";
        font_.drawShadowed(altStr, -font_.textWidth(altStr, CWN)*0.5f, 0.88f,
                           CWN, CHN, {0.30f, 0.90f, 1.0f, 0.95f});
        // Glider name
        font_.drawShadowed(gs.name, -font_.textWidth(gs.name, CWN)*0.5f, 0.80f,
                           CWN, CHN, {gs.canopy.r, gs.canopy.g, gs.canopy.b, 0.9f});
        font_.drawShadowed("WASD STEER", -font_.textWidth("WASD STEER", CWS)*0.5f, -0.86f,
                           CWS, CHS, {0.9f, 0.9f, 0.9f, 0.7f});
    }

    if (settings_.showFPS) {
        std::string fps = std::to_string(int(fps_)) + " FPS";
        font_.drawShadowed(fps, -0.945f, 0.73f, CWS, CHS, {0.6f, 1.0f, 0.6f, 0.80f});
    }
}

// ── Render ────────────────────────────────────────────────────────────────────

void Game::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (state_ == GameState::Menu) {
        glDisable(GL_DEPTH_TEST);
        renderMenu();
        return;
    }
    if (state_ == GameState::BusFlight) {
        renderBusFlight();
        return;
    }

    float     aspect = height_ > 0 ? float(width_) / float(height_) : 16.0f / 9.0f;
    // ADS: zoom FOV per weapon type
    float savedFov = camera_.fov;
    if (isAiming_ && !player_.pickaxeActive) {
        switch (player_.weapons[player_.currentWeapon].type) {
            case WeaponType::Sniper:  camera_.fov = settings_.fov * 0.22f; break;
            case WeaponType::Rifle:   camera_.fov = settings_.fov * 0.48f; break;
            case WeaponType::SMG:     camera_.fov = settings_.fov * 0.65f; break;
            default:                  camera_.fov = settings_.fov * 0.75f; break;
        }
    }
    glm::mat4 proj   = camera_.projection(aspect);
    camera_.fov = savedFov;
    // TPS: use clipped view to avoid camera clipping through buildings
    glm::mat4 view   = (camera_.mode == CameraMode::TPS)
                       ? camera_.viewClipped(player_, world_)
                       : camera_.view(player_);
    glm::vec3 eye    = (camera_.mode == CameraMode::TPS)
                       ? camera_.positionWithClip(player_, world_)
                       : camera_.position(player_);

    glEnable(GL_DEPTH_TEST);
    worldShader_.use();
    worldShader_.setMat4("uView", view);
    worldShader_.setMat4("uProj", proj);
    world_.render(worldShader_, eye);
    if (!isArena_) zone_.render(worldShader_, kLightDir, eye);
    botManager_.render(worldShader_, kLightDir, eye);

    if (camera_.mode == CameraMode::TPS && !playerDead_)
        botManager_.renderPlayerCharacter(worldShader_, kLightDir, eye,
                                          player_.position, playerFacing_, playerWalkPhase_, playerSkinIdx_);

    pickupManager_.render(worldShader_, kLightDir, eye);
    dropManager_.render(worldShader_, kLightDir, eye);

    effects_.renderTracers(proj * view);

    // ── First-person viewmodel ────────────────────────────────────────────────
    if (camera_.mode == CameraMode::FPS && !playerDead_ && !victoryAchieved_
        && state_ == GameState::Playing && !player_.pickaxeActive
        && player_.hasWeapon[player_.currentWeapon])
    {
        // Separate near clip to avoid z-fighting with the world
        glm::mat4 vmProj = glm::perspective(glm::radians(65.0f), aspect, 0.04f, 100.0f);
        glm::vec3 fwd    = camera_.forward();
        glm::vec3 rgt    = camera_.right();
        glm::vec3 camUp  = glm::normalize(glm::cross(rgt, fwd));

        // Gun root: lower-right of view, pulled in when aiming
        float aimShift = isAiming_ ? -0.18f : 0.0f;
        float bob      = sinf(playerWalkPhase_ * 2.0f) * 0.010f;
        glm::vec3 root = eye + fwd * 0.55f
                             + rgt * (0.30f + aimShift)
                             - camUp * (0.18f - bob);

        // Rotation matrix: align box local axes with camera axes
        glm::mat4 gunRot(glm::vec4(rgt,    0.0f),
                         glm::vec4(camUp,  0.0f),
                         glm::vec4(fwd,    0.0f),
                         glm::vec4(0,0,0,  1.0f));

        WeaponType wt = player_.weapons[player_.currentWeapon].type;
        static const glm::vec3 kGunCol[5] = {
            {0.50f,0.50f,0.52f},   // Pistol  – dark grey
            {0.22f,0.40f,0.20f},   // Rifle   – olive
            {0.44f,0.34f,0.22f},   // Shotgun – tan
            {0.18f,0.26f,0.52f},   // Sniper  – steel blue
            {0.42f,0.16f,0.50f},   // SMG     – purple
        };
        glm::vec3 gc = kGunCol[player_.currentWeapon];

        // Draw one oriented box: offset (fx=right, fy=up, fz=forward) from root
        auto gb2 = [&](float fx, float fy, float fz,
                       float sx, float sy, float sz, const glm::vec3& col) {
            glm::vec3 ctr = root + rgt*fx + camUp*fy + fwd*fz;
            glm::mat4 m   = glm::translate(glm::mat4(1.0f), ctr)
                          * gunRot
                          * glm::scale(glm::mat4(1.0f), {sx, sy, sz});
            worldShader_.setMat4  ("uModel", m);
            worldShader_.setVec3  ("uColor", col);
            worldShader_.setFloat ("uAlpha", 1.0f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        };

        worldShader_.use();
        worldShader_.setMat4 ("uProj",     vmProj);
        worldShader_.setMat4 ("uView",     view);
        worldShader_.setVec3 ("uLightDir", kLightDir);
        worldShader_.setVec3 ("uViewPos",  eye);
        world_.beginBoxRender();

        switch (wt) {
        case WeaponType::Pistol:
            gb2(0,  0,     0,      0.06f, 0.08f, 0.30f, gc);           // body
            gb2(0, -0.10f,-0.05f,  0.05f, 0.14f, 0.09f, gc*0.85f);    // grip
            gb2(0, -0.02f, 0.16f,  0.04f, 0.04f, 0.16f, gc*1.10f);    // barrel
            break;
        case WeaponType::Rifle:
            gb2(0,  0,     0,      0.06f, 0.07f, 0.58f, gc);           // body
            gb2(0, -0.10f,-0.10f,  0.05f, 0.17f, 0.13f, gc*0.85f);    // grip
            gb2(0, -0.02f,-0.22f,  0.05f, 0.06f, 0.20f, gc*0.90f);    // stock
            gb2(0, -0.02f, 0.28f,  0.04f, 0.04f, 0.14f, gc*1.15f);    // muzzle
            gb2(0, -0.06f, 0.06f,  0.05f, 0.07f, 0.20f, {0.14f,0.14f,0.16f}); // scope body
            gb2(0, -0.06f, 0.17f,  0.055f,0.04f, 0.04f, {0.06f,0.06f,0.10f}); // scope lens
            break;
        case WeaponType::Shotgun:
            gb2(0,  0,     0,      0.08f, 0.09f, 0.50f, gc);           // body
            gb2(0, -0.12f,-0.08f,  0.06f, 0.20f, 0.11f, gc*0.85f);    // grip
            gb2(0, -0.02f,-0.20f,  0.06f, 0.06f, 0.18f, gc*0.90f);    // stock
            gb2(0, -0.01f, 0.27f,  0.06f, 0.06f, 0.08f, {0.50f,0.40f,0.30f}); // muzzle
            gb2(0,  0.04f, 0.05f,  0.06f, 0.04f, 0.40f, gc*0.78f);    // pump rail
            break;
        case WeaponType::Sniper:
            gb2(0,  0,     0,      0.05f, 0.07f, 0.78f, gc);           // long body
            gb2(0, -0.10f,-0.14f,  0.05f, 0.17f, 0.12f, gc*0.85f);    // grip
            gb2(0, -0.02f,-0.30f,  0.05f, 0.05f, 0.24f, gc*0.90f);    // stock
            gb2(0, -0.06f, 0.10f,  0.05f, 0.09f, 0.36f, {0.10f,0.10f,0.12f}); // scope
            gb2(0, -0.06f, 0.28f,  0.055f,0.04f, 0.06f, {0.05f,0.05f,0.08f}); // scope front lens
            gb2(0, -0.06f,-0.08f,  0.055f,0.04f, 0.06f, {0.05f,0.05f,0.08f}); // scope rear lens
            gb2(0, -0.02f, 0.40f,  0.03f, 0.03f, 0.05f, gc*1.1f);     // suppressor
            break;
        case WeaponType::SMG:
            gb2(0,  0,     0,      0.06f, 0.08f, 0.42f, gc);           // body
            gb2(0, -0.10f,-0.05f,  0.05f, 0.16f, 0.10f, gc*0.85f);    // grip
            gb2(0,  0.05f,-0.05f,  0.04f, 0.22f, 0.05f, {0.32f,0.30f,0.28f}); // magazine
            gb2(0, -0.02f, 0.22f,  0.04f, 0.04f, 0.09f, gc*1.10f);    // barrel
            gb2(0, -0.05f, 0.06f,  0.04f, 0.06f, 0.14f, {0.12f,0.12f,0.15f}); // red-dot scope
            break;
        }

        world_.endBoxRender();
        // Restore projection for HUD
        worldShader_.setMat4("uProj", proj);
    }

    glDisable(GL_DEPTH_TEST);
    glm::mat4 projView = proj * view;
    dropManager_.renderLabels(player_.position, projView, font_, width_, height_);
    renderHUD(proj, view);
}

// ── Arena HUD ─────────────────────────────────────────────────────────────────

void Game::renderArenaHUD() {
    const float CWS = 0.019f, CHS = 0.033f;
    const float CWN = 0.022f, CHN = 0.039f;

    // ── Arena timer (top-center) ──────────────────────────────────────────────
    int secs = std::max(0, (int)arenaTimeLeft_);
    char timeBuf[32];
    std::snprintf(timeBuf, sizeof(timeBuf), "ARENA  %d:%02d", secs / 60, secs % 60);
    float pulse = (arenaTimeLeft_ < 30.0f) ? 0.6f + 0.4f * sinf(gameTime_ * 6.0f) : 1.0f;
    glm::vec4 timeCol = (arenaTimeLeft_ < 30.0f)
        ? glm::vec4(1.0f, 0.25f, 0.1f, pulse)
        : glm::vec4(0.20f, 1.0f, 0.45f, 0.95f);
    font_.drawShadowed(timeBuf, -font_.textWidth(timeBuf, CWN)*0.5f, 0.94f, CWN, CHN, timeCol);

    // ── Scoreboard panel (right side, below minimap) ──────────────────────────
    const float px  = 0.63f;
    float       py  = 0.50f;
    const float row = CHN + 0.012f;

    font_.drawShadowed("ARENA  SCORES", px, py, CWS*0.82f, CHS*0.82f, {0.15f, 0.88f, 1.0f, 0.80f});
    py -= row * 0.9f;

    // Collect bot kills into a sortable list
    struct Entry { int kills; bool isPlayer; int idx; };
    std::vector<Entry> entries;
    entries.push_back({arenaPlayerScore_, true, -1});
    for (int i = 0; i < botManager_.totalCount(); ++i)
        entries.push_back({botManager_.bots()[i].kills, false, i});
    std::sort(entries.begin(), entries.end(),
              [](const Entry& a, const Entry& b){ return a.kills > b.kills; });

    int shown = std::min((int)entries.size(), 8);
    for (int i = 0; i < shown; ++i) {
        const Entry& e = entries[i];
        char buf[48];
        if (e.isPlayer) {
            std::snprintf(buf, sizeof(buf), "YOU       %d", e.kills);
        } else {
            std::snprintf(buf, sizeof(buf), "BOT-%02d    %d", e.idx + 1, e.kills);
        }
        glm::vec4 col = e.isPlayer
            ? glm::vec4(1.0f, 0.92f, 0.10f, 0.95f)
            : glm::vec4(0.75f, 0.85f, 1.0f, 0.80f);
        if (i == 0) col.a = 1.0f;
        font_.drawShadowed(buf, px, py, CWS, CHS, col);
        py -= row;
    }

    // ── Respawn countdown when dead ───────────────────────────────────────────
    if (playerDead_) {
        int rs = std::max(0, 5 - (int)respawnTimer_);
        char rb[32];
        std::snprintf(rb, sizeof(rb), "RESPAWNING  %d", rs);
        font_.drawShadowed(rb, -font_.textWidth(rb, 0.048f)*0.5f, 0.0f,
                           0.048f, 0.084f, {1.0f, 0.55f, 0.10f, 0.95f});
    }

    // ── Difficulty tag ────────────────────────────────────────────────────────
    const DifficultyParams& dp = getDifficulty(difficulty_);
    char dBuf[32];
    std::snprintf(dBuf, sizeof(dBuf), "DIFF: %s", dp.name);
    font_.drawShadowed(dBuf, -0.97f, -0.92f, CWS*0.80f, CHS*0.80f, {0.60f,0.80f,1.0f,0.65f});
}

// ── Shared: 3-D character preview ────────────────────────────────────────────
// Renders into a sub-rectangle of the screen (NDC coords: x0,y0 = bottom-left,
// w/h in NDC units).  Caller must have GL depth test DISABLED before calling;
// this function enables/disables it internally.

void Game::renderCharPreview(float x0, float y0, float w, float h, int skinIdx) {
    if (skinIdx < 0) skinIdx = playerSkinIdx_;

    int px = (int)((x0 * 0.5f + 0.5f) * width_);
    int py = (int)((y0 * 0.5f + 0.5f) * height_);
    int pw = (int)(w * 0.5f * width_);
    int ph = (int)(h * 0.5f * height_);
    if (pw < 1 || ph < 1) return;

    glEnable(GL_SCISSOR_TEST);
    glScissor(px, py, pw, ph);
    glViewport(px, py, pw, ph);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    float angle  = gameTime_ * 0.55f;
    glm::vec3 cFwd{sinf(angle), 0.0f, cosf(angle)};
    float aspect = float(pw) / float(std::max(ph, 1));
    glm::mat4 pProj = glm::perspective(glm::radians(42.0f), aspect, 0.1f, 50.0f);
    glm::mat4 pView = glm::lookAt(glm::vec3(0,1.55f,5.2f), glm::vec3(0,1.25f,0), glm::vec3(0,1,0));
    worldShader_.use();
    worldShader_.setMat4("uProj", pProj);
    worldShader_.setMat4("uView", pView);
    botManager_.renderPlayerCharacter(worldShader_, kLightDir,
                                      glm::vec3(0,1.55f,5.2f), glm::vec3(0,0,0),
                                      cFwd, gameTime_ * 1.4f, skinIdx);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, width_, height_);
}

void Game::renderLobbyBackground() {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    float aspect = float(width_) / float(std::max(height_, 1));
    glm::mat4 bProj = glm::perspective(glm::radians(62.0f), aspect, 0.5f, 600.0f);
    // Slowly orbit above the town
    float angle = gameTime_ * 0.045f;
    glm::vec3 bEye(sinf(angle)*80.0f, 24.0f, cosf(angle)*80.0f);
    glm::mat4 bView = glm::lookAt(bEye, glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0,1,0));
    worldShader_.use();
    worldShader_.setMat4("uProj", bProj);
    worldShader_.setMat4("uView", bView);
    world_.render(worldShader_, bEye);
    glDisable(GL_DEPTH_TEST);
    // Dark blue-purple tint over world
    ui_.renderQuad(uiShader_, -1.0f, 1.0f, 2.0f, 2.0f, {0.01f, 0.03f, 0.12f, 0.60f});
}

// ── Glider grid (reused inside Locker) ───────────────────────────────────────

static void renderGliderGrid(UI& ui, Shader& sh, Font& font,
                              const std::vector<GliderSkin>& gliders,
                              int gliderIdx, float gameTime) {
    const float SW = 0.115f, SH = 0.095f, GAPX = 0.010f, GAPY = 0.010f;
    const float STRIDE_X = SW + GAPX, STRIDE_Y = SH + GAPY;
    const float START_X  = -0.90f, START_Y = 0.76f;
    for (int i = 0; i < 150; ++i) {
        int col = i % 15, row = i / 15;
        float sx = START_X + col * STRIDE_X;
        float sy = START_Y - row * STRIDE_Y;
        const GliderSkin& g = gliders[i];
        bool selected = (i == gliderIdx);
        ui.renderQuad(sh, sx, sy, SW, SH,
            selected ? glm::vec4(1.0f,0.90f,0.10f,0.25f)
                     : glm::vec4(0.03f,0.04f,0.08f,0.80f));
        ui.renderQuad(sh, sx+0.005f, sy-0.006f, SW-0.010f, SH-0.018f,
                      {g.canopy.r,g.canopy.g,g.canopy.b,1.0f});
        ui.renderQuad(sh, sx+0.005f, sy-SH+0.016f, SW-0.010f, 0.014f,
                      {g.stripe.r,g.stripe.g,g.stripe.b,1.0f});
        if (selected) {
            ui.renderQuad(sh, sx,           sy,            SW,    0.004f,{1,0.85f,0,1});
            ui.renderQuad(sh, sx,           sy-SH+0.004f,  SW,    0.004f,{1,0.85f,0,1});
            ui.renderQuad(sh, sx,           sy-SH+0.004f,  0.004f,SH-0.004f,{1,0.85f,0,1});
            ui.renderQuad(sh, sx+SW-0.004f, sy-SH+0.004f,  0.004f,SH-0.004f,{1,0.85f,0,1});
        }
    }
    const GliderSkin& sel = gliders[gliderIdx];
    const float CWT=0.034f,CHT=0.060f,CWH=0.026f,CHH=0.046f,CWS=0.019f,CHS=0.034f;
    font.drawShadowed("SELECT  GLIDER", -font.textWidth("SELECT  GLIDER",CWT)*0.5f, 0.91f,
                      CWT,CHT,{0.30f,0.90f,1.0f,1.0f});
    font.drawShadowed(sel.name, -font.textWidth(sel.name,CWH)*0.5f, -0.76f,
                      CWH,CHH,{sel.canopy.r,sel.canopy.g,sel.canopy.b,1.0f});
    char buf[32]; std::snprintf(buf,sizeof(buf),"%d / 150",gliderIdx+1);
    font.drawShadowed(buf,-font.textWidth(buf,CWS)*0.5f,-0.84f,CWS,CHS,{0.6f,0.8f,1.0f,0.7f});
    float pulse = 0.6f+0.4f*sinf(gameTime*3.0f);
    const char* nav="ARROWS browse  [ENTER] equip  [G] toggle skins/gliders";
    font.drawShadowed(nav,-font.textWidth(nav,CWS)*0.5f,-0.93f,CWS,CHS,{0.20f,1.0f,0.45f,pulse});
}

// ── Shared top nav bar ────────────────────────────────────────────────────────

static void renderTopBar(UI& ui, Shader& sh, Font& font,
                          MenuPage page, int dbucks, float gameTime,
                          float mouseNX=0.0f, float mouseNY=-2.0f) {
    // Background bar
    ui.renderQuad(sh, -1.0f, 1.0f, 2.0f, 0.155f, {0.01f,0.02f,0.08f,0.97f});
    ui.renderQuad(sh, -1.0f, 0.845f, 2.0f, 0.005f, {0.10f,0.75f,1.0f,0.60f});

    // Game title (left)
    const float CTW=0.028f,CTH=0.050f;
    font.drawShadowed("BATTLE ROYALE", -0.96f, 0.950f, CTW, CTH, {1.0f,0.88f,0.08f,1.0f});

    // D-Bucks (right)
    const float CWS=0.019f,CHS=0.034f;
    // D-Bucks icon (green square)
    ui.renderQuad(sh, 0.96f-0.038f, 0.960f, 0.034f, 0.034f, {0.15f,0.80f,0.35f,0.90f});
    char dbuf[32]; std::snprintf(dbuf,sizeof(dbuf),"%d",dbucks);
    font.drawShadowed(dbuf, 0.96f-0.038f-0.006f-font.textWidth(dbuf,CWS), 0.955f,
                      CWS, CHS, {0.25f,1.0f,0.55f,1.0f});

    // Tabs
    struct Tab { const char* label; MenuPage page; };
    static const Tab kTabs[] = {
        {"PLAY",     MenuPage::Main},
        {"LOCKER",   MenuPage::Locker},
        {"SHOP",     MenuPage::Shop},
        {"SETTINGS", MenuPage::Settings},
    };
    const int kTabCount = 4;
    const float TX=-0.30f, TW=0.15f, TH=0.055f, TGAP=0.018f;
    for (int i = 0; i < kTabCount; ++i) {
        bool active = (kTabs[i].page == page);
        float tabX = TX + i*(TW+TGAP);
        bool hovTab = (mouseNX>=tabX-0.008f && mouseNX<=tabX+TW+0.008f &&
                       mouseNY<=1.0f && mouseNY>=0.845f);
        if (active) {
            ui.renderQuad(sh, tabX-0.006f, 0.870f, TW+0.012f, TH+0.006f, {0.10f,0.55f,1.0f,0.35f});
            ui.renderQuad(sh, tabX-0.006f, 0.850f, TW+0.012f, 0.005f,    {0.15f,0.90f,1.0f,1.0f});
        } else if (hovTab) {
            ui.renderQuad(sh, tabX-0.006f, 0.870f, TW+0.012f, TH+0.006f, {0.10f,0.40f,0.80f,0.20f});
        }
        glm::vec4 tc = active ? glm::vec4(1.0f,0.95f,0.20f,1.0f)
                     : hovTab ? glm::vec4(0.90f,0.95f,1.0f,1.0f)
                              : glm::vec4(0.60f,0.75f,0.95f,0.70f);
        font.drawShadowed(kTabs[i].label, tabX, 0.875f, 0.020f, 0.036f, tc);
    }
    (void)gameTime;
}

// ── Menu screen ───────────────────────────────────────────────────────────────

void Game::renderMenu() {
    // ── 3D lobby background ─────────────────────────────────────────────────
    renderLobbyBackground();

    // Hover helper (used throughout)
    auto hov = [&](float x0, float topY, float w, float h) -> bool {
        return menuMouseNX_>=x0 && menuMouseNX_<=x0+w &&
               menuMouseNY_<=topY && menuMouseNY_>=topY-h;
    };

    const float CWS=0.018f, CHS=0.032f;
    const float CWN=0.022f, CHN=0.039f;
    const float CWH=0.028f, CHH=0.050f;

    // ── Shared top nav bar ────────────────────────────────────────────────────
    renderTopBar(ui_, uiShader_, font_, menuPage_, saveData_.dbucks, gameTime_, menuMouseNX_, menuMouseNY_);

    // ── Difficulty page ───────────────────────────────────────────────────────
    if (menuPage_ == MenuPage::Difficulty) {
        static const char* kDiffDesc[6]={
            "Forgiving aim — good for newcomers",
            "Balanced challenge — standard play",
            "Alert enemies — fast reflexes",
            "Near-perfect aim — relentless",
            "Elite soldiers — no mercy",
            "MAXIMUM LETHALITY — not recommended",
        };
        static const glm::vec3 kDiffCol[6]={
            {0.30f,1.0f,0.40f},{0.60f,1.0f,0.35f},{1.0f,0.85f,0.20f},
            {1.0f,0.55f,0.10f},{1.0f,0.22f,0.10f},{1.0f,0.10f,0.55f},
        };
        ui_.renderQuad(uiShader_,-0.72f,0.78f,1.44f,1.62f,{0.0f,0.03f,0.10f,0.96f});
        ui_.renderQuad(uiShader_,-0.72f,0.78f,1.44f,0.005f,{0.1f,0.8f,1.0f,0.8f});
        ui_.renderQuad(uiShader_,-0.72f,-0.84f+0.005f,1.44f,0.005f,{0.1f,0.8f,1.0f,0.5f});
        font_.drawShadowed("SELECT  DIFFICULTY",
                           -font_.textWidth("SELECT  DIFFICULTY",CWH)*0.5f,0.68f,
                           CWH,CHH,{0.15f,0.90f,1.0f,1.0f});
        const float rowY0=0.46f, rowGap=0.19f;
        for (int i=0;i<6;++i){
            bool sel=(i==(int)difficulty_);
            float ry=rowY0-i*rowGap;
            glm::vec3 dc=kDiffCol[i];
            bool hovRow=hov(-0.68f,ry+CHN*0.6f,1.36f,CHN+0.018f);
            if(sel||hovRow) ui_.renderQuad(uiShader_,-0.68f,ry+CHN*0.6f,1.36f,CHN+0.018f,
                              {0.08f,0.40f,0.85f,sel?0.30f:0.15f});
            float da=sel?1.0f:(hovRow?0.85f:0.55f);
            const DifficultyParams& dp=getDifficulty(static_cast<Difficulty>(i));
            std::string label=(sel?"▶ ":"  ")+std::string(dp.name);
            font_.drawShadowed(label.c_str(),-0.64f,ry,CWN,CHN,{dc.r,dc.g,dc.b,da});
            font_.drawShadowed(kDiffDesc[i],-0.58f,ry-CHN*0.78f,CWS*0.82f,CHS*0.82f,
                sel?glm::vec4(0.80f,0.90f,1.0f,0.90f):glm::vec4(0.50f,0.60f,0.75f,0.55f));
        }
        bool hovBack=hov(-0.20f,-0.76f,0.40f,0.08f);
        ui_.renderQuad(uiShader_,-0.20f,-0.76f,0.40f,0.08f,{0.0f,0.05f,0.15f,hovBack?0.80f:0.55f});
        ui_.renderQuad(uiShader_,-0.20f,-0.76f,0.40f,0.004f,{0.3f,0.6f,1.0f,0.8f});
        font_.drawShadowed("BACK",-font_.textWidth("BACK",CWS)*0.5f,-0.783f,CWS,CHS,{0.7f,0.85f,1.0f,1.0f});
        font_.drawShadowed("CLICK row to set  •  [ESC] back",
                           -font_.textWidth("CLICK row to set  •  [ESC] back",CWS)*0.5f,-0.86f,
                           CWS,CHS,{0.45f,0.65f,1.0f,0.55f});
        return;
    }

    // ── Settings page ─────────────────────────────────────────────────────────
    if (menuPage_ == MenuPage::Settings) {
        ui_.renderQuad(uiShader_,-0.65f,0.76f,1.30f,1.40f,{0.0f,0.03f,0.12f,0.96f});
        ui_.renderQuad(uiShader_,-0.65f,0.76f,1.30f,0.005f,{0.1f,0.8f,1.0f,0.8f});
        ui_.renderQuad(uiShader_,-0.65f,-0.64f+0.005f,1.30f,0.005f,{0.1f,0.8f,1.0f,0.5f});
        font_.drawShadowed("SYSTEM  CONFIG",-font_.textWidth("SYSTEM  CONFIG",CWH)*0.5f,
                           0.64f,CWH,CHH,{0.15f,0.90f,1.0f,1.0f});
        static const char* kLabels[3]={"SENSITIVITY","FIELD OF VIEW","SHOW FPS"};
        const float rowY[3]={0.42f,0.18f,-0.06f};
        for(int i=0;i<3;++i){
            bool sel=(i==settingIdx_);
            bool hovRow=hov(-0.60f,rowY[i]+CHN*0.6f,1.20f,CHN+0.02f);
            glm::vec4 lc=sel?glm::vec4(1,0.92f,0.10f,1):(hovRow?glm::vec4(1,1,1,0.95f):glm::vec4(0.70f,0.85f,1.0f,0.80f));
            if(sel||hovRow) ui_.renderQuad(uiShader_,-0.60f,rowY[i]+CHN*0.6f,1.20f,CHN+0.02f,{0.1f,0.5f,0.9f,sel?0.22f:0.12f});
            font_.drawShadowed(kLabels[i],-0.56f,rowY[i],CWN,CHN,lc);
            std::string val;
            if(i==0){char b[16];std::snprintf(b,16,"%.2f",settings_.sensitivity);val=b;}
            else if(i==1) val=std::to_string(int(settings_.fov));
            else val=settings_.showFPS?"ON":"OFF";
            std::string row="◀  "+val+"  ▶";
            float rvx=0.56f-font_.textWidth(row,CWN);
            font_.drawShadowed(row.c_str(),rvx,rowY[i],CWN,CHN,
                sel?glm::vec4(1,0.92f,0.1f,1):glm::vec4(0.8f,0.9f,1.0f,0.85f));
        }
        bool hovBack2=hov(-0.20f,-0.52f,0.40f,0.08f);
        ui_.renderQuad(uiShader_,-0.20f,-0.52f,0.40f,0.08f,{0.0f,0.05f,0.15f,hovBack2?0.80f:0.55f});
        ui_.renderQuad(uiShader_,-0.20f,-0.52f,0.40f,0.004f,{0.3f,0.6f,1.0f,0.8f});
        font_.drawShadowed("BACK",-font_.textWidth("BACK",CWS)*0.5f,-0.543f,CWS,CHS,{0.7f,0.85f,1.0f,1.0f});
        font_.drawShadowed("UP/DOWN select   ◀▶ adjust   [ESC] back",
                           -font_.textWidth("UP/DOWN select   ◀▶ adjust   [ESC] back",CWS)*0.5f,-0.68f,
                           CWS,CHS,{0.45f,0.65f,1.0f,0.55f});
        return;
    }

    // ── PLAY page ─────────────────────────────────────────────────────────────
    if (menuPage_ == MenuPage::Main) {
        // Large centered character (75% wide)
        renderCharPreview(-1.0f, -1.0f, 1.50f, 1.848f);

        // Platform glow beneath character
        float glow = 0.45f + 0.35f*sinf(gameTime_*1.8f);
        ui_.renderQuad(uiShader_, -0.55f, -0.68f, 1.10f, 0.10f, {0.10f,0.55f,1.0f, glow*0.18f});
        ui_.renderQuad(uiShader_, -0.38f, -0.69f, 0.76f, 0.06f, {0.15f,0.75f,1.0f, glow*0.32f});
        ui_.renderQuad(uiShader_, -0.24f, -0.70f, 0.48f, 0.04f, {0.20f,0.90f,1.0f, glow*0.55f});

        // Bottom-left skin info panel (overlaid on character area)
        ui_.renderQuad(uiShader_, -0.98f, -0.62f, 0.82f, 0.32f, {0.0f,0.02f,0.08f,0.82f});
        ui_.renderQuad(uiShader_, -0.98f, -0.62f, 0.82f, 0.004f, {0.10f,0.65f,1.0f,0.60f});
        const Skin& sk = kSkins[playerSkinIdx_];
        font_.drawShadowed(sk.name, -0.92f, -0.67f, CWH, CHH, {1.0f,0.92f,0.10f,1.0f});
        // Style badge
        const char* styleName = skinStyleName(sk.style);
        float bw = font_.textWidth(styleName, CWS)+0.040f;
        ui_.renderQuad(uiShader_, -0.92f, -0.76f, bw, 0.040f, {0.08f,0.30f,0.70f,0.90f});
        ui_.renderQuad(uiShader_, -0.92f, -0.76f, bw, 0.003f, {0.15f,0.60f,1.0f,0.80f});
        font_.drawShadowed(styleName, -0.90f, -0.768f, CWS, CHS, {0.65f,0.90f,1.0f,1.0f});
        // Skin nav arrows
        bool hovL=hov(-0.98f,-0.80f,0.10f,0.10f), hovR=hov(-0.16f,-0.80f,0.10f,0.10f);
        ui_.renderQuad(uiShader_,-0.98f,-0.80f,0.10f,0.10f,{0.05f,0.20f,0.50f,hovL?0.70f:0.40f});
        ui_.renderQuad(uiShader_,-0.16f,-0.80f,0.10f,0.10f,{0.05f,0.20f,0.50f,hovR?0.70f:0.40f});
        font_.drawShadowed("◀",-0.975f,-0.808f, CWN,CHN,{1,1,1,hovL?1.0f:0.70f});
        font_.drawShadowed("▶",-0.155f,-0.808f, CWN,CHN,{1,1,1,hovR?1.0f:0.70f});

        // Right stats panel (overlapping right edge)
        ui_.renderQuad(uiShader_, 0.53f, 0.82f, 0.45f, 1.70f, {0.0f,0.02f,0.08f,0.88f});
        ui_.renderQuad(uiShader_, 0.53f, 0.82f, 0.45f, 0.004f, {0.10f,0.65f,1.0f,0.50f});
        const DifficultyParams& dp = getDifficulty(difficulty_);
        static const glm::vec3 kDiffCol[6]={
            {0.30f,1.0f,0.40f},{0.60f,1.0f,0.35f},{1.0f,0.85f,0.20f},
            {1.0f,0.55f,0.10f},{1.0f,0.22f,0.10f},{1.0f,0.10f,0.55f},
        };
        glm::vec3 dc = kDiffCol[(int)difficulty_];
        char buf[64];
        std::snprintf(buf,sizeof(buf),"KILLS\n%d", saveData_.totalKills);
        font_.drawShadowed("KILLS", 0.60f, 0.74f, CWS, CHS, {0.60f,0.78f,1.0f,0.70f});
        font_.drawShadowed(std::to_string(saveData_.totalKills).c_str(), 0.60f, 0.68f, CWN, CHN, {1,1,1,1});
        ui_.renderQuad(uiShader_, 0.55f, 0.60f, 0.40f, 0.003f, {0.10f,0.50f,1.0f,0.30f});
        font_.drawShadowed("DIFFICULTY", 0.60f, 0.54f, CWS, CHS, {0.60f,0.78f,1.0f,0.70f});
        font_.drawShadowed(dp.name, 0.60f, 0.48f, CWN, CHN, {dc.r,dc.g,dc.b,1.0f});
        ui_.renderQuad(uiShader_, 0.55f, 0.40f, 0.40f, 0.003f, {0.10f,0.50f,1.0f,0.30f});
        font_.drawShadowed("D-BUCKS", 0.60f, 0.34f, CWS, CHS, {0.60f,0.78f,1.0f,0.70f});
        font_.drawShadowed(std::to_string(saveData_.dbucks).c_str(), 0.60f, 0.28f, CWN, CHN, {0.25f,1.0f,0.55f,1.0f});
        ui_.renderQuad(uiShader_, 0.55f, 0.20f, 0.40f, 0.003f, {0.10f,0.50f,1.0f,0.30f});

        // Controls (compact, in stats panel)
        static const char* kHints[]={
            "WASD+Mouse  Move/Look",
            "Space Jump  Shift Sprint",
            "LMB/[F] Shoot  [R] Reload",
            "[G] Heal  [Q] Dash  [E] Grapple",
            "[V] FPS/TPS  [I] Inventory",
        };
        float hy=0.12f;
        for(auto* h:kHints){
            font_.drawShadowed(h,0.57f,hy,CWS*0.80f,CHS*0.80f,{0.65f,0.78f,1.0f,0.75f});
            hy -= CHS*0.80f+0.012f;
        }

        // Arena button (in stats panel)
        bool hovArena=hov(0.55f,-0.56f,0.40f,0.075f);
        ui_.renderQuad(uiShader_,0.55f,-0.56f,0.40f,0.075f,{0.20f,0.10f,0.02f,hovArena?0.85f:0.60f});
        ui_.renderQuad(uiShader_,0.55f,-0.56f,0.40f,0.003f,{1.0f,0.60f,0.05f,0.90f});
        font_.drawShadowed("ARENA",0.62f,-0.575f,CWS,CHS,{1.0f,0.68f,0.10f,1.0f});
        font_.drawShadowed("MODE",0.62f,-0.610f,CWS*0.82f,CHS*0.82f,{0.80f,0.55f,0.08f,0.85f});

        // [D] DIFFICULTY shortcut
        bool hovDiff=hov(0.55f,-0.64f,0.40f,0.055f);
        ui_.renderQuad(uiShader_,0.55f,-0.64f,0.40f,0.055f,{0.02f,0.08f,0.20f,hovDiff?0.80f:0.55f});
        ui_.renderQuad(uiShader_,0.55f,-0.64f,0.40f,0.002f,{0.1f,0.5f,1.0f,0.6f});
        font_.drawShadowed("[D] DIFFICULTY",0.58f,-0.648f,CWS*0.82f,CHS*0.82f,{0.5f,0.75f,1.0f,0.85f});

        // PLAY button (large, centered at bottom)
        bool hovPlay = hov(-0.30f,-0.80f,0.60f,0.16f);
        float pulse = hovPlay ? 1.0f : (0.55f+0.45f*sinf(gameTime_*3.0f));
        glm::vec4 playFill = hovPlay ? glm::vec4(0.15f,0.85f,0.25f,0.90f) : glm::vec4(0.08f,0.55f,0.15f,0.75f);
        ui_.renderQuad(uiShader_,-0.30f,-0.96f,0.60f,0.16f, playFill);
        ui_.renderQuad(uiShader_,-0.30f,-0.80f,0.60f,0.005f,{0.25f,1.0f,0.40f,pulse});
        ui_.renderQuad(uiShader_,-0.30f,-0.96f,0.60f,0.005f,{0.25f,1.0f,0.40f,pulse*0.6f});
        ui_.renderQuad(uiShader_,-0.30f,-0.96f,0.005f,0.16f, {0.25f,1.0f,0.40f,pulse*0.5f});
        ui_.renderQuad(uiShader_, 0.295f,-0.96f,0.005f,0.16f, {0.25f,1.0f,0.40f,pulse*0.5f});
        font_.drawShadowed("PLAY", -font_.textWidth("PLAY",0.048f)*0.5f,-0.858f, 0.048f,0.084f,{0.20f,1.0f,0.40f,pulse});
        return;
    }

    // ── LOCKER page ──────────────────────────────────────────────────────────
    if (menuPage_ == MenuPage::Locker) {
        if (lockerGlider_) {
            renderGliderGrid(ui_, uiShader_, font_, getGliders(), gliderIdx_, gameTime_);
            font_.drawShadowed("CLICK to select  •  [ESC] back",
                               -font_.textWidth("CLICK to select  •  [ESC] back",CWS)*0.5f,-0.96f,
                               CWS,CHS,{0.50f,0.70f,0.95f,0.65f});
            return;
        }

        // Left: character preview (48% of width)
        renderCharPreview(-1.0f, -1.0f, 0.96f, 1.848f);

        // Platform glow
        float glow = 0.45f + 0.35f*sinf(gameTime_*1.8f);
        ui_.renderQuad(uiShader_, -0.42f, -0.68f, 0.84f, 0.08f, {0.10f,0.55f,1.0f, glow*0.20f});
        ui_.renderQuad(uiShader_, -0.28f, -0.70f, 0.56f, 0.04f, {0.20f,0.90f,1.0f, glow*0.45f});

        // Equipped skin name below character
        {
            const Skin& sk=kSkins[playerSkinIdx_];
            ui_.renderQuad(uiShader_,-0.97f,-0.74f,0.92f,0.20f,{0.0f,0.02f,0.08f,0.82f});
            ui_.renderQuad(uiShader_,-0.97f,-0.74f,0.92f,0.003f,{0.10f,0.65f,1.0f,0.50f});
            font_.drawShadowed(sk.name,-0.90f,-0.78f,CWH,CHH,{1.0f,0.92f,0.10f,1.0f});
            const char* equipped="EQUIPPED";
            ui_.renderQuad(uiShader_,-0.90f,-0.82f,font_.textWidth(equipped,CWS)+0.03f,0.038f,{0.05f,0.35f,0.12f,0.90f});
            font_.drawShadowed(equipped,-0.88f,-0.827f,CWS,CHS,{0.20f,1.0f,0.45f,1.0f});
        }

        // Right: grid panel
        const int owned = (int)saveData_.ownedSkins.size();
        const int kCols = 4;
        const float TW_SK=0.185f, TH_SK=0.180f, TGAP_SK=0.016f;
        const float GX0=-0.02f, GY0=0.72f;

        // Panel bg
        ui_.renderQuad(uiShader_,-0.04f,0.84f,1.02f,1.72f,{0.01f,0.03f,0.10f,0.88f});
        ui_.renderQuad(uiShader_,-0.04f,0.84f,1.02f,0.005f,{0.10f,0.65f,1.0f,0.60f});
        font_.drawShadowed("LOCKER",-0.00f,0.77f,CWH,CHH,{0.15f,0.88f,1.0f,1.0f});

        // Gliders button
        bool hovG=hov(-0.02f,0.66f,0.28f,0.040f);
        ui_.renderQuad(uiShader_,-0.02f,0.66f-0.040f,0.28f,0.040f,{0.05f,0.15f,0.35f,hovG?0.85f:0.60f});
        ui_.renderQuad(uiShader_,-0.02f,0.66f,0.28f,0.003f,{0.15f,0.50f,1.0f,0.70f});
        font_.drawShadowed("GLIDERS",-0.00f,0.649f,CWS,CHS,{0.50f,0.78f,1.0f,0.90f});

        for(int i=0;i<owned;++i){
            int col=i%kCols, row=i/kCols;
            float sx=GX0+col*(TW_SK+TGAP_SK);
            float sy=GY0-0.08f-row*(TH_SK+TGAP_SK);
            int skinIdx=saveData_.ownedSkins[i];
            bool sel=(skinIdx==playerSkinIdx_);
            bool hovCard=hov(sx,sy,TW_SK,TH_SK);
            const Skin& sk=kSkins[skinIdx];

            // Card bg
            glm::vec4 cardBg = sel ? glm::vec4(0.12f,0.42f,0.88f,0.40f)
                             : hovCard ? glm::vec4(0.08f,0.22f,0.50f,0.30f)
                             : glm::vec4(0.03f,0.05f,0.12f,0.88f);
            ui_.renderQuad(uiShader_,sx,sy,TW_SK,TH_SK,cardBg);
            // Big torso color fill (skin preview)
            ui_.renderQuad(uiShader_,sx+0.007f,sy-0.010f,TW_SK-0.014f,TH_SK*0.62f,
                           {sk.torso.r,sk.torso.g,sk.torso.b,1.0f});
            // Pants color (lower half)
            ui_.renderQuad(uiShader_,sx+0.007f,sy-TH_SK*0.38f-0.010f,TW_SK-0.014f,TH_SK*0.26f,
                           {sk.pants.r,sk.pants.g,sk.pants.b,1.0f});
            // Accent stripe at very bottom of swatch
            ui_.renderQuad(uiShader_,sx+0.007f,sy-TH_SK+0.030f,TW_SK-0.014f,0.016f,
                           {sk.accent.r,sk.accent.g,sk.accent.b,1.0f});
            // Skin name label (bottom strip)
            ui_.renderQuad(uiShader_,sx,sy-TH_SK+0.028f,TW_SK,0.028f,{0.0f,0.02f,0.08f,0.88f});
            font_.drawShadowed(sk.name,sx+0.007f,sy-TH_SK+0.024f,CWS*0.70f,CHS*0.70f,{1,1,1,0.90f});
            // Border
            glm::vec4 bc = sel ? glm::vec4(1.0f,0.90f,0.10f,1.0f)
                         : hovCard ? glm::vec4(0.40f,0.70f,1.0f,0.90f)
                         : glm::vec4(0.10f,0.30f,0.65f,0.45f);
            float bt=0.004f;
            ui_.renderQuad(uiShader_,sx,    sy,          TW_SK, bt, bc);
            ui_.renderQuad(uiShader_,sx,    sy-TH_SK+bt, TW_SK, bt, bc);
            ui_.renderQuad(uiShader_,sx,    sy-TH_SK+bt, bt, TH_SK-bt, bc);
            ui_.renderQuad(uiShader_,sx+TW_SK-bt,sy-TH_SK+bt,bt,TH_SK-bt,bc);
            // Equipped badge
            if(sel){
                ui_.renderQuad(uiShader_,sx+0.007f,sy-0.014f,TW_SK*0.60f,0.030f,{0.05f,0.35f,0.10f,0.90f});
                font_.drawShadowed("EQUIPPED",sx+0.010f,sy-0.016f,CWS*0.62f,CHS*0.62f,{0.20f,1.0f,0.45f,1.0f});
            }
        }
        // Bottom hint
        font_.drawShadowed("CLICK skin to equip  •  GLIDERS for glider select  •  [ESC] back",
                           -0.02f,-0.90f,CWS*0.76f,CHS*0.76f,{0.45f,0.65f,0.95f,0.60f});
        return;
    }

    // ── SHOP page ─────────────────────────────────────────────────────────────
    if (menuPage_ == MenuPage::Shop) {
        std::vector<int> forSale;
        for(int i=0;i<kSkinCount;++i)
            if(!saveData_.ownsSkin(i)) forSale.push_back(i);
        const int total=(int)forSale.size();

        if (total == 0) {
            renderCharPreview(-1.0f,-1.0f,2.0f,1.848f, playerSkinIdx_);
            font_.drawShadowed("YOU OWN ALL SKINS",
                               -font_.textWidth("YOU OWN ALL SKINS",CWH)*0.5f,0.0f,
                               CWH,CHH,{0.25f,1.0f,0.55f,0.95f});
            return;
        }

        // Left: live 3D preview of selected shop skin
        int previewSkin = forSale[std::min(shopSel_, total-1)];
        renderCharPreview(-1.0f,-1.0f,0.96f,1.848f, previewSkin);

        // Glow under preview
        float glow=0.45f+0.35f*sinf(gameTime_*1.8f);
        ui_.renderQuad(uiShader_,-0.42f,-0.68f,0.84f,0.08f,{0.10f,0.55f,1.0f,glow*0.20f});
        ui_.renderQuad(uiShader_,-0.28f,-0.70f,0.56f,0.04f,{0.20f,0.90f,1.0f,glow*0.45f});

        // Selected skin info (bottom left, overlaid on character)
        {
            const Skin& sk=kSkins[previewSkin];
            int price=skinPrice(previewSkin);
            bool canBuy=(saveData_.dbucks>=price);
            ui_.renderQuad(uiShader_,-0.97f,-0.62f,0.92f,0.32f,{0.0f,0.02f,0.08f,0.88f});
            ui_.renderQuad(uiShader_,-0.97f,-0.62f,0.92f,0.003f,{0.10f,0.65f,1.0f,0.50f});
            font_.drawShadowed(sk.name,-0.90f,-0.67f,CWH,CHH,{1.0f,0.92f,0.10f,1.0f});
            char pbuf[32]; std::snprintf(pbuf,sizeof(pbuf),"%d D-BUCKS",price);
            glm::vec4 pc=canBuy?glm::vec4(0.20f,1.0f,0.45f,1.0f):glm::vec4(1.0f,0.30f,0.20f,1.0f);
            font_.drawShadowed(pbuf,-0.90f,-0.76f,CWN,CHN,pc);

            // BUY button
            bool hovBuy=hov(-0.30f,-0.80f,0.60f,0.10f);
            glm::vec4 buyFill=canBuy
                ?(hovBuy?glm::vec4(0.15f,0.85f,0.25f,0.95f):glm::vec4(0.08f,0.55f,0.15f,0.80f))
                :glm::vec4(0.25f,0.08f,0.08f,0.70f);
            ui_.renderQuad(uiShader_,-0.30f,-0.90f,0.60f,0.10f,buyFill);
            ui_.renderQuad(uiShader_,-0.30f,-0.80f,0.60f,0.004f,canBuy?glm::vec4(0.25f,1.0f,0.40f,0.90f):glm::vec4(0.8f,0.2f,0.2f,0.5f));
            const char* buyTxt = canBuy ? "BUY NOW" : "NOT ENOUGH D-BUCKS";
            float bts = canBuy ? 0.026f : 0.018f;
            float bth = canBuy ? 0.046f : 0.032f;
            font_.drawShadowed(buyTxt,-font_.textWidth(buyTxt,bts)*0.5f,-0.835f,bts,bth,{1,1,1,1});
        }

        // Right: shop grid (4 cols)
        ui_.renderQuad(uiShader_,-0.04f,0.84f,1.02f,1.72f,{0.01f,0.03f,0.10f,0.88f});
        ui_.renderQuad(uiShader_,-0.04f,0.84f,1.02f,0.005f,{0.10f,0.65f,1.0f,0.60f});
        font_.drawShadowed("ITEM SHOP",-0.00f,0.77f,CWH,CHH,{0.15f,0.90f,1.0f,1.0f});
        char budBuf[32]; std::snprintf(budBuf,sizeof(budBuf),"Budget: %d D",saveData_.dbucks);
        font_.drawShadowed(budBuf,0.55f,0.77f,CWS,CHS,{0.25f,1.0f,0.55f,0.90f});

        const int kCols=4, kRows=3;
        const float TW_SH=0.220f, TH_SH=0.200f, TGAP_SH=0.014f;
        const float GX0=0.02f, GY0=0.72f;

        for(int vi=0;vi<kRows;++vi){
            int globalRow=shopScroll_+vi;
            for(int col=0;col<kCols;++col){
                int idx=globalRow*kCols+col;
                if(idx>=total) break;
                float sx=GX0+col*(TW_SH+TGAP_SH);
                float sy=GY0-vi*(TH_SH+TGAP_SH);
                int skinIdx=forSale[idx];
                bool sel=(idx==shopSel_);
                bool hovCard=hov(sx,sy,TW_SH,TH_SH);
                const Skin& sk=kSkins[skinIdx];
                int price=skinPrice(skinIdx);
                bool canBuy=(saveData_.dbucks>=price);

                glm::vec4 cardBg=sel?glm::vec4(0.12f,0.42f,0.88f,0.40f)
                               :hovCard?glm::vec4(0.08f,0.22f,0.50f,0.30f)
                               :glm::vec4(0.03f,0.05f,0.12f,0.88f);
                ui_.renderQuad(uiShader_,sx,sy,TW_SH,TH_SH,cardBg);
                // Torso fill
                ui_.renderQuad(uiShader_,sx+0.007f,sy-0.010f,TW_SH-0.014f,TH_SH*0.60f,
                               {sk.torso.r,sk.torso.g,sk.torso.b,1.0f});
                ui_.renderQuad(uiShader_,sx+0.007f,sy-TH_SH*0.38f-0.010f,TW_SH-0.014f,TH_SH*0.24f,
                               {sk.pants.r,sk.pants.g,sk.pants.b,1.0f});
                ui_.renderQuad(uiShader_,sx+0.007f,sy-TH_SH+0.032f,TW_SH-0.014f,0.016f,
                               {sk.accent.r,sk.accent.g,sk.accent.b,1.0f});
                // Name strip
                ui_.renderQuad(uiShader_,sx,sy-TH_SH+0.030f,TW_SH,0.030f,{0.0f,0.02f,0.08f,0.88f});
                font_.drawShadowed(sk.name,sx+0.007f,sy-TH_SH+0.026f,CWS*0.72f,CHS*0.72f,{1,1,1,0.90f});
                // Price pill
                char pbuf[16]; std::snprintf(pbuf,sizeof(pbuf),"%d",price);
                glm::vec4 pc=canBuy?glm::vec4(0.10f,0.65f,0.25f,0.90f):glm::vec4(0.50f,0.10f,0.10f,0.80f);
                float pw2=font_.textWidth(pbuf,CWS)+0.032f;
                ui_.renderQuad(uiShader_,sx+TW_SH-pw2-0.006f,sy-0.010f,pw2,0.038f,pc);
                font_.drawShadowed(pbuf,sx+TW_SH-pw2-0.002f,sy-0.014f,CWS,CHS,{1,1,1,1});
                // Border
                glm::vec4 bc=sel?glm::vec4(1.0f,0.88f,0.10f,1.0f)
                           :hovCard?glm::vec4(0.40f,0.70f,1.0f,0.90f)
                           :glm::vec4(0.10f,0.30f,0.65f,0.45f);
                float bt=0.004f;
                ui_.renderQuad(uiShader_,sx,    sy,          TW_SH,bt,bc);
                ui_.renderQuad(uiShader_,sx,    sy-TH_SH+bt, TW_SH,bt,bc);
                ui_.renderQuad(uiShader_,sx,    sy-TH_SH+bt, bt,TH_SH-bt,bc);
                ui_.renderQuad(uiShader_,sx+TW_SH-bt,sy-TH_SH+bt,bt,TH_SH-bt,bc);
            }
        }
        // Scroll buttons
        if(shopScroll_>0){
            bool hovU=hov(0.85f,0.34f,0.11f,0.07f);
            ui_.renderQuad(uiShader_,0.85f,0.34f-0.07f,0.11f,0.07f,{0.05f,0.15f,0.40f,hovU?0.80f:0.55f});
            font_.drawShadowed("▲",0.876f,0.322f,CWN,CHN,{0.7f,0.9f,1.0f,0.90f});
        }
        if((shopScroll_+kRows)*kCols<total){
            bool hovD=hov(0.85f,0.24f,0.11f,0.07f);
            ui_.renderQuad(uiShader_,0.85f,0.24f-0.07f,0.11f,0.07f,{0.05f,0.15f,0.40f,hovD?0.80f:0.55f});
            font_.drawShadowed("▼",0.876f,0.222f,CWN,CHN,{0.7f,0.9f,1.0f,0.90f});
        }
        font_.drawShadowed("CLICK skin to preview & buy  •  [ESC] back",
                           -0.02f,-0.94f,CWS*0.78f,CHS*0.78f,{0.45f,0.65f,0.95f,0.60f});
        return;
    }
}

// ── In-game HUD ───────────────────────────────────────────────────────────────

void Game::renderHUD(const glm::mat4& proj, const glm::mat4& view) {
    const float CWN = 0.024f, CHN = 0.042f;
    const float CWS = 0.019f, CHS = 0.034f;
    const float CWX = 0.015f, CHX = 0.027f;  // tiny label size

    // ─────────────────────────────────────────────────────────────────────────
    // Damage flash (screen border)
    float dmgA = effects_.damageFlashAlpha();
    if (dmgA > 0.0f) {
        const float T = 0.14f;
        float a = dmgA * 0.72f;
        ui_.renderQuad(uiShader_, -1.0f,    1.0f,      2.0f, T, {1,0.05f,0.05f,a});
        ui_.renderQuad(uiShader_, -1.0f,   -1.0f+T,    2.0f, T, {1,0.05f,0.05f,a});
        ui_.renderQuad(uiShader_, -1.0f,   -1.0f+T,    T, 2.0f-T*2, {1,0.05f,0.05f,a});
        ui_.renderQuad(uiShader_,  1.0f-T, -1.0f+T,    T, 2.0f-T*2, {1,0.05f,0.05f,a});
        ui_.renderQuad(uiShader_, -1.0f,    1.0f,      2.0f, 2.0f,   {0.8f,0,0,dmgA*0.12f});
    }

    // Water overlay (ocean = drowning, lake = swimming)
    if (playerInWater_ || playerInLake_) {
        float pulse = 0.30f + 0.18f * sinf(gameTime_ * 5.0f);
        ui_.renderQuad(uiShader_, -1.0f, 1.0f, 2.0f, 2.0f, {0.02f, 0.12f, 0.45f, pulse});
        const float T = 0.18f;
        glm::vec4 wc = {0.05f, 0.30f, 0.75f, 0.65f};
        ui_.renderQuad(uiShader_, -1.0f,    1.0f,      2.0f, T, wc);
        ui_.renderQuad(uiShader_, -1.0f,   -1.0f+T,    2.0f, T, wc);
        ui_.renderQuad(uiShader_, -1.0f,   -1.0f+T,    T, 2.0f-T*2, wc);
        ui_.renderQuad(uiShader_,  1.0f-T, -1.0f+T,    T, 2.0f-T*2, wc);
        if (playerInWater_) {
            const char* drown = "DROWNING";
            font_.drawShadowed(drown, -font_.textWidth(drown, 0.060f)*0.5f, 0.10f,
                               0.060f, 0.105f, {0.40f, 0.80f, 1.0f, 0.95f});
            const char* ret = "SWIM BACK TO LAND";
            font_.drawShadowed(ret, -font_.textWidth(ret, 0.022f)*0.5f, -0.02f,
                               0.022f, 0.038f, {0.75f, 0.90f, 1.0f, 0.85f});
        } else {
            const char* swim = "SWIMMING";
            font_.drawShadowed(swim, -font_.textWidth(swim, 0.060f)*0.5f, 0.10f,
                               0.060f, 0.105f, {0.40f, 0.90f, 1.0f, 0.95f});
        }
    }

    // ADS scope / vignette overlay
    if (isAiming_ && !player_.pickaxeActive) {
        WeaponType adsWt = player_.weapons[player_.currentWeapon].type;
        if (adsWt == WeaponType::Sniper) {
            // Full sniper scope: black panels around a circular window + crosshairs
            const float SR = 0.74f;   // scope radius in NDC
            const float SH = SR;      // scope is square-ish (aspect-corrected in practice)
            // Left / right solid black bars
            ui_.renderQuad(uiShader_, -1.0f,     1.0f, 1.0f - SR*0.5f, 2.0f, {0,0,0,1.0f});
            ui_.renderQuad(uiShader_,  SR*0.5f,  1.0f, 1.0f - SR*0.5f, 2.0f, {0,0,0,1.0f});
            // Top / bottom bars
            ui_.renderQuad(uiShader_, -SR*0.5f,  1.0f, SR, 1.0f - SH*0.5f, {0,0,0,0.96f});
            ui_.renderQuad(uiShader_, -SR*0.5f, -SH*0.5f, SR, 1.0f - SH*0.5f, {0,0,0,0.96f});
            // Scope circle border (thin black ring approximated by slightly larger dark quad)
            ui_.renderQuad(uiShader_, -SR*0.5f - 0.008f, SH*0.5f + 0.008f,
                           SR + 0.016f, SR + 0.016f, {0,0,0,0.50f});
            // Crosshair lines
            const float CW = 0.003f;
            ui_.renderQuad(uiShader_, -SR*0.5f, -CW*0.5f, SR,    CW, {0,0,0,0.80f}); // horizontal
            ui_.renderQuad(uiShader_, -CW*0.5f, -SH*0.5f, CW, SH,    {0,0,0,0.80f}); // vertical
            // Center dot
            ui_.renderQuad(uiShader_, -0.008f, -0.008f + 0.008f, 0.016f, 0.016f, {0,0,0,0.90f});
        } else if (adsWt == WeaponType::Rifle || adsWt == WeaponType::SMG) {
            // Rifle / SMG: vignette bars + clean dot reticle
            const float V = 0.28f;
            ui_.renderQuad(uiShader_, -1.0f,    1.0f,    2.0f, V,       {0,0,0,0.58f});
            ui_.renderQuad(uiShader_, -1.0f,   -1.0f+V,  2.0f, V,       {0,0,0,0.58f});
            ui_.renderQuad(uiShader_, -1.0f,   -1.0f+V,  V, 2.0f-V*2,   {0,0,0,0.58f});
            ui_.renderQuad(uiShader_,  1.0f-V, -1.0f+V,  V, 2.0f-V*2,   {0,0,0,0.58f});
            // Thin ADS crosshair
            float rLen = (adsWt == WeaponType::Rifle) ? 0.06f : 0.09f;
            float rGap = 0.018f;
            float rW   = 0.003f;
            ui_.renderQuad(uiShader_, -(rLen+rGap), -rW*0.5f,  rLen, rW, {1,1,1,0.85f}); // left
            ui_.renderQuad(uiShader_,  rGap,        -rW*0.5f,  rLen, rW, {1,1,1,0.85f}); // right
            ui_.renderQuad(uiShader_, -rW*0.5f, rGap,          rW, rLen, {1,1,1,0.85f}); // top
            ui_.renderQuad(uiShader_, -rW*0.5f, -(rLen+rGap),  rW, rLen, {1,1,1,0.85f}); // bottom
            ui_.renderQuad(uiShader_, -0.006f,  -0.006f+0.006f, 0.012f, 0.012f, {1,1,1,0.95f}); // dot
        } else {
            // Pistol / Shotgun: light vignette only
            const float V = 0.28f;
            ui_.renderQuad(uiShader_, -1.0f,    1.0f,    2.0f, V,     {0,0,0,0.48f});
            ui_.renderQuad(uiShader_, -1.0f,   -1.0f+V,  2.0f, V,     {0,0,0,0.48f});
            ui_.renderQuad(uiShader_, -1.0f,   -1.0f+V,  V, 2.0f-V*2, {0,0,0,0.48f});
            ui_.renderQuad(uiShader_,  1.0f-V, -1.0f+V,  V, 2.0f-V*2, {0,0,0,0.48f});
        }
    }

    // Muzzle flash
    float muzzA = effects_.muzzleFlashAlpha();
    if (muzzA > 0.0f && camera_.mode == CameraMode::FPS) {
        float r = 0.10f * muzzA;
        ui_.renderQuad(uiShader_, 0.18f-r, -0.60f, r*2.2f, r, {1,0.92f,0.4f,muzzA*0.95f});
        ui_.renderQuad(uiShader_, 0.10f-r*1.4f, -0.64f, r*3.0f, r*1.3f, {1,0.70f,0.2f,muzzA*0.50f});
    }

    // ─────────────────────────────────────────────────────────────────────────
    // BOTTOM-LEFT: Health + Shield panel (Fortnite style)
    // Panel background
    ui_.renderQuad(uiShader_, -1.000f, -0.685f, 0.520f, 0.335f, {0.0f,0.0f,0.0f,0.55f});
    ui_.renderQuad(uiShader_, -1.000f, -0.685f, 0.520f, 0.004f, {0.5f,0.5f,0.5f,0.4f}); // top edge

    // Shield bar (blue, on top)
    float shFrac = player_.shield / player_.maxShield;
    ui_.renderQuad(uiShader_, -0.980f, -0.730f, 0.480f, 0.040f, {0.05f,0.10f,0.25f,0.90f}); // bg
    ui_.renderQuad(uiShader_, -0.980f, -0.730f, 0.480f*shFrac, 0.040f, {0.20f,0.55f,1.00f,1.0f});
    // Shield text
    font_.drawShadowed("SH", -0.978f, -0.695f, CWX, CHX, {0.55f,0.80f,1.0f,0.9f});
    {
        std::string sv = std::to_string(int(player_.shield));
        font_.drawShadowed(sv, -0.510f - font_.textWidth(sv, CWN), -0.695f, CWN, CHN, {1,1,1,1});
    }

    // Health bar (green, below)
    float hpFrac = player_.health / 100.0f;
    ui_.renderQuad(uiShader_, -0.980f, -0.790f, 0.480f, 0.040f, {0.04f,0.18f,0.05f,0.90f}); // bg
    ui_.renderQuad(uiShader_, -0.980f, -0.790f, 0.480f*hpFrac, 0.040f, {0.08f,0.88f,0.18f,1.0f});
    // Health text
    font_.drawShadowed("HP", -0.978f, -0.756f, CWX, CHX, {0.55f,1.0f,0.65f,0.9f});
    {
        std::string hv = std::to_string(int(player_.health));
        font_.drawShadowed(hv, -0.510f - font_.textWidth(hv, CWN), -0.756f, CWN, CHN, {1,1,1,1});
    }

    // Heal key hint
    if (player_.medkits > 0 || player_.bandages > 0 || player_.shieldPotions > 0 || player_.miniShields > 0) {
        std::string ht = "[G] HEAL";
        if (player_.medkits       > 0) ht += "  MED x"  + std::to_string(player_.medkits);
        if (player_.bandages      > 0) ht += "  BND x"  + std::to_string(player_.bandages);
        if (player_.shieldPotions > 0) ht += "  SHLD x" + std::to_string(player_.shieldPotions);
        if (player_.miniShields   > 0) ht += "  MINI x" + std::to_string(player_.miniShields);
        font_.drawShadowed(ht, -0.978f, -0.844f, CWX, CHX, {1.0f,0.80f,0.35f,0.9f});
    }

    // ─────────────────────────────────────────────────────────────────────────
    // BOTTOM-CENTER: Weapon slots (5 slots)
    static const glm::vec3 kWepColors[5] = {
        {0.65f,0.65f,0.68f},  // PISTOL  - silver
        {0.25f,0.70f,0.28f},  // RIFLE   - green
        {0.78f,0.50f,0.22f},  // SHOTGUN - bronze
        {0.22f,0.48f,0.88f},  // SNIPER  - blue
        {0.65f,0.25f,0.78f},  // SMG     - purple
    };
    static const char* kWepNames[5] = { "PISTOL","RIFLE","SHOTGN","SNIPER","SMG" };

    const float slotW = 0.134f, slotH = 0.215f, slotGap = 0.016f;
    const float slotStride = slotW + slotGap;
    const float slotsStart = -(5.0f*slotW + 4.0f*slotGap) * 0.5f;
    const float slotBot    = -0.980f;

    for (int i = 0; i < 5; ++i) {
        bool owned  = player_.hasWeapon[i];
        bool active = !player_.pickaxeActive && (i == player_.currentWeapon) && owned;
        float sx = slotsStart + i * slotStride;
        float sy = slotBot + slotH;

        // Slot background: dimmer when empty
        glm::vec4 slotBg = active   ? glm::vec4(0.22f, 0.18f, 0.06f, 0.96f)
                         : owned    ? glm::vec4(0.06f, 0.06f, 0.10f, 0.85f)
                         :            glm::vec4(0.02f, 0.02f, 0.04f, 0.55f);
        ui_.renderQuad(uiShader_, sx, sy, slotW, slotH, slotBg);

        if (active) {
            ui_.renderQuad(uiShader_, sx, sy+0.002f, slotW, 0.006f, {1.0f,0.85f,0.0f,1.0f});
            ui_.renderQuad(uiShader_, sx, slotBot,    slotW, 0.004f, {1.0f,0.85f,0.0f,0.7f});
        } else {
            ui_.renderQuad(uiShader_, sx, sy+0.002f, slotW, 0.003f,
                           owned ? glm::vec4(0.4f,0.4f,0.5f,0.5f) : glm::vec4(0.2f,0.2f,0.2f,0.2f));
        }

        if (owned) {
            // Weapon color icon
            float iconW = slotW * 0.52f, iconH = 0.058f;
            float iconX = sx + (slotW - iconW) * 0.5f;
            float iconY = slotBot + slotH * 0.45f + iconH * 0.5f + 0.010f;
            const glm::vec3& wc = kWepColors[i];
            glm::vec4 wCol = active ? glm::vec4(wc.r, wc.g, wc.b, 1.0f)
                                    : glm::vec4(wc.r*0.55f, wc.g*0.55f, wc.b*0.55f, 0.85f);
            ui_.renderQuad(uiShader_, iconX, iconY+iconH, iconW, iconH, wCol);

            // Weapon name (show variant name if has nameIdx, else base name)
            const char* wepDispName = (player_.weapons[i].nameIdx > 0)
                ? getVariantName(player_.weapons[i].type, player_.weapons[i].nameIdx)
                : kWepNames[i];
            float wnw = font_.textWidth(wepDispName, CWX);
            font_.drawShadowed(wepDispName, sx+(slotW-wnw)*0.5f, slotBot+slotH*0.45f+0.004f,
                               CWX, CHX, active ? glm::vec4(1,1,1,1) : glm::vec4(0.55f,0.55f,0.6f,0.8f));

            // Ammo
            const Weapon& wpn = player_.weapons[i];
            std::string ammoStr = std::to_string(wpn.ammo) + "/" + std::to_string(wpn.reserve);
            float aw = font_.textWidth(ammoStr, CWX);
            font_.drawShadowed(ammoStr, sx+(slotW-aw)*0.5f, slotBot+0.006f,
                               CWX, CHX, active ? glm::vec4(1,0.88f,0.4f,1) : glm::vec4(0.5f,0.5f,0.5f,0.8f));
            if (active && wpn.reloading)
                font_.drawShadowed("RELOAD", sx+(slotW-font_.textWidth("RELOAD",CWX))*0.5f,
                                   slotBot+CHX+0.012f, CWX, CHX, {1,0.5f,0.1f,1});
        } else {
            // Empty slot indicator
            float ew = font_.textWidth("EMPTY", CWX);
            font_.drawShadowed("EMPTY", sx+(slotW-ew)*0.5f, slotBot+slotH*0.45f+0.004f,
                               CWX, CHX, {0.25f,0.25f,0.28f,0.50f});
        }
    }

    // Key hints above slots
    for (int i = 0; i < 5; ++i) {
        float sx = slotsStart + i * slotStride;
        bool active = !player_.pickaxeActive && (i == player_.currentWeapon) && player_.hasWeapon[i];
        const char* kNum[] = {"1","2","3","4","5"};
        float nw = font_.textWidth(kNum[i], CWX);
        font_.drawShadowed(kNum[i], sx+(slotW-nw)*0.5f, slotBot+slotH+0.006f,
                           CWX, CHX, active ? glm::vec4(1,0.85f,0,1) : glm::vec4(0.5f,0.5f,0.5f,0.7f));
    }

    // Pickaxe slot (left of weapon slots)
    {
        float psx = slotsStart - slotW - slotGap;
        float psy = slotBot + slotH;
        bool pActive = player_.pickaxeActive;
        glm::vec4 pBg = pActive ? glm::vec4(0.05f,0.18f,0.06f,0.95f) : glm::vec4(0.02f,0.04f,0.02f,0.60f);
        ui_.renderQuad(uiShader_, psx, psy, slotW, slotH, pBg);
        if (pActive) {
            ui_.renderQuad(uiShader_, psx, psy+0.002f, slotW, 0.006f, {0.25f,1.0f,0.35f,0.9f});
        }
        // Pickaxe icon: brown rectangle
        float icW = slotW*0.30f, icH = slotH*0.55f;
        float icX = psx+(slotW-icW)*0.5f, icY = slotBot+slotH*0.30f;
        ui_.renderQuad(uiShader_, icX, icY+icH, icW, icH, {0.62f,0.40f,0.18f, pActive?1.0f:0.5f});
        float pw = font_.textWidth("PICKAXE", CWX);
        font_.drawShadowed("PICKAXE", psx+(slotW-pw)*0.5f, slotBot+0.006f,
                           CWX, CHX, pActive ? glm::vec4(0.4f,1,0.5f,1) : glm::vec4(0.3f,0.5f,0.3f,0.6f));
        font_.drawShadowed("[X]", psx+(slotW-font_.textWidth("[X]",CWX))*0.5f, slotBot+slotH+0.006f,
                           CWX, CHX, pActive ? glm::vec4(0.25f,1,0.4f,1) : glm::vec4(0.35f,0.55f,0.35f,0.6f));
    }

    // Keybind hints below slots
    const std::string combatHint = player_.pickaxeActive
        ? "[LMB] MELEE  [X] PICKAXE  [1-5] EQUIP GUN"
        : "[TAB] CYCLE  [R] RELOAD  [F]/LMB SHOOT  [X] PICKAXE";
    font_.drawShadowed(combatHint, -font_.textWidth(combatHint, CWX)*0.5f, slotBot-0.012f,
                       CWX, CHX, {0.55f,0.70f,0.90f,0.70f});

    // ─────────────────────────────────────────────────────────────────────────
    // BOTTOM-RIGHT: utility keybind hint
    font_.drawShadowed("[V] FPS/TPS  [M] MAP", 0.630f, -0.700f,
                       CWX, CHX, {0.60f,0.75f,0.90f,0.75f});

    // Dash / Grapple indicators
    ui_.renderItems(uiShader_,
                    player_.dashCooldown,    Player::kDashCooldown,
                    player_.grappleCooldown, Player::kGrappleCooldown,
                    player_.grappleActive,
                    player_.medkits, player_.bandages);
    // Key labels next to dash/grapple icons
    font_.drawShadowed("[Q] DASH", 0.615f, -0.880f, CWX, CHX, {0.85f,0.85f,0.85f,0.8f});
    font_.drawShadowed("[E] INTERACT/GRAPPLE", 0.615f+0.09f+0.025f, -0.880f, CWX, CHX, {0.85f,0.85f,0.85f,0.8f});

    // ─────────────────────────────────────────────────────────────────────────
    // TOP-RIGHT: Player count + kill feed
    {
        int alive = botManager_.aliveCount() + (playerDead_ ? 0 : 1);
        int total = botManager_.totalCount() + 1;
        ui_.renderPlayerCount(uiShader_, alive, total);
        std::string cnt = std::to_string(alive) + "/" + std::to_string(total) + " ALIVE";
        font_.drawShadowed(cnt, 0.97f - font_.textWidth(cnt, CWS), 0.935f, CWS, CHS, {0.75f,0.92f,1.0f,0.95f});
    }

    // Kill feed — displayed just below the minimap
    {
        int kfc = effects_.killFeedCount();
        float kfy = 0.565f;   // just below minimap bottom (mmY0 = 0.590)
        const std::string kfStr = "BOT ELIMINATED";
        for (int i = kfc-1; i >= 0; --i) {
            float ka = effects_.killFeedAlpha(i);
            font_.drawShadowed(kfStr, 0.97f-font_.textWidth(kfStr,CWS), kfy, CWS, CHS, {1,0.45f,0.08f,ka});
            kfy -= CHS + 0.006f;
        }
    }

    // Game time (top center)
    {
        int mins = int(gameTime_) / 60;
        int secs = int(gameTime_) % 60;
        char tbuf[16];
        std::snprintf(tbuf, sizeof(tbuf), "%d:%02d", mins, secs);
        font_.drawShadowed(tbuf, -font_.textWidth(tbuf, CWS)*0.5f, 0.958f, CWS, CHS, {1,1,1,0.9f});
        // Final minute warning
        if (zone_.phase() >= 3) {
            float pulse = 0.7f + 0.3f * sinf(gameTime_ * 4.0f);
            font_.drawShadowed("FINAL ZONE - NO RESPAWN",
                               -font_.textWidth("FINAL ZONE - NO RESPAWN",CWX)*0.5f, 0.928f,
                               CWX, CHX, {1.0f,0.20f,0.10f,pulse});
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // TOP-CENTER: Zone info
    ui_.renderZoneBar(uiShader_, zone_.phaseTimeLeft(), zone_.phaseTimeFull(),
                      zone_.isShrinking(), zone_.phase());
    {
        std::string zs = (zone_.isShrinking()
            ? "ZONE " + std::to_string(zone_.phase()+1) + "  SHRINKING"
            : "ZONE " + std::to_string(zone_.phase()+1) + "  WAIT")
            + "  " + std::to_string(int(zone_.phaseTimeLeft())) + "s";
        font_.drawShadowed(zs, -font_.textWidth(zs,CWS)*0.5f, -0.938f, CWS, CHS, {1,1,1,0.9f});
    }

    if (!playerDead_ && !zone_.isInside(player_.position)) {
        ui_.renderZoneWarning(uiShader_, gameTime_);
        std::string dpsStr = "ZONE DMG  " + std::to_string(int(zone_.damagePerSec())) + "/s";
        font_.drawShadowed(dpsStr, -font_.textWidth(dpsStr,CWN)*0.5f, -0.60f, CWN, CHN, {1,0.15f,0.05f,0.95f});
    }

    // ─────────────────────────────────────────────────────────────────────────
    // CENTER: Crosshair
    {
        float aimSpread = isAiming_ ? crosshairSpread_ * 0.5f : crosshairSpread_;
        ui_.renderCrosshair(uiShader_, aimSpread, effects_.hitMarkerAlpha() > 0.0f);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Floating damage numbers
    {
        glm::mat4 pv = proj * view;
        for (const auto& dn : effects_.damageNumbers()) {
            glm::vec4 clip = pv * glm::vec4(dn.worldPos, 1.0f);
            if (clip.w < 0.01f) continue;
            float nx = clip.x / clip.w, ny = clip.y / clip.w;
            if (fabsf(nx) > 1.3f || fabsf(ny) > 1.3f) continue;

            float t      = 1.0f - dn.life / dn.maxLife;
            float disp_y = ny + t * 0.22f;
            float alpha  = dn.life / dn.maxLife;

            if (dn.headshot) {
                float cw = 0.052f, ch = 0.092f;
                std::string ds = std::to_string(int(dn.value));
                float dx = nx - font_.textWidth(ds, cw) * 0.5f;
                font_.drawShadowed(ds, dx, disp_y + ch*0.5f, cw, ch, {1,0.15f,0.10f,alpha});
                const std::string hs = "HEADSHOT";
                float hx = nx - font_.textWidth(hs, 0.022f) * 0.5f;
                font_.drawShadowed(hs, hx, disp_y+ch*0.5f+0.070f, 0.022f, 0.038f, {1,0.55f,0.10f,alpha});
            } else {
                std::string ds = std::to_string(int(dn.value));
                float dx = nx - font_.textWidth(ds, 0.033f) * 0.5f;
                font_.drawShadowed(ds, dx, disp_y+0.029f, 0.033f, 0.058f, {1,0.92f,0.15f,alpha});
            }
        }
    }

    // FPS counter
    if (settings_.showFPS) {
        std::string fps = std::to_string(int(fps_)) + " FPS";
        font_.drawShadowed(fps, -0.945f, 0.74f, CWX, CHX, {0.6f,1.0f,0.6f,0.80f});
    }

    // TOP-LEFT: Kill counter + D-Bucks (this match)
    if (!isArena_) {
        char killBuf[32];
        std::snprintf(killBuf, sizeof(killBuf), "KILLS  %d", gameKills_);
        font_.drawShadowed(killBuf, -0.945f, 0.680f, CWX, CHX, {1.0f,0.88f,0.20f,0.95f});
        char dbBuf[32];
        int projectedBucks = saveData_.dbucks + gameKills_ * 100;
        std::snprintf(dbBuf, sizeof(dbBuf), "D-BUCKS  %d", projectedBucks);
        font_.drawShadowed(dbBuf, -0.945f, 0.640f, CWX, CHX, {0.25f,1.0f,0.55f,0.90f});
    }

    // Camera mode indicator
    ui_.renderCameraIndicator(uiShader_, camera_.mode == CameraMode::FPS);

    // ─────────────────────────────────────────────────────────────────────────
    // MINIMAP  (top-right corner; [M] expands to full overlay)
    if (state_ == GameState::Playing) {
        const float kWH = 250.0f;

        float mmX0, mmY0, mmW, mmH;
        if (mapOpen_) {
            mmX0 = -0.82f; mmY0 = -0.82f; mmW = 1.64f; mmH = 1.64f;
        } else {
            mmX0 = 0.630f; mmY0 = 0.590f; mmW = 0.350f; mmH = 0.300f;
        }

        // World [-120,120] -> minimap NDC
        auto toMX = [&](float wx) -> float {
            return mmX0 + (wx + kWH) / (2.0f*kWH) * mmW;
        };
        auto toMY = [&](float wz) -> float {
            return mmY0 + mmH * (1.0f - (wz + kWH) / (2.0f*kWH));
        };

        // Background + border
        ui_.renderQuad(uiShader_, mmX0, mmY0+mmH, mmW, mmH,
                       mapOpen_ ? glm::vec4(0.02f,0.05f,0.10f,0.92f)
                                : glm::vec4(0.04f,0.08f,0.14f,0.78f));
        float bdr = mapOpen_ ? 0.006f : 0.003f;
        glm::vec4 bdrCol = {0.25f, 0.55f, 0.85f, 0.85f};
        ui_.renderQuad(uiShader_, mmX0,        mmY0+mmH,     mmW, bdr,  bdrCol);
        ui_.renderQuad(uiShader_, mmX0,        mmY0,         mmW, bdr,  bdrCol);
        ui_.renderQuad(uiShader_, mmX0,        mmY0,         bdr, mmH,  bdrCol);
        ui_.renderQuad(uiShader_, mmX0+mmW-bdr,mmY0,         bdr, mmH,  bdrCol);

        // Zone ring (approximated with dots)
        {
            glm::vec2 zc = zone_.center();
            float     zr = zone_.radius();
            float zmx = toMX(zc.x);
            float zmy = toMY(zc.y);
            float zmrx = zr / (2.0f*kWH) * mmW;
            float zmry = zr / (2.0f*kWH) * mmH;
            int N = mapOpen_ ? 72 : 36;
            float ds = mapOpen_ ? 0.007f : 0.004f;
            for (int i = 0; i < N; ++i) {
                float ang = i * 6.28318f / N;
                float px = zmx + zmrx * cosf(ang);
                float py = zmy + zmry * sinf(ang);
                if (px < mmX0+bdr || px > mmX0+mmW-bdr ||
                    py < mmY0+bdr || py > mmY0+mmH-bdr) continue;
                ui_.renderQuad(uiShader_, px-ds*0.5f, py+ds*0.5f, ds, ds,
                               {0.22f,0.60f,1.0f,0.80f});
            }
        }

        // Buildings (skip terrain — hills/mountains shown as ground, not structures)
        float bldS = mapOpen_ ? 0.011f : 0.006f;
        for (const Building& b : world_.buildings()) {
            if (b.isTerrain) continue;
            float bx = toMX(b.center.x);
            float by = toMY(b.center.z);
            if (bx < mmX0+bdr || bx > mmX0+mmW-bdr ||
                by < mmY0+bdr || by > mmY0+mmH-bdr) continue;
            ui_.renderQuad(uiShader_, bx-bldS*0.5f, by+bldS*0.5f, bldS, bldS,
                           {b.wallColor.r*0.85f, b.wallColor.g*0.85f, b.wallColor.b*0.85f, 0.90f});
        }

        // POI markers + labels
        float poiDot = mapOpen_ ? 0.013f : 0.007f;
        for (int i = 0; i < World::kPoiCount; ++i) {
            float px = toMX(World::kPois[i].wx);
            float py = toMY(World::kPois[i].wz);
            if (px < mmX0+bdr || px > mmX0+mmW-bdr ||
                py < mmY0+bdr || py > mmY0+mmH-bdr) continue;
            ui_.renderQuad(uiShader_, px-poiDot*0.5f, py+poiDot*0.5f, poiDot, poiDot,
                           {1.0f,0.85f,0.25f,0.95f});
            if (mapOpen_) {
                const char* nm = World::kPois[i].name;
                float nw = font_.textWidth(nm, CWX);
                font_.drawShadowed(nm, px-nw*0.5f, py+poiDot+0.004f,
                                   CWX, CHX, {1.0f,0.88f,0.35f,0.95f});
            }
        }

        // Bots
        float botDS = mapOpen_ ? 0.009f : 0.005f;
        for (const Bot& bot : botManager_.bots()) {
            if (bot.isEliminated()) continue;
            float bx = toMX(bot.position.x);
            float by = toMY(bot.position.z);
            if (bx < mmX0+bdr || bx > mmX0+mmW-bdr ||
                by < mmY0+bdr || by > mmY0+mmH-bdr) continue;
            glm::vec4 bc = bot.isAlive()
                ? glm::vec4(1.0f,0.15f,0.12f,0.90f)
                : glm::vec4(0.55f,0.35f,0.35f,0.55f);
            ui_.renderQuad(uiShader_, bx-botDS*0.5f, by+botDS*0.5f, botDS, botDS, bc);
        }

        // Player dot (cyan, larger)
        {
            float ds = mapOpen_ ? 0.018f : 0.010f;
            float px = toMX(player_.position.x);
            float py = toMY(player_.position.z);
            ui_.renderQuad(uiShader_, px-ds*0.5f, py+ds*0.5f, ds, ds, {0.15f,0.95f,1.0f,1.0f});
        }

        // Hint label
        if (!mapOpen_) {
            font_.drawShadowed("[M] MAP", mmX0+0.005f, mmY0+mmH-0.003f,
                               0.012f, 0.020f, {0.45f,0.70f,1.0f,0.80f});
        } else {
            const char* cl = "[M] CLOSE MAP";
            font_.drawShadowed(cl, -font_.textWidth(cl,CWS)*0.5f, -(mmY0+mmH)+0.010f,
                               CWS, CHS, {0.5f,0.75f,1.0f,0.90f});
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Context-sensitive hover prompts (centered at y~0.12)
    if (!playerDead_ && !victoryAchieved_) {
        const float CWP = 0.028f, CHP = 0.050f;
        int doorIdx = world_.nearDoor(player_.position);

        // Find nearby unopened chest (non-destructive check)
        int nearChestIdx = -1;
        for (int ci = 0; ci < dropManager_.chestCount(); ++ci) {
            const Chest& ch = dropManager_.chest(ci);
            if (!ch.opened && glm::length(ch.pos - player_.position) <= 2.8f) {
                nearChestIdx = ci;
                break;
            }
        }
        // Find nearby item (non-destructive check)
        int nearItemIdx = -1;
        for (int ii = 0; ii < dropManager_.itemCount(); ++ii) {
            if (glm::length(dropManager_.item(ii).pos - player_.position) <= 2.5f) {
                nearItemIdx = ii;
                break;
            }
        }

        if (doorIdx >= 0) {
            bool isOpen = world_.buildings()[doorIdx].doorOpen;
            const char* doorTxt = isOpen ? "[E] CLOSE DOOR" : "[E] OPEN DOOR";
            float cx = -font_.textWidth(doorTxt, CWP) * 0.5f;
            font_.drawShadowed(doorTxt, cx, 0.12f, CWP, CHP, {1.0f, 0.9f, 0.3f, 1.0f});
        } else if (nearChestIdx >= 0) {
            bool isBlue = dropManager_.chest(nearChestIdx).isBlue;
            const char* chestTxt = isBlue ? "[E] OPEN CHEST  (RARE)" : "[E] OPEN CHEST";
            float cx = -font_.textWidth(chestTxt, CWP) * 0.5f;
            glm::vec4 chestCol = isBlue ? glm::vec4(0.3f, 0.7f, 1.0f, 1.0f) : glm::vec4(1.0f, 0.85f, 0.1f, 1.0f);
            font_.drawShadowed(chestTxt, cx, 0.12f, CWP, CHP, chestCol);
        } else if (nearItemIdx >= 0) {
            const DroppedItem& di = dropManager_.item(nearItemIdx);
            std::string pickupTxt;
            if (di.hasWeapon) {
                pickupTxt = std::string("[E] PICK UP  ") + getVariantName(di.weaponType, di.nameIdx);
            } else {
                pickupTxt = std::string("[E] PICK UP  ")
                    + getWeaponStats(di.weaponType).name + " AMMO";
            }
            float cx = -font_.textWidth(pickupTxt, CWP) * 0.5f;
            font_.drawShadowed(pickupTxt, cx, 0.12f, CWP, CHP, {0.6f, 1.0f, 0.5f, 1.0f});
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Victory screen
    if (victoryAchieved_) {
        float pulse = 0.70f + 0.30f * sinf(victoryTimer_ * 2.5f);
        ui_.renderQuad(uiShader_, -1.0f, 1.0f, 2.0f, 2.0f, {0.0f,0.05f,0.0f,0.40f});
        const std::string vr = "VICTORY  ROYALE";
        font_.drawShadowed(vr, -font_.textWidth(vr, 0.068f)*0.5f, 0.20f,
                           0.068f, 0.120f, {1.0f, 0.92f, 0.08f, pulse});
        const std::string sub2 = "#1 VICTORY ROYALE";
        font_.drawShadowed(sub2, -font_.textWidth(sub2, CWN)*0.5f, 0.02f,
                           CWN, CHN, {0.20f,1.0f,0.45f,0.95f});
        std::string et = "Returning to lobby in " + std::to_string(std::max(0, 12 - int(victoryTimer_))) + "s";
        font_.drawShadowed(et, -font_.textWidth(et, CWS)*0.5f, -0.10f,
                           CWS, CHS, {0.75f,0.85f,1.0f,0.80f});
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Death screen
    if (playerDead_) {
        ui_.renderQuad(uiShader_, -1.0f, 1.0f, 2.0f, 2.0f, {0.4f,0.0f,0.0f,0.35f});
        font_.drawShadowed("YOU  DIED",
                           -font_.textWidth("YOU DIED", 0.070f)*0.5f, 0.18f,
                           0.070f, 0.124f, {1.0f,0.15f,0.10f,1.0f});
        const std::string elim = "ELIMINATED";
        font_.drawShadowed(elim, -font_.textWidth(elim, CWN)*0.5f, 0.02f,
                           CWN, CHN, {1.0f,0.30f,0.10f,1.0f});
        float pulse = 0.7f + 0.3f * sinf(gameTime_ * 3.0f);
        const std::string ng = "[R]  NEW GAME";
        font_.drawShadowed(ng, -font_.textWidth(ng, CWN)*0.5f, -0.12f,
                           CWN, CHN, {0.80f,0.90f,1.0f,pulse});
        int alive2 = botManager_.aliveCount();
        std::string placeTxt = std::to_string(alive2 + 1) + " / 30 PLAYERS REMAINING";
        font_.drawShadowed(placeTxt, -font_.textWidth(placeTxt, CWS)*0.5f, -0.26f,
                           CWS, CHS, {0.75f,0.75f,0.75f,0.80f});
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Inventory / drop menu  [I]
    if (inventoryOpen_ && !playerDead_ && !victoryAchieved_) {
        static const char* kSlotLabel[5] = {"PISTOL","RIFLE","SHOTGUN","SNIPER","SMG"};
        static const glm::vec3 kSlotColor[5] = {
            {0.65f,0.65f,0.68f},{0.25f,0.70f,0.28f},{0.78f,0.50f,0.22f},{0.22f,0.48f,0.88f},{0.65f,0.25f,0.78f}
        };

        const float PW = 0.70f, PH = 0.90f;
        const float PX = -PW*0.5f, PY = -PH*0.5f + PH;

        // Background panel
        ui_.renderQuad(uiShader_, PX-0.012f, PY+0.012f, PW+0.024f, PH+0.024f, {0,0,0,0.80f});
        ui_.renderQuad(uiShader_, PX, PY, PW, PH, {0.04f,0.06f,0.10f,0.95f});
        ui_.renderQuad(uiShader_, PX, PY+PH, PW, 0.004f, {0.25f,0.55f,1.0f,0.9f});
        ui_.renderQuad(uiShader_, PX, PY,    PW, 0.004f, {0.25f,0.55f,1.0f,0.5f});

        // Title
        const char* title = "INVENTORY";
        font_.drawShadowed(title, PX+(PW-font_.textWidth(title,CWN))*0.5f, PY+PH-0.04f,
                           CWN, CHN, {0.30f,0.85f,1.0f,1.0f});

        const float rowH = 0.090f, rowY0 = PY + PH - 0.10f - rowH;

        for (int i = 0; i < 5; ++i) {
            float ry = rowY0 - i * (rowH + 0.006f);
            bool sel = (i == inventorySel_);

            glm::vec4 bg = sel ? glm::vec4(0.20f,0.28f,0.42f,0.95f)
                               : glm::vec4(0.06f,0.08f,0.12f,0.80f);
            ui_.renderQuad(uiShader_, PX+0.012f, ry, PW-0.024f, rowH, bg);
            if (sel)
                ui_.renderQuad(uiShader_, PX+0.012f, ry+rowH-0.003f, PW-0.024f, 0.003f, {0.30f,0.70f,1.0f,0.9f});

            bool owned = player_.hasWeapon[i];
            glm::vec3 col = owned ? kSlotColor[i] : glm::vec3(0.25f);
            ui_.renderQuad(uiShader_, PX+0.020f, ry+rowH*0.2f, 0.030f, rowH*0.6f, {col.r,col.g,col.b,1.0f});
            const char* wname = owned
                ? (player_.weapons[i].nameIdx > 0 ? getVariantName(player_.weapons[i].type, player_.weapons[i].nameIdx) : kSlotLabel[i])
                : kSlotLabel[i];
            font_.drawShadowed(wname, PX+0.065f, ry+rowH*0.45f, CWS, CHS,
                owned ? glm::vec4(1,1,1,1) : glm::vec4(0.35f,0.35f,0.40f,0.7f));
            if (owned) {
                std::string ammoTxt = std::to_string(player_.weapons[i].ammo)+"/"+std::to_string(player_.weapons[i].reserve);
                font_.drawShadowed(ammoTxt, PX+PW-font_.textWidth(ammoTxt,CWX)-0.020f, ry+rowH*0.45f,
                                   CWX, CHX, {1.0f,0.88f,0.4f,0.9f});
            } else {
                font_.drawShadowed("EMPTY", PX+PW-font_.textWidth("EMPTY",CWX)-0.020f, ry+rowH*0.45f,
                                   CWX, CHX, {0.25f,0.25f,0.30f,0.5f});
            }
        }

        // Controls hint
        font_.drawShadowed("UP/DOWN select   [D] drop   [I]/[ESC] close",
                           PX+0.012f, PY+0.022f, CWX, CHX, {0.55f,0.70f,0.90f,0.75f});
    }

    // ── Arena-specific overlay ────────────────────────────────────────────────
    if (isArena_) renderArenaHUD();
}

// ── Main loop ─────────────────────────────────────────────────────────────────

void Game::run() {
    lastFrame_ = float(glfwGetTime());
    while (!glfwWindowShouldClose(window_)) {
        float now = float(glfwGetTime());
        float dt  = now - lastFrame_;
        if (dt > 0.1f) dt = 0.1f;
        lastFrame_ = now;
        glfwPollEvents();
        update(dt);
        render();
        glfwSwapBuffers(window_);
    }
}
