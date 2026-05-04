#include "World.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>
#include <vector>

// ── POI definitions ───────────────────────────────────────────────────────────

const MapPOI World::kPois[World::kPoiCount] = {
    { "LAUNCH SITE",     1.0f,   -1.0f },   //  0
    { "PLEASANT PARK",   0.0f,  -58.0f },   //  1
    { "DUSTY DEPOT",     0.0f,   56.0f },   //  2
    { "SALTY SPRINGS",  55.0f,   -2.0f },   //  3
    { "TILTED TOWERS", -56.0f,    3.0f },   //  4
    { "RETAIL ROW",     55.0f,  -52.0f },   //  5
    { "LONELY LODGE",  -54.0f,   55.0f },   //  6
    { "LOOT LAKE",     -52.0f,  -56.0f },   //  7
    { "FATAL FIELDS",   56.0f,   56.0f },   //  8
    { "HAUNTED HILLS", -140.0f,-110.0f },   //  9  gothic
    { "NEO CITY",       140.0f,  50.0f },   // 10  futuristic
    { "SNOBBY SHORES",  162.0f,   0.0f },   // 11  luxury mansions
    { "TOMATO TOWN",     0.0f, -162.0f },   // 12  diner district
    { "WAILING WOODS", -162.0f,  80.0f },   // 13  dark forest
    { "GREASY GROVE",  -80.0f,  162.0f },   // 14  fast-food suburb
    { "MOISTY MIRE",   140.0f, -132.0f },   // 15  swamp
    { "LUCKY LANDING", -162.0f,   5.0f },   // 16  Asian-inspired
    { "PARADISE PALMS",  30.0f, 177.0f },   // 17  beach resort
    { "RISKY REELS",   157.0f,  -82.0f },   // 18  drive-in theater
    { "SHIFTY SHAFTS", -92.0f, -162.0f },   // 19  mining tunnels
};

// ── Geometry ──────────────────────────────────────────────────────────────────

static const float kCubeVerts[] = {
    -0.5f,-0.5f,-0.5f,  0,0,-1,   0.5f,-0.5f,-0.5f,  0,0,-1,   0.5f, 0.5f,-0.5f,  0,0,-1,
     0.5f, 0.5f,-0.5f,  0,0,-1,  -0.5f, 0.5f,-0.5f,  0,0,-1,  -0.5f,-0.5f,-0.5f,  0,0,-1,
    -0.5f,-0.5f, 0.5f,  0,0, 1,   0.5f,-0.5f, 0.5f,  0,0, 1,   0.5f, 0.5f, 0.5f,  0,0, 1,
     0.5f, 0.5f, 0.5f,  0,0, 1,  -0.5f, 0.5f, 0.5f,  0,0, 1,  -0.5f,-0.5f, 0.5f,  0,0, 1,
    -0.5f, 0.5f, 0.5f, -1,0, 0,  -0.5f, 0.5f,-0.5f, -1,0, 0,  -0.5f,-0.5f,-0.5f, -1,0, 0,
    -0.5f,-0.5f,-0.5f, -1,0, 0,  -0.5f,-0.5f, 0.5f, -1,0, 0,  -0.5f, 0.5f, 0.5f, -1,0, 0,
     0.5f, 0.5f, 0.5f,  1,0, 0,   0.5f, 0.5f,-0.5f,  1,0, 0,   0.5f,-0.5f,-0.5f,  1,0, 0,
     0.5f,-0.5f,-0.5f,  1,0, 0,   0.5f,-0.5f, 0.5f,  1,0, 0,   0.5f, 0.5f, 0.5f,  1,0, 0,
    -0.5f,-0.5f,-0.5f,  0,-1,0,   0.5f,-0.5f,-0.5f,  0,-1,0,   0.5f,-0.5f, 0.5f,  0,-1,0,
     0.5f,-0.5f, 0.5f,  0,-1,0,  -0.5f,-0.5f, 0.5f,  0,-1,0,  -0.5f,-0.5f,-0.5f,  0,-1,0,
    -0.5f, 0.5f,-0.5f,  0, 1,0,   0.5f, 0.5f,-0.5f,  0, 1,0,   0.5f, 0.5f, 0.5f,  0, 1,0,
     0.5f, 0.5f, 0.5f,  0, 1,0,  -0.5f, 0.5f, 0.5f,  0, 1,0,  -0.5f, 0.5f,-0.5f,  0, 1,0,
};

static const float kGroundHalf = 250.0f;
static const float kWaterHalf  = 800.0f;  // ocean extends far
static const float kWaterY     = -0.35f;  // sits just below land level
static const float kGroundVerts[] = {
    -kGroundHalf, 0.0f, -kGroundHalf,  0,1,0,
     kGroundHalf, 0.0f, -kGroundHalf,  0,1,0,
     kGroundHalf, 0.0f,  kGroundHalf,  0,1,0,
     kGroundHalf, 0.0f,  kGroundHalf,  0,1,0,
    -kGroundHalf, 0.0f,  kGroundHalf,  0,1,0,
    -kGroundHalf, 0.0f, -kGroundHalf,  0,1,0,
};

World::~World() {
    if (cubeVBO_)   glDeleteBuffers(1, &cubeVBO_);
    if (cubeVAO_)   glDeleteVertexArrays(1, &cubeVAO_);
    if (groundVBO_) glDeleteBuffers(1, &groundVBO_);
    if (groundVAO_) glDeleteVertexArrays(1, &groundVAO_);
    if (waterVBO_)  glDeleteBuffers(1, &waterVBO_);
    if (waterVAO_)  glDeleteVertexArrays(1, &waterVAO_);
}

void World::setupCube() {
    glGenVertexArrays(1, &cubeVAO_);
    glGenBuffers(1, &cubeVBO_);
    glBindVertexArray(cubeVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVerts), kCubeVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glBindVertexArray(0);
}

void World::setupGround() {
    glGenVertexArrays(1, &groundVAO_);
    glGenBuffers(1, &groundVBO_);
    glBindVertexArray(groundVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kGroundVerts), kGroundVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glBindVertexArray(0);
}

