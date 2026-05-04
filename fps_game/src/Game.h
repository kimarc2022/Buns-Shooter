#pragma once

#include "Shader.h"
#include "Camera.h"
#include "Player.h"
#include "World.h"
#include "UI.h"
#include "Font.h"
#include "Effects.h"
#include "BotManager.h"
#include "DropManager.h"
#include "Difficulty.h"
#include "Pickup.h"
#include "Zone.h"
#include "Skin.h"
#include "SaveData.h"

#include <random>
#include <string>
#include <vector>

struct GLFWwindow;

enum class GameState { Menu, BusFlight, Playing };
enum class MenuPage  { Main, Locker, Shop, Settings, Difficulty };

struct GameSettings {
    float sensitivity = 0.12f;
    float fov         = 70.0f;
    bool  showFPS     = false;
};

class Game {
public:
    bool init(int w, int h, const char* title);
    void run();
    void shutdown();

    void onMouseMove  (double x, double y);
    void onKey        (int key, int action);
    void onMouseButton(int button, int action);
    void onResize     (int w, int h);

private:
    void update(float dt);
    void render();
    void respawnPlayer();          // reset health/weapons only
    void adjustSetting(int delta);

    // ── Bus flight ─────────────────────────────────────────────────────────────
    void startBusFlight(bool isRespawn = false);
    void updateBusFlight(float dt);
    void renderBusFlight();
    void renderBusModel (const glm::mat4& proj, const glm::mat4& view);
    void renderBusHUD();
    void renderGlider   (const glm::mat4& proj, const glm::mat4& view);

    GLFWwindow* window_ = nullptr;
    int  width_  = 1280;
    int  height_ = 720;

    bool   firstMouse_ = true;
    double lastX_ = 0.0, lastY_ = 0.0;
    float  lastFrame_  = 0.0f;

    Shader        worldShader_;
    Shader        uiShader_;
    Camera        camera_;
    Player        player_;
    World         world_;
    UI            ui_;
    Font          font_;
    Effects       effects_;
    BotManager    botManager_;
    DropManager   dropManager_;
    PickupManager pickupManager_;
    Zone          zone_;

    std::mt19937  rng_ { 42u };

    Difficulty   difficulty_ = Difficulty::Easy;
    GameSettings settings_;
    SaveData     saveData_;

    GameState state_         = GameState::Menu;
    MenuPage  menuPage_      = MenuPage::Main;
    int       settingIdx_    = 0;
    int       playerSkinIdx_ = 0;
    int       gliderIdx_     = 0;   // 0..149

    // ── Per-match stats ────────────────────────────────────────────────────────
    int   gameKills_    = 0;   // kills this match (used for D-Buck award)

    // ── Locker / shop UI state ────────────────────────────────────────────────
    int   lockerSel_    = 0;   // index into saveData_.ownedSkins
    int   shopSel_      = 0;   // index into the un-owned skin list
    int   shopScroll_   = 0;   // top row shown in shop grid
    bool  lockerGlider_ = false; // true = showing glider sub-page in Locker

    float gameTime_        = 0.0f;
    float zoneWarnTimer_   = 0.0f;
    float crosshairSpread_ = 0.0f;
    float fps_             = 60.0f;
    float playerWalkPhase_ = 0.0f;
    glm::vec3 playerFacing_{ 0.0f, 0.0f, 1.0f };

    // Player death
    bool  playerDead_    = false;
    float respawnTimer_  = 0.0f;
    bool  playerInWater_ = false;   // ocean (outside boundary) — takes damage
    bool  playerInLake_  = false;   // inland water — swimming, no damage

    // Zone start delay after landing
    float zoneStartDelay_ = 0.0f;

    // Inventory drop menu
    bool  inventoryOpen_ = false;
    int   inventorySel_  = 0;   // 0-4 weapon slots

    // Victory
    bool  victoryAchieved_ = false;
    float victoryTimer_    = 0.0f;

    // ── Bus flight state ───────────────────────────────────────────────────────
    float     busT_           = 0.0f;
    int       busPhase_       = 0;       // 0=riding 1=skydiving 2=gliding
    bool      busIsRespawn_   = false;
    glm::vec3 busStart_       { -230.0f, 120.0f, 0.0f };
    glm::vec3 busEnd_         {  230.0f, 120.0f, 0.0f };
    static constexpr float kBusDuration = 30.0f;
    float gliderDeployAnim_   = 0.0f;    // 0=folded 1=fully open

    // ── Glider animation ───────────────────────────────────────────────────────
    float gliderHeading_  = 0.0f;   // smoothed world-space yaw (radians)
    float gliderBank_     = 0.0f;   // roll angle from turning (radians)
    float gliderYawRate_  = 0.0f;   // smoothed dYaw/dt for banking

    // ── Minimap ────────────────────────────────────────────────────────────────
    bool  mapOpen_   = false;
    bool  mWasDown_  = false;

    // ── Key edge-tracking (gameplay) ──────────────────────────────────────────
    bool isAiming_    = false;

    bool vWasDown_    = false;
    bool rWasDown_    = false;
    bool tabWasDown_  = false;
    bool qWasDown_    = false;
    bool eWasDown_    = false;
    bool gWasDown_    = false;
    bool spaceWasDown_= false;
    bool enterWasDown_= false;
    bool pWasDown_    = false;   // pickaxe equip
    bool iWasDown_    = false;   // inventory toggle
    bool dWasDown_    = false;   // drop item
    bool slot_[5]     = {};      // 1-5 weapon slots

    // ── Arena mode ────────────────────────────────────────────────────────────
    bool  isArena_          = false;
    int   arenaPlayerScore_ = 0;
    float arenaWeaponTimer_ = 0.0f;
    float arenaTimeLeft_    = 0.0f;
    static constexpr float kArenaTime            = 180.0f;
    static constexpr float kArenaWeaponInterval  =  20.0f;

    // ── Key edge-tracking (menu navigation) ───────────────────────────────────
    bool upWasDown_    = false;
    bool downWasDown_  = false;
    bool leftWasDown_  = false;
    bool rightWasDown_ = false;
    bool sWasDown_     = false;
    bool aWasDown_     = false;   // arena launch / locker sub-tab toggle
    bool diffWasDown_  = false;   // difficulty page
    bool lWasDown_     = false;   // locker page
    bool hWasDown_     = false;   // shop (H=sHop) page
    bool gMenuWasDown_ = false;   // glider sub-tab in locker

    void renderMenu();
    void renderHUD(const glm::mat4& proj, const glm::mat4& view);
    void startArena();
    void renderArenaHUD();
    void awardAndSave();
    void renderCharPreview(float x0, float y0, float w, float h, int skinIdx = -1);
    void renderLobbyBackground();   // 3D world orbiting-camera backdrop

    // ── Mouse / cursor state ──────────────────────────────────────────────────
    float menuMouseNX_ = 0.0f;   // NDC -1..1, updated every frame
    float menuMouseNY_ = 0.0f;
    bool  cursorLocked_ = false;  // tracks current GLFW cursor mode
};