// Inland water zones: {x0, x1, z0, z1}
static const float kInlandWater[][4] = {
    {-72.0f, -32.0f,  -78.0f, -38.0f},   // Loot Lake
    {-28.0f,  28.0f,  -46.0f, -20.0f},   // Central River
    {115.0f, 168.0f, -162.0f,-108.0f},   // Moisty Mire
};
static constexpr int kInlandWaterCount = 3;

void World::setupWater() {
    // Build a single vertex buffer: ocean quad + inland lake quads
    struct Patch { float x0, z0, x1, z1, y; };
    const float W = kWaterHalf, Y = kWaterY;
    Patch patches[1 + kInlandWaterCount];
    patches[0] = {-W, -W,  W,  W, Y};          // ocean
    for (int i = 0; i < kInlandWaterCount; ++i)
        patches[i+1] = {kInlandWater[i][0], kInlandWater[i][2],
                        kInlandWater[i][1], kInlandWater[i][3], 0.02f};

    int np = 1 + kInlandWaterCount;
    std::vector<float> verts;
    verts.reserve(np * 6 * 6);
    for (int i = 0; i < np; ++i) {
        const Patch& p = patches[i];
        float x0=p.x0, z0=p.z0, x1=p.x1, z1=p.z1, wy=p.y;
        verts.insert(verts.end(), {x0,wy,z0,0,1,0, x1,wy,z0,0,1,0, x1,wy,z1,0,1,0});
        verts.insert(verts.end(), {x1,wy,z1,0,1,0, x0,wy,z1,0,1,0, x0,wy,z0,0,1,0});
    }
    waterVertCount_ = np * 6;

    glGenVertexArrays(1, &waterVAO_);
    glGenBuffers(1, &waterVBO_);
    glBindVertexArray(waterVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO_);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size()*sizeof(float)), verts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glBindVertexArray(0);
}

// ── Building population ───────────────────────────────────────────────────────

void World::populate() {
    // cx, cz, sx, sy, sz,  wall R/G/B,  doorFace (0=+Z 1=-Z 2=+X 3=-X),  poiIndex(-1=none)
    struct BDef {
        float cx, cz, sx, sy, sz;
        float wr, wg, wb;
        int   doorFace;
        int   poiIdx;  // index into kPois for the named POI; -1 = unnamed
    };
    static const BDef kBD[] = {
        // ── Launch Site (POI 0) — sprawling command centre ────────────────────
        {  1.0f,  -1.0f, 11.0f, 10.0f, 10.0f,  0.55f,0.50f,0.45f,  0,  0},
        { 28.0f,   8.0f,  8.0f,  6.0f,  7.0f,  0.60f,0.58f,0.52f,  3, -1},
        {-24.0f,   7.0f,  6.0f,  4.0f,  5.0f,  0.65f,0.60f,0.50f,  1, -1},

        // ── Pleasant Park (POI 1) — suburban streets ──────────────────────────
        {-12.0f, -60.0f,  9.0f,  4.5f, 10.0f,  0.75f,0.68f,0.52f,  0,  1},
        { 18.0f, -52.0f,  6.0f,  3.6f,  7.0f,  0.70f,0.60f,0.45f,  1, -1},
        { -8.0f, -84.0f, 13.0f,  5.0f,  9.0f,  0.55f,0.25f,0.20f,  0, -1},

        // ── Dusty Depot (POI 2) — industrial warehouses ───────────────────────
        { -7.0f,  58.0f, 18.0f,  6.0f, 12.0f,  0.45f,0.52f,0.45f,  1,  2},
        { 22.0f,  66.0f, 11.0f,  5.0f,  9.0f,  0.50f,0.48f,0.42f,  3, -1},
        {-22.0f,  74.0f,  5.0f,  8.0f,  5.0f,  0.68f,0.65f,0.60f,  0, -1},

        // ── Salty Springs (POI 3) — residential cluster ───────────────────────
        { 56.0f,  -3.0f, 10.0f,  3.5f, 10.0f,  0.72f,0.65f,0.52f,  3,  3},
        { 74.0f,  12.0f,  7.0f,  3.0f,  6.0f,  0.68f,0.62f,0.50f,  1, -1},
        { 66.0f, -26.0f,  4.0f,  7.0f,  4.0f,  0.70f,0.63f,0.50f,  0, -1},

        // ── Tilted Towers (POI 4) — tall urban towers ─────────────────────────
        {-57.0f,   2.0f, 13.0f,  3.0f,  9.0f,  0.55f,0.56f,0.50f,  2,  4},
        {-80.0f,  18.0f,  9.0f,  2.5f,  7.0f,  0.52f,0.53f,0.48f,  0, -1},
        {-76.0f,  -8.0f,  3.5f,  6.0f,  3.5f,  0.50f,0.52f,0.47f,  2, -1},

        // ── Retail Row (POI 5) — strip of shops ───────────────────────────────
        { 53.0f, -53.0f,  8.0f,  3.5f,  8.0f,  0.72f,0.65f,0.55f,  0,  5},
        { 76.0f, -64.0f,  7.0f,  3.6f,  7.0f,  0.68f,0.72f,0.62f,  3, -1},
        { 54.0f, -32.0f,  9.0f,  3.5f,  6.0f,  0.60f,0.65f,0.70f,  2, -1},

        // ── Lonely Lodge (POI 6) — remote forest cabin ────────────────────────
        {-53.0f,  54.0f,  7.0f,  3.5f,  8.0f,  0.70f,0.68f,0.55f,  0,  6},
        {-76.0f,  70.0f,  7.0f,  4.0f,  6.0f,  0.65f,0.60f,0.50f,  1, -1},
        {-30.0f,  70.0f,  8.0f,  3.5f,  6.0f,  0.60f,0.68f,0.62f,  3, -1},

        // ── Loot Lake (POI 7) — lakeside shacks ───────────────────────────────
        {-53.0f, -57.0f,  7.0f,  3.5f,  8.0f,  0.68f,0.65f,0.58f,  0,  7},
        {-76.0f, -70.0f,  7.0f,  3.8f,  7.0f,  0.72f,0.70f,0.60f,  2, -1},

        // ── Fatal Fields (POI 8) — farmstead ─────────────────────────────────
        { 57.0f,  57.0f,  7.0f,  3.5f,  8.0f,  0.75f,0.68f,0.58f,  0,  8},
        { 80.0f,  74.0f,  8.0f,  4.0f,  7.0f,  0.70f,0.65f,0.55f,  3, -1},

        // ── Mid-ring outposts (open-world combat cover) ───────────────────────
        {  98.0f,  22.0f,  9.0f,  4.0f,  9.0f,  0.72f,0.66f,0.54f,  3, -1},
        { -98.0f, -20.0f,  8.0f,  4.5f,  8.0f,  0.65f,0.60f,0.50f,  2, -1},
        {  22.0f,  98.0f,  9.0f,  4.0f,  9.0f,  0.68f,0.64f,0.52f,  1, -1},
        { -22.0f, -98.0f,  8.0f,  3.8f,  8.0f,  0.74f,0.68f,0.56f,  0, -1},

        // ── Haunted Hills (POI 9) — gothic manor + crypt ─────────────────────
        {-140.0f,-110.0f, 16.0f,  9.0f, 12.0f,  0.25f,0.22f,0.28f,  0,  9},
        {-118.0f, -98.0f,  6.0f,  4.5f,  5.0f,  0.28f,0.24f,0.30f,  2, -1},
        {-162.0f,-128.0f,  5.0f,  9.0f,  6.0f,  0.22f,0.20f,0.26f,  1, -1},

        // ── Neo City (POI 10) — glass towers + labs ───────────────────────────
        { 140.0f,  50.0f, 12.0f, 22.0f, 12.0f,  0.38f,0.44f,0.50f,  0, 10},
        { 118.0f,  38.0f,  8.0f,  5.0f,  7.0f,  0.20f,0.45f,0.68f,  2, -1},
        { 162.0f,  70.0f, 12.0f,  4.5f,  9.0f,  0.35f,0.38f,0.46f,  3, -1},

        // ── Snobby Shores (POI 11) — white luxury mansions ───────────────────
        { 163.0f,   0.0f, 15.0f,  7.5f, 12.0f,  0.93f,0.91f,0.86f,  3, 11},
        { 186.0f, -22.0f,  8.0f,  4.5f,  7.0f,  0.90f,0.88f,0.83f,  0, -1},
        { 142.0f,  24.0f,  7.0f,  3.5f,  6.0f,  0.88f,0.86f,0.80f,  2, -1},

        // ── Tomato Town (POI 12) — red diner district ────────────────────────
        {   0.0f,-163.0f, 14.0f,  5.5f, 10.0f,  0.82f,0.26f,0.16f,  0, 12},
        {  24.0f,-182.0f,  7.0f,  4.0f,  6.0f,  0.86f,0.52f,0.10f,  1, -1},
        { -22.0f,-145.0f,  9.0f,  4.5f,  7.0f,  0.76f,0.22f,0.12f,  2, -1},

        // ── Wailing Woods (POI 13) — dark timber lodges ───────────────────────
        {-163.0f,  80.0f, 12.0f,  5.5f, 10.0f,  0.36f,0.26f,0.18f,  0, 13},
        {-144.0f,  100.0f,  6.0f,  3.5f,  5.0f,  0.42f,0.30f,0.20f,  2, -1},
        {-184.0f,  62.0f,  5.0f,  4.5f,  4.0f,  0.38f,0.28f,0.18f,  1, -1},

        // ── Greasy Grove (POI 14) — yellow fast-food suburb ──────────────────
        { -80.0f, 163.0f, 15.0f,  5.0f, 11.0f,  0.84f,0.74f,0.20f,  1, 14},
        { -56.0f, 146.0f,  7.0f,  4.0f,  6.0f,  0.80f,0.66f,0.16f,  0, -1},
        {-104.0f, 184.0f,  9.0f,  3.5f,  7.0f,  0.86f,0.62f,0.14f,  2, -1},

        // ── Moisty Mire (POI 15) — swamp shacks ──────────────────────────────
        { 140.0f,-133.0f,  9.0f,  3.5f,  8.0f,  0.29f,0.36f,0.23f,  0, 15},
        { 164.0f,-152.0f,  5.0f,  3.0f,  5.0f,  0.33f,0.38f,0.24f,  3, -1},
        { 118.0f,-114.0f,  7.0f,  3.5f,  6.0f,  0.30f,0.34f,0.20f,  2, -1},

        // ── Lucky Landing (POI 16) — red-gold temple ─────────────────────────
        {-163.0f,   5.0f, 15.0f,  8.0f, 12.0f,  0.82f,0.22f,0.18f,  0, 16},
        {-144.0f,  26.0f,  7.0f,  4.5f,  6.0f,  0.86f,0.64f,0.10f,  1, -1},
        {-184.0f, -16.0f,  6.0f,  3.5f,  5.0f,  0.80f,0.18f,0.16f,  2, -1},

        // ── Paradise Palms (POI 17) — teal beach resort ──────────────────────
        {  30.0f, 178.0f, 17.0f,  7.5f, 12.0f,  0.91f,0.88f,0.76f,  0, 17},
        {  58.0f, 197.0f,  6.0f,  3.0f,  5.0f,  0.46f,0.74f,0.76f,  1, -1},
        {   4.0f, 156.0f,  8.0f,  4.5f,  7.0f,  0.89f,0.86f,0.73f,  2, -1},

        // ── Risky Reels (POI 18) — rusty drive-in ────────────────────────────
        { 157.0f, -83.0f, 11.0f,  7.0f,  9.0f,  0.46f,0.33f,0.23f,  3, 18},
        { 180.0f, -64.0f,  6.0f,  3.5f,  5.0f,  0.41f,0.29f,0.21f,  0, -1},
        { 136.0f,-104.0f,  5.0f,  4.5f,  4.0f,  0.43f,0.31f,0.23f,  2, -1},

        // ── Shifty Shafts (POI 19) — dark mine complex ───────────────────────
        { -92.0f,-163.0f, 13.0f,  6.0f, 10.0f,  0.32f,0.30f,0.27f,  0, 19},
        { -68.0f,-180.0f,  8.0f,  4.0f,  7.0f,  0.36f,0.34f,0.30f,  1, -1},
        {-114.0f,-148.0f,  6.0f,  4.5f,  5.0f,  0.43f,0.41f,0.39f,  2, -1},
    };

    for (const BDef& bd : kBD) {
        Building b;
        b.size      = {bd.sx, bd.sy, bd.sz};
        b.center    = {bd.cx, bd.sy * 0.5f, bd.cz};
        b.wallColor = {bd.wr, bd.wg, bd.wb};
        // Roof: slightly lighter / more saturated
        b.roofColor = {std::min(1.0f, bd.wr * 1.12f),
                       std::min(1.0f, bd.wg * 1.05f),
                       std::min(1.0f, bd.wb * 0.98f)};
        // Trim: darker
        b.trimColor = {bd.wr * 0.60f, bd.wg * 0.58f, bd.wb * 0.55f};
        b.doorFace  = bd.doorFace;
        b.poiName   = (bd.poiIdx >= 0) ? kPois[bd.poiIdx].name : nullptr;
        buildings_.push_back(b);

        // Also add to boxes_ so bots / LOS raycast see solid buildings.
        Box bx;
        bx.center = b.center;
        bx.size   = b.size;
        bx.color  = b.wallColor;
        boxes_.push_back(bx);
    }

    // ── Terrain: hills, mountains, ridges ─────────────────────────────────────
    struct TDef { float cx, cz, sx, sy, sz, wr, wg, wb; };
    static const TDef kTD[] = {
        // NE Mountain (3 tiers)
        {  90.0f, -95.0f, 52.0f, 14.0f, 52.0f, 0.48f,0.44f,0.40f},
        {  90.0f, -95.0f, 30.0f,  7.0f, 30.0f, 0.52f,0.48f,0.44f},
        {  90.0f, -95.0f, 14.0f,  5.0f, 14.0f, 0.58f,0.54f,0.52f},
        // SW Mountain (3 tiers)
        { -88.0f,  92.0f, 48.0f, 13.0f, 48.0f, 0.46f,0.43f,0.39f},
        { -88.0f,  92.0f, 28.0f,  6.0f, 28.0f, 0.50f,0.47f,0.43f},
        { -88.0f,  92.0f, 14.0f,  4.0f, 14.0f, 0.55f,0.52f,0.50f},
        // North Hill (2 tiers)
        {   8.0f,-125.0f, 56.0f,  8.0f, 42.0f, 0.28f,0.50f,0.20f},
        {   8.0f,-125.0f, 30.0f,  5.0f, 22.0f, 0.32f,0.55f,0.22f},
        // South Hill (2 tiers)
        {  -6.0f, 125.0f, 50.0f,  7.0f, 44.0f, 0.28f,0.50f,0.20f},
        {  -6.0f, 125.0f, 26.0f,  4.0f, 22.0f, 0.32f,0.55f,0.22f},
        // West Ridge (2 tiers)
        {-178.0f,  25.0f, 22.0f, 12.0f, 92.0f, 0.46f,0.42f,0.36f},
        {-178.0f,  25.0f, 16.0f,  5.0f, 74.0f, 0.50f,0.46f,0.40f},
        // East Ridge
        { 178.0f, -22.0f, 22.0f, 10.0f, 85.0f, 0.46f,0.42f,0.36f},
        // SE Hill
        { 110.0f, 105.0f, 46.0f,  8.0f, 46.0f, 0.30f,0.52f,0.20f},
        { 110.0f, 105.0f, 24.0f,  4.0f, 24.0f, 0.34f,0.56f,0.22f},
        // NW Hill
        {-108.0f, -98.0f, 44.0f,  7.0f, 44.0f, 0.30f,0.52f,0.20f},
        {-108.0f, -98.0f, 22.0f,  4.0f, 22.0f, 0.34f,0.56f,0.22f},
    };
    for (const TDef& td : kTD) {
        Building b;
        b.size      = {td.sx, td.sy, td.sz};
        b.center    = {td.cx, td.sy * 0.5f, td.cz};
        b.wallColor = {td.wr, td.wg, td.wb};
        b.roofColor = b.wallColor;
        b.trimColor = b.wallColor * 0.80f;
        b.doorFace  = -1;
        b.isTerrain = true;
        buildings_.push_back(b);

        Box bx;
        bx.center = b.center;
        bx.size   = b.size;
        bx.color  = b.wallColor;
        boxes_.push_back(bx);
    }

    // ── Low walls / cover (boxes_ only – no door) ─────────────────────────────
    struct LW { float cx, cz, sx, sy, sz, r, g, b; };
    static const LW kLW[] = {
        {  24.0f,  18.0f,  5.0f,  2.0f,  2.5f,  0.50f, 0.50f, 0.48f},
        { -22.0f, -24.0f,  4.0f,  1.8f,  3.5f,  0.48f, 0.48f, 0.46f},
        {  30.0f, -38.0f,  3.0f,  2.0f,  5.0f,  0.52f, 0.50f, 0.48f},
        { -35.0f,  28.0f,  4.0f,  1.8f,  3.0f,  0.50f, 0.48f, 0.46f},
        {  18.0f,  38.0f,  3.0f,  1.8f,  4.5f,  0.49f, 0.49f, 0.47f},
        { -28.0f,  15.0f,  2.0f,  2.2f,  6.0f,  0.51f, 0.50f, 0.48f},
        // Extra cover rocks
        {  42.0f,  30.0f,  3.5f,  1.5f,  3.5f,  0.52f, 0.50f, 0.46f},
        { -40.0f, -20.0f,  4.0f,  1.6f,  3.0f,  0.50f, 0.49f, 0.47f},
        {   8.0f,  -38.0f, 3.0f,  1.8f,  4.0f,  0.48f, 0.50f, 0.48f},
        { -18.0f,  42.0f,  3.5f,  1.8f,  3.5f,  0.51f, 0.50f, 0.48f},
    };
    for (const LW& w : kLW) {
        Box bx;
        bx.center = {w.cx, w.sy * 0.5f, w.cz};
        bx.size   = {w.sx, w.sy, w.sz};
        bx.color  = {w.r,  w.g,  w.b};
        boxes_.push_back(bx);
    }

    // ── Trees ─────────────────────────────────────────────────────────────────
    struct Tree { float cx, cz, th, fr; };
    static const Tree kTrees[] = {
        { 36.0f,-42.0f, 3.5f, 2.2f}, { 41.0f,-38.0f, 4.2f, 2.8f}, { 38.0f,-46.0f, 2.8f, 1.8f},
        { 34.0f,-35.0f, 3.8f, 2.4f}, { 43.0f,-43.0f, 3.2f, 2.0f},
        {-38.0f, 36.0f, 4.0f, 2.6f}, {-43.0f, 41.0f, 3.5f, 2.2f}, {-36.0f, 39.0f, 4.5f, 2.8f},
        {-41.0f, 34.0f, 3.0f, 2.0f}, {-45.0f, 43.0f, 3.8f, 2.4f},
        { 29.0f, 26.0f, 3.5f, 2.0f}, { 33.0f, 22.0f, 4.0f, 2.5f}, { 26.0f, 29.0f, 3.2f, 1.9f},
        {-29.0f,-33.0f, 3.8f, 2.3f}, {-36.0f,-29.0f, 4.2f, 2.7f}, {-31.0f,-36.0f, 3.0f, 1.8f},
        { -8.0f,-82.0f, 4.5f, 2.8f}, {  5.0f,-86.0f, 3.8f, 2.4f}, {-20.0f,-88.0f, 4.0f, 2.6f},
        { 16.0f,-80.0f, 5.0f, 3.0f}, {-30.0f,-76.0f, 3.5f, 2.2f}, { 26.0f,-78.0f, 4.2f, 2.6f},
        {  0.0f,-92.0f, 4.8f, 3.0f}, {-12.0f,-75.0f, 3.6f, 2.3f},
        {  8.0f, 82.0f, 4.5f, 2.8f}, { -5.0f, 86.0f, 3.8f, 2.4f}, { 23.0f, 84.0f, 4.0f, 2.6f},
        {-18.0f, 80.0f, 5.0f, 3.0f}, { 31.0f, 78.0f, 3.5f, 2.2f}, {-28.0f, 82.0f, 4.2f, 2.6f},
        {  0.0f, 92.0f, 4.8f, 3.0f}, { 12.0f, 75.0f, 3.6f, 2.3f},
        { 82.0f,-10.0f, 4.0f, 2.5f}, { 85.0f,  5.0f, 3.8f, 2.3f}, { 87.0f,-22.0f, 4.5f, 2.8f},
        { 80.0f, 15.0f, 3.5f, 2.1f}, { 90.0f,  0.0f, 4.2f, 2.6f}, { 84.0f, 28.0f, 3.8f, 2.4f},
        {-82.0f, 10.0f, 4.0f, 2.5f}, {-85.0f, -5.0f, 3.8f, 2.3f}, {-87.0f, 22.0f, 4.5f, 2.8f},
        {-80.0f,-15.0f, 3.5f, 2.1f}, {-90.0f,  0.0f, 4.2f, 2.6f}, {-84.0f,-28.0f, 3.8f, 2.4f},
        { 92.0f,-92.0f, 5.0f, 3.5f}, { 87.0f,-87.0f, 4.5f, 2.8f}, { 94.0f,-82.0f, 4.0f, 2.5f},
        {-92.0f, 92.0f, 5.0f, 3.5f}, {-87.0f, 87.0f, 4.5f, 2.8f}, {-94.0f, 82.0f, 4.0f, 2.5f},
        { 92.0f, 92.0f, 5.0f, 3.5f}, { 87.0f, 87.0f, 4.5f, 2.8f},
        {-92.0f,-92.0f, 5.0f, 3.5f}, {-87.0f,-87.0f, 4.5f, 2.8f},
        { 72.0f,-35.0f, 3.8f, 2.4f}, { 68.0f,-28.0f, 3.4f, 2.1f},
        {-72.0f, 35.0f, 3.8f, 2.4f}, {-68.0f, 28.0f, 3.4f, 2.1f},
        { 18.0f,-115.0f,5.2f, 3.2f}, {-15.0f,-118.0f,4.8f, 3.0f},
        { 18.0f, 115.0f,5.2f, 3.2f}, {-15.0f, 118.0f,4.8f, 3.0f},
    };

    static const glm::vec3 kTrunkCol{0.35f, 0.22f, 0.10f};
    static const glm::vec3 kLeaf0   {0.18f, 0.52f, 0.14f};
    static const glm::vec3 kLeaf1   {0.22f, 0.60f, 0.16f};
    static const glm::vec3 kLeaf2   {0.26f, 0.68f, 0.18f};

    for (const Tree& t : kTrees) {
        Box trunk;
        trunk.center = {t.cx, t.th * 0.5f, t.cz};
        trunk.size   = {0.55f, t.th, 0.55f};
        trunk.color  = kTrunkCol;
        boxes_.push_back(trunk);

        float lh[3] = {t.th + t.fr*0.25f, t.th + t.fr*0.85f, t.th + t.fr*1.45f};
        float lr[3] = {t.fr, t.fr*0.65f, t.fr*0.35f};
        const glm::vec3 lc[3] = {kLeaf0, kLeaf1, kLeaf2};
        for (int i = 0; i < 3; ++i) {
            Box leaf;
            leaf.center = {t.cx, lh[i], t.cz};
            leaf.size   = {lr[i]*2.0f, lr[i]*0.75f, lr[i]*2.0f};
            leaf.color  = lc[i];
            boxes_.push_back(leaf);
        }
    }
}

// ── Building rendering ────────────────────────────────────────────────────────

void World::rb(Shader& sh, const glm::vec3& c, const glm::vec3& s, const glm::vec3& col) const {
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), c), s);
    sh.setMat4("uModel", m);
    sh.setVec3("uColor", col);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

bool World::isInsideBuilding(const glm::vec3& pos) const {
    for (const Building& b : buildings_) {
        if (b.isTerrain || b.doorOpen) continue;
        glm::vec3 mn = b.center - b.size * 0.5f;
        glm::vec3 mx = b.center + b.size * 0.5f;
        if (pos.x > mn.x && pos.x < mx.x &&
            pos.y > mn.y && pos.y < mx.y &&
            pos.z > mn.z && pos.z < mx.z)
            return true;
    }
    return false;
}

bool World::isInlandWater(const glm::vec3& pos) const {
    for (int i = 0; i < kInlandWaterCount; ++i) {
        if (pos.x > kInlandWater[i][0] && pos.x < kInlandWater[i][1] &&
            pos.z > kInlandWater[i][2] && pos.z < kInlandWater[i][3])
            return true;
    }
    return false;
}

void World::renderBuilding(Shader& sh, const Building& b) const {
    // Terrain boxes render as simple colored solids
    if (b.isTerrain) {
        rb(sh, b.center, b.size, b.wallColor);
        return;
    }

    const glm::vec3& wc  = b.wallColor;
    const glm::vec3& rc  = b.roofColor;
    const glm::vec3& tc  = b.trimColor;
    const glm::vec3  win = {0.12f, 0.18f, 0.28f};   // dark tinted window
    const glm::vec3  wfr = wc * 1.35f;               // light window frame
    const glm::vec3  drc = {0.30f, 0.18f, 0.08f};   // door wood colour
    const glm::vec3  dfr = wc * 1.45f;               // door frame
    const glm::vec3  dhd = {0.85f, 0.72f, 0.30f};   // door handle gold

    const float cx = b.center.x, cy = b.center.y, cz = b.center.z;
    const float hx = b.size.x * 0.5f;
    const float hy = b.size.y * 0.5f;
    const float hz = b.size.z * 0.5f;
    const float bot = cy - hy;    // y at ground level
    const float top = cy + hy;    // y at roof top

    // ── Main walls ─────────────────────────────────────────────────────────────
    rb(sh, b.center, b.size, wc);

    // ── Contrasting corner pillars ─────────────────────────────────────────────
    float pilW = std::min(0.40f, hx * 0.12f);
    float pilH = b.size.y * 0.95f;
    glm::vec3 pilC = wc * 0.75f;
    // Four vertical corner pillars
    for (int xi : {-1, 1}) for (int zi : {-1, 1}) {
        rb(sh, {cx + xi*(hx - pilW*0.5f), bot + pilH*0.5f, cz + zi*(hz - pilW*0.5f)},
           {pilW, pilH, pilW}, pilC);
    }

    // ── Base trim band ─────────────────────────────────────────────────────────
    float trimH = std::min(0.30f, b.size.y * 0.08f);
    rb(sh, {cx, bot + trimH*0.5f, cz}, {b.size.x + 0.04f, trimH, b.size.z + 0.04f}, tc);

    // ── Roof (flat / slightly peaked) ──────────────────────────────────────────
    float roofT  = std::max(0.28f, b.size.y * 0.06f);
    float roofOH = 0.35f;
    rb(sh, {cx, top + roofT*0.5f, cz},
       {b.size.x + roofOH*2, roofT, b.size.z + roofOH*2}, rc);
    // Roof edge coping
    rb(sh, {cx, top + roofT + 0.08f, cz},
       {b.size.x + roofOH*2 + 0.10f, 0.16f, b.size.z + roofOH*2 + 0.10f}, tc);
    // Central roof ridge (gives pitched-roof feel)
    if (b.size.y > 3.5f) {
        float ridgeW = b.size.x > b.size.z ? b.size.x * 0.80f : b.size.z * 0.80f;
        bool  longX  = (b.size.x >= b.size.z);
        rb(sh, {cx, top + roofT + 0.30f, cz},
           longX ? glm::vec3(ridgeW, 0.22f, 0.30f) : glm::vec3(0.30f, 0.22f, ridgeW),
           rc * 1.10f);
    }

    // ── Windows ────────────────────────────────────────────────────────────────
    float winW = 0.90f, winH = 0.80f;
    float winY = bot + b.size.y * 0.50f;   // vertically centred on wall
    float winD = 0.08f;                     // how far window recesses into wall

    // Helper lambda to draw a window + frame on a face.
    auto drawWin = [&](const glm::vec3& wpos, const glm::vec3& wsize,
                       const glm::vec3& fsize) {
        rb(sh, wpos, wsize, win);
        // Frame: four thin bars
        rb(sh, wpos + glm::vec3(0, winH*0.5f + 0.05f, 0), {fsize.x + 0.14f, 0.10f, fsize.z + 0.14f}, wfr);
        rb(sh, wpos + glm::vec3(0,-winH*0.5f - 0.05f, 0), {fsize.x + 0.14f, 0.10f, fsize.z + 0.14f}, wfr);
    };

    // +X face windows (if face not used as door)
    if (b.doorFace != 2) {
        int nWin = std::max(1, int(b.size.z / 3.5f));
        float spacing = b.size.z / (nWin + 1);
        for (int i = 1; i <= nWin; ++i) {
            float wz = cz - hz + i * spacing;
            glm::vec3 wp = {cx + hx + winD, winY, wz};
            drawWin(wp, {winD*2, winH, winW}, {winD*2, winH, winW});
        }
    }
    // -X face windows
    if (b.doorFace != 3) {
        int nWin = std::max(1, int(b.size.z / 3.5f));
        float spacing = b.size.z / (nWin + 1);
        for (int i = 1; i <= nWin; ++i) {
            float wz = cz - hz + i * spacing;
            glm::vec3 wp = {cx - hx - winD, winY, wz};
            drawWin(wp, {winD*2, winH, winW}, {winD*2, winH, winW});
        }
    }
    // +Z face windows
    if (b.doorFace != 0) {
        int nWin = std::max(1, int(b.size.x / 3.5f));
        float spacing = b.size.x / (nWin + 1);
        for (int i = 1; i <= nWin; ++i) {
            float wx2 = cx - hx + i * spacing;
            glm::vec3 wp = {wx2, winY, cz + hz + winD};
            drawWin(wp, {winW, winH, winD*2}, {winW, winH, winD*2});
        }
    }
    // -Z face windows
    if (b.doorFace != 1) {
        int nWin = std::max(1, int(b.size.x / 3.5f));
        float spacing = b.size.x / (nWin + 1);
        for (int i = 1; i <= nWin; ++i) {
            float wx2 = cx - hx + i * spacing;
            glm::vec3 wp = {wx2, winY, cz - hz - winD};
            drawWin(wp, {winW, winH, winD*2}, {winW, winH, winD*2});
        }
    }

    // ── Interior: inner wall panels, floor, ceiling, furniture ────────────────
    {
        float wpT = 0.22f;          // inner panel thickness (wall thickness)
        float wpH = b.size.y * 0.95f;
        glm::vec3 wci = wc * 0.80f; // slightly darker interior tone

        // Inset panels sit 3 cm inside the outer wall face so depth-testing
        // always keeps them in front of window back-faces (no culling needed).
        const float ins = 0.03f;
        rb(sh, {cx, bot + wpH*0.5f, cz + hz - ins - wpT*0.5f},
               {b.size.x, wpH, wpT}, wci);   // +Z inner wall
        rb(sh, {cx, bot + wpH*0.5f, cz - hz + ins + wpT*0.5f},
               {b.size.x, wpH, wpT}, wci);   // -Z inner wall
        rb(sh, {cx + hx - ins - wpT*0.5f, bot + wpH*0.5f, cz},
               {wpT, wpH, b.size.z}, wci);   // +X inner wall
        rb(sh, {cx - hx + ins + wpT*0.5f, bot + wpH*0.5f, cz},
               {wpT, wpH, b.size.z}, wci);   // -X inner wall

        // Interior floor (dark worn wood)
        rb(sh, {cx, bot + 0.06f, cz},
               {b.size.x - wpT*2.0f, 0.12f, b.size.z - wpT*2.0f},
               glm::vec3(0.32f, 0.24f, 0.14f));

        // Interior ceiling band
        rb(sh, {cx, top - 0.10f, cz},
               {b.size.x - wpT*2.0f, 0.10f, b.size.z - wpT*2.0f},
               wc * 0.88f);

        // Furniture in larger buildings
        float iw = b.size.x - wpT*2.0f;
        float id = b.size.z - wpT*2.0f;
        if (iw > 3.5f && id > 3.5f) {
            float toff   = (b.doorFace == 0 || b.doorFace == 1) ? -1.0f : 1.0f;
            float tableH = std::min(0.80f, b.size.y * 0.22f);
            glm::vec3 tc2 = {cx + toff*0.8f, bot + tableH, cz - toff*0.8f};
            rb(sh, tc2,                                    {1.4f, 0.08f, 0.80f}, {0.42f, 0.30f, 0.15f});
            rb(sh, tc2 + glm::vec3(0, -tableH*0.5f, 0),   {1.4f, tableH, 0.80f}, {0.35f, 0.24f, 0.12f});
        }
        if (iw > 5.0f && id > 5.0f) {
            float cOff = std::min(iw, id) * 0.35f;
            glm::vec3 cb2 = {cx + cOff, bot + 0.30f, cz + cOff};
            rb(sh, cb2,                              {0.60f, 0.60f, 0.60f}, {0.38f, 0.34f, 0.28f});
            rb(sh, cb2 + glm::vec3(0, 0.60f, 0),    {0.55f, 0.55f, 0.55f}, {0.45f, 0.40f, 0.32f});
            rb(sh, cb2 + glm::vec3(0.65f, 0, 0.1f), {0.62f, 0.62f, 0.58f}, {0.40f, 0.36f, 0.30f});
        }
    }

    // ── Door ───────────────────────────────────────────────────────────────────
    const float DW = 1.50f;   // door width
    const float DH = std::min(2.40f, b.size.y * 0.75f);
    glm::vec3 dc = doorCenter(b);   // door face centre at door-height level
    // Actual door panel centre is at dc.y = bot + DH/2
    glm::vec3 dpanelCenter = {dc.x, bot + DH*0.5f, dc.z};

    if (!b.doorOpen) {
        // Closed door panel (on the wall face)
        glm::vec3 dpSize;
        switch (b.doorFace) {
            case 0: case 1: dpSize = {DW, DH, 0.15f}; break;
            default:        dpSize = {0.15f, DH, DW};  break;
        }
        rb(sh, dpanelCenter, dpSize, drc);
        // Door frame
        rb(sh, dpanelCenter + glm::vec3(0, DH*0.5f + 0.08f, 0),
           (b.doorFace < 2) ? glm::vec3(DW + 0.20f, 0.15f, 0.18f) : glm::vec3(0.18f, 0.15f, DW + 0.20f),
           dfr);
        // Door handle
        glm::vec3 handleOff = {0, -0.10f, 0};
        switch (b.doorFace) {
            case 0: handleOff.x =  DW * 0.38f; handleOff.z =  0.10f; break;
            case 1: handleOff.x = -DW * 0.38f; handleOff.z = -0.10f; break;
            case 2: handleOff.z = -DW * 0.38f; handleOff.x =  0.10f; break;
            case 3: handleOff.z =  DW * 0.38f; handleOff.x = -0.10f; break;
        }
        rb(sh, dpanelCenter + handleOff, {0.12f, 0.22f, 0.12f}, dhd);
    } else {
        // Open door — panel swung 90 degrees along the wall face
        glm::vec3 swungPos = dpanelCenter;
        glm::vec3 dpSize;
        switch (b.doorFace) {
            case 0: swungPos.x += hx - 0.075f; swungPos.z -= DW*0.5f; dpSize = {0.15f, DH, DW}; break;
            case 1: swungPos.x -= hx - 0.075f; swungPos.z += DW*0.5f; dpSize = {0.15f, DH, DW}; break;
            case 2: swungPos.z += hz - 0.075f; swungPos.x -= DW*0.5f; dpSize = {DW, DH, 0.15f}; break;
            case 3: swungPos.z -= hz - 0.075f; swungPos.x += DW*0.5f; dpSize = {DW, DH, 0.15f}; break;
        }
        rb(sh, swungPos, dpSize, drc * 0.80f);
    }
}

// ── Door helpers ──────────────────────────────────────────────────────────────

glm::vec3 World::doorCenter(const Building& b) const {
    float hx = b.size.x * 0.5f;
    float hz = b.size.z * 0.5f;
    float doorY = b.center.y - b.size.y*0.5f + 1.20f;
    switch (b.doorFace) {
        case 0: return {b.center.x, doorY, b.center.z + hz};
        case 1: return {b.center.x, doorY, b.center.z - hz};
        case 2: return {b.center.x + hx, doorY, b.center.z};
        case 3: return {b.center.x - hx, doorY, b.center.z};
    }
    return b.center;
}

int World::nearDoor(const glm::vec3& pos) const {
    const float kDist = 2.5f;
    for (int i = 0; i < (int)buildings_.size(); ++i) {
        glm::vec3 dc = doorCenter(buildings_[i]);
        float d2 = (pos.x - dc.x)*(pos.x - dc.x) + (pos.z - dc.z)*(pos.z - dc.z);
        if (d2 <= kDist * kDist) return i;
    }
    return -1;
}

int World::tryToggleDoor(const glm::vec3& pos) {
    int idx = nearDoor(pos);
    if (idx >= 0) buildings_[idx].doorOpen = !buildings_[idx].doorOpen;
    return idx;
}

// ── Player–building collision ─────────────────────────────────────────────────

bool World::resolvePlayerBuilding(glm::vec3& pos, float radius, float height) const {
    bool landedOnRoof = false;
    for (const Building& b : buildings_) {
        if (b.doorOpen) continue;

        glm::vec3 bMin = b.center - b.size * 0.5f;
        glm::vec3 bMax = b.center + b.size * 0.5f;

        // Player AABB
        float pMinX = pos.x - radius, pMaxX = pos.x + radius;
        float pMinY = pos.y,          pMaxY = pos.y + height;
        float pMinZ = pos.z - radius, pMaxZ = pos.z + radius;

        if (pMaxX <= bMin.x || pMinX >= bMax.x) continue;
        if (pMaxY <= bMin.y || pMinY >= bMax.y) continue;
        if (pMaxZ <= bMin.z || pMinZ >= bMax.z) continue;

        // Penetration depths (positive = overlap amount)
        float pushPosX = bMax.x - pMinX;   // push player right (+X)
        float pushNegX = pMaxX - bMin.x;   // push player left  (-X)
        float pushPosZ = bMax.z - pMinZ;   // push player fwd   (+Z)
        float pushNegZ = pMaxZ - bMin.z;   // push player back  (-Z)
        float pushPosY = bMax.y - pMinY;   // push player up (land on roof)

        float minD = pushPosX;
        int   axis = 0;
        if (pushNegX < minD) { minD = pushNegX; axis = 1; }
        if (pushPosZ < minD) { minD = pushPosZ; axis = 2; }
        if (pushNegZ < minD) { minD = pushNegZ; axis = 3; }
        // Roof landing: prefer Y push only when player bottom is close to building top
        if (pushPosY < minD && pMinY >= bMax.y - 0.55f) { minD = pushPosY; axis = 4; }

        switch (axis) {
            case 0: pos.x = bMax.x + radius + 0.01f; break;
            case 1: pos.x = bMin.x - radius - 0.01f; break;
            case 2: pos.z = bMax.z + radius + 0.01f; break;
            case 3: pos.z = bMin.z - radius - 0.01f; break;
            case 4: pos.y = bMax.y; landedOnRoof = true; break;
        }
    }
    return landedOnRoof;
}

// ── Raycast ───────────────────────────────────────────────────────────────────

static float rayVsAABB(const glm::vec3& o, const glm::vec3& d,
                        const glm::vec3& mn, const glm::vec3& mx) {
    float tMin = 0.0f, tMax = 1e9f;
    for (int i = 0; i < 3; ++i) {
        if (fabsf(d[i]) < 1e-7f) {
            if (o[i] < mn[i] || o[i] > mx[i]) return 1e9f;
        } else {
            float inv = 1.0f / d[i];
            float t1  = (mn[i] - o[i]) * inv;
            float t2  = (mx[i] - o[i]) * inv;
            if (t1 > t2) std::swap(t1, t2);
            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            if (tMin > tMax) return 1e9f;
        }
    }
    return tMin;
}

float World::raycast(const glm::vec3& origin, const glm::vec3& dir,
                     float maxDist, glm::vec3* hitPoint) const {
    float bestT = maxDist;
    for (const Box& box : boxes_) {
        glm::vec3 mn = box.center - box.size * 0.5f;
        glm::vec3 mx = box.center + box.size * 0.5f;
        float t = rayVsAABB(origin, dir, mn, mx);
        if (t < bestT) bestT = t;
    }
    // Also check buildings so TPS camera clips against them
    for (const Building& b : buildings_) {
        glm::vec3 mn = b.center - b.size * 0.5f;
        glm::vec3 mx = b.center + b.size * 0.5f;
        float t = rayVsAABB(origin, dir, mn, mx);
        if (t < bestT) bestT = t;
    }
    if (hitPoint) *hitPoint = origin + dir * bestT;
    return bestT;
}

// ── GL helpers ────────────────────────────────────────────────────────────────

void World::init() { setupCube(); setupGround(); setupWater(); populate(); }

void World::beginBoxRender() { glBindVertexArray(cubeVAO_); }
void World::endBoxRender  () { glBindVertexArray(0); }

void World::drawBox(Shader& sh, const glm::vec3& center,
                    const glm::vec3& size, const glm::vec3& color) {
    rb(sh, center, size, color);
}

// ── Full render ───────────────────────────────────────────────────────────────

void World::render(Shader& shader, const glm::vec3& viewPos) {
    shader.use();
    shader.setVec3 ("uLightDir", glm::normalize(glm::vec3(-0.4f, -1.0f, -0.3f)));
    shader.setVec3 ("uViewPos",  viewPos);
    shader.setFloat("uAlpha",    1.0f);

    glm::mat4 identity(1.0f);

    // Water (rendered before ground so land sits on top)
    shader.setMat4("uModel", identity);
    shader.setVec3("uColor", glm::vec3(0.06f, 0.28f, 0.62f));
    shader.setFloat("uAlpha", 1.0f);
    glBindVertexArray(waterVAO_);
    glDrawArrays(GL_TRIANGLES, 0, waterVertCount_);

    // Ground
    shader.setMat4("uModel", identity);
    shader.setVec3("uColor", glm::vec3(0.28f, 0.52f, 0.26f));
    glBindVertexArray(groundVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Low walls and trees
    glBindVertexArray(cubeVAO_);
    int buildingBoxes = (int)buildings_.size();  // first N boxes correspond to buildings
    for (int i = buildingBoxes; i < (int)boxes_.size(); ++i) {
        const Box& bx = boxes_[i];
        glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), bx.center), bx.size);
        shader.setMat4("uModel", m);
        shader.setVec3("uColor", bx.color);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Detailed buildings
    for (const Building& b : buildings_)
        renderBuilding(shader, b);

    glBindVertexArray(0);
}
