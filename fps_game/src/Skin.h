#pragma once
#include <glm/glm.hpp>

enum class SkinStyle : int {
    Suit=0, Military, SciFi, Casual, Armor,
    Ninja, Cowboy, Robot, Medic, Raider
};

struct Skin {
    const char* name;
    SkinStyle   style;
    glm::vec3   skin;    // face / exposed flesh
    glm::vec3   hair;    // hair or helmet base
    glm::vec3   torso;   // jacket / armour main
    glm::vec3   detail;  // chest panel / shirt front
    glm::vec3   pants;   // legs
    glm::vec3   boots;   // footwear
    glm::vec3   lhand;   // left hand (glove / mechanical)
    glm::vec3   accent;  // belt, glow, visor, tie
};

// 100 skins (10 styles × 10 colour themes)
inline const Skin kSkins[100] = {

// ── STYLE 0: SUIT ──────────────────────────────────────────────────────────────
{"Dark Baron",       SkinStyle::Suit,     {.87f,.68f,.52f},{.08f,.06f,.06f},{.07f,.09f,.24f},{.90f,.90f,.93f},{.05f,.06f,.16f},{.05f,.05f,.07f},{.84f,.62f,.08f},{.84f,.62f,.08f}},
{"Shadow Elite",     SkinStyle::Suit,     {.94f,.82f,.70f},{.25f,.22f,.20f},{.18f,.18f,.20f},{.92f,.92f,.94f},{.15f,.15f,.18f},{.10f,.10f,.12f},{.78f,.78f,.82f},{.60f,.65f,.70f}},
{"Navy Commander",   SkinStyle::Suit,     {.70f,.50f,.35f},{.16f,.10f,.08f},{.08f,.12f,.35f},{.72f,.82f,.96f},{.06f,.10f,.30f},{.05f,.05f,.10f},{.87f,.68f,.52f},{.20f,.80f,.90f}},
{"Midnight Agent",   SkinStyle::Suit,     {.55f,.38f,.26f},{.06f,.06f,.08f},{.10f,.10f,.12f},{.30f,.30f,.32f},{.08f,.08f,.10f},{.06f,.06f,.07f},{.10f,.10f,.12f},{.85f,.15f,.15f}},
{"Gold Baron",       SkinStyle::Suit,     {.87f,.68f,.52f},{.25f,.15f,.08f},{.30f,.20f,.10f},{.80f,.68f,.42f},{.25f,.16f,.08f},{.20f,.14f,.06f},{.84f,.62f,.08f},{.84f,.62f,.08f}},
{"Silver Executive", SkinStyle::Suit,     {.94f,.88f,.80f},{.85f,.85f,.88f},{.65f,.70f,.78f},{.95f,.95f,.97f},{.55f,.58f,.65f},{.50f,.52f,.58f},{.70f,.72f,.78f},{.40f,.55f,.85f}},
{"Crimson CEO",      SkinStyle::Suit,     {.87f,.68f,.52f},{.08f,.06f,.06f},{.45f,.06f,.08f},{.92f,.92f,.94f},{.12f,.05f,.07f},{.06f,.05f,.06f},{.84f,.62f,.08f},{.84f,.62f,.08f}},
{"Emerald Don",      SkinStyle::Suit,     {.80f,.60f,.42f},{.10f,.08f,.06f},{.08f,.24f,.12f},{.92f,.92f,.94f},{.06f,.18f,.09f},{.05f,.06f,.05f},{.87f,.68f,.52f},{.84f,.62f,.08f}},
{"Violet Sovereign", SkinStyle::Suit,     {.94f,.82f,.70f},{.38f,.15f,.48f},{.28f,.12f,.40f},{.92f,.92f,.94f},{.20f,.08f,.30f},{.06f,.05f,.08f},{.70f,.70f,.78f},{.70f,.70f,.78f}},
{"Steel Diplomat",   SkinStyle::Suit,     {.90f,.78f,.64f},{.55f,.55f,.58f},{.42f,.46f,.52f},{.90f,.90f,.92f},{.38f,.40f,.45f},{.30f,.32f,.36f},{.87f,.68f,.52f},{.90f,.60f,.15f}},

// ── STYLE 1: MILITARY ──────────────────────────────────────────────────────────
{"Desert Ghost",     SkinStyle::Military, {.80f,.65f,.48f},{.60f,.52f,.32f},{.62f,.54f,.32f},{.52f,.44f,.26f},{.58f,.50f,.28f},{.35f,.28f,.16f},{.62f,.54f,.32f},{.80f,.70f,.40f}},
{"Urban Ranger",     SkinStyle::Military, {.75f,.58f,.40f},{.22f,.24f,.26f},{.25f,.28f,.30f},{.32f,.35f,.38f},{.22f,.25f,.28f},{.18f,.18f,.20f},{.25f,.28f,.30f},{.45f,.65f,.30f}},
{"Jungle Recon",     SkinStyle::Military, {.70f,.55f,.38f},{.12f,.18f,.08f},{.15f,.22f,.10f},{.20f,.28f,.14f},{.12f,.18f,.08f},{.10f,.12f,.06f},{.15f,.22f,.10f},{.55f,.80f,.20f}},
{"Arctic Sniper",    SkinStyle::Military, {.94f,.88f,.80f},{.95f,.95f,.97f},{.90f,.92f,.95f},{.85f,.88f,.92f},{.88f,.90f,.94f},{.80f,.82f,.88f},{.90f,.92f,.95f},{.60f,.80f,.95f}},
{"Night Stalker",    SkinStyle::Military, {.55f,.38f,.26f},{.08f,.08f,.10f},{.10f,.10f,.12f},{.15f,.16f,.18f},{.08f,.08f,.10f},{.06f,.06f,.08f},{.10f,.10f,.12f},{.20f,.80f,.30f}},
{"Steel Commando",   SkinStyle::Military, {.75f,.58f,.40f},{.35f,.35f,.38f},{.38f,.40f,.44f},{.50f,.52f,.56f},{.35f,.36f,.40f},{.28f,.28f,.32f},{.38f,.40f,.44f},{.85f,.62f,.08f}},
{"Sand Viper",       SkinStyle::Military, {.82f,.68f,.50f},{.72f,.62f,.38f},{.68f,.58f,.34f},{.80f,.70f,.44f},{.64f,.54f,.30f},{.52f,.42f,.22f},{.68f,.58f,.34f},{.90f,.30f,.15f}},
{"Wolf Pack",        SkinStyle::Military, {.78f,.60f,.42f},{.30f,.28f,.26f},{.28f,.26f,.24f},{.38f,.36f,.34f},{.25f,.23f,.21f},{.18f,.16f,.14f},{.28f,.26f,.24f},{.70f,.70f,.75f}},
{"Iron Sentinel",    SkinStyle::Military, {.72f,.55f,.38f},{.25f,.24f,.22f},{.40f,.38f,.35f},{.55f,.52f,.48f},{.38f,.36f,.34f},{.30f,.28f,.26f},{.40f,.38f,.35f},{.88f,.88f,.90f}},
{"Black Ops",        SkinStyle::Military, {.50f,.35f,.22f},{.06f,.06f,.07f},{.08f,.08f,.09f},{.12f,.12f,.14f},{.07f,.07f,.08f},{.05f,.05f,.06f},{.08f,.08f,.09f},{.10f,.90f,.35f}},

// ── STYLE 2: SCI-FI ────────────────────────────────────────────────────────────
{"Neon Phantom",     SkinStyle::SciFi,    {.80f,.90f,.95f},{.05f,.05f,.10f},{.08f,.10f,.18f},{.12f,.15f,.25f},{.06f,.08f,.16f},{.05f,.06f,.12f},{.10f,.90f,.95f},{.10f,.90f,.95f}},
{"Cyber Specter",    SkinStyle::SciFi,    {.85f,.92f,.88f},{.10f,.10f,.12f},{.10f,.14f,.18f},{.18f,.22f,.28f},{.08f,.12f,.16f},{.06f,.08f,.12f},{.20f,.95f,.60f},{.20f,.95f,.60f}},
{"Plasma Knight",    SkinStyle::SciFi,    {.75f,.85f,.95f},{.08f,.06f,.12f},{.12f,.06f,.20f},{.20f,.10f,.32f},{.10f,.05f,.18f},{.08f,.04f,.14f},{.90f,.30f,.95f},{.90f,.30f,.95f}},
{"Void Walker",      SkinStyle::SciFi,    {.60f,.65f,.75f},{.05f,.05f,.08f},{.06f,.06f,.10f},{.10f,.10f,.16f},{.05f,.05f,.08f},{.04f,.04f,.07f},{.15f,.15f,.95f},{.15f,.15f,.95f}},
{"Quantum Reaper",   SkinStyle::SciFi,    {.90f,.85f,.80f},{.08f,.08f,.06f},{.15f,.08f,.06f},{.25f,.14f,.10f},{.12f,.06f,.04f},{.09f,.05f,.04f},{.95f,.50f,.10f},{.95f,.50f,.10f}},
{"Hex Core",         SkinStyle::SciFi,    {.75f,.90f,.80f},{.06f,.10f,.06f},{.06f,.14f,.08f},{.10f,.22f,.12f},{.05f,.12f,.06f},{.04f,.09f,.05f},{.20f,.95f,.40f},{.20f,.95f,.40f}},
{"Signal Ghost",     SkinStyle::SciFi,    {.92f,.92f,.95f},{.10f,.10f,.14f},{.14f,.14f,.20f},{.20f,.20f,.28f},{.12f,.12f,.18f},{.09f,.09f,.14f},{.95f,.95f,.20f},{.95f,.95f,.20f}},
{"Data Wraith",      SkinStyle::SciFi,    {.70f,.80f,.90f},{.08f,.08f,.12f},{.10f,.12f,.18f},{.15f,.18f,.28f},{.08f,.10f,.16f},{.06f,.08f,.12f},{.20f,.60f,.95f},{.20f,.60f,.95f}},
{"Chrome Edge",      SkinStyle::SciFi,    {.88f,.88f,.90f},{.60f,.65f,.70f},{.55f,.60f,.68f},{.78f,.82f,.88f},{.50f,.55f,.62f},{.45f,.48f,.55f},{.90f,.90f,.92f},{.90f,.90f,.92f}},
{"Laser Grid",       SkinStyle::SciFi,    {.80f,.90f,.85f},{.10f,.06f,.08f},{.12f,.06f,.10f},{.20f,.10f,.18f},{.10f,.05f,.08f},{.08f,.04f,.06f},{.95f,.15f,.60f},{.95f,.15f,.60f}},

// ── STYLE 3: CASUAL ────────────────────────────────────────────────────────────
{"Street King",      SkinStyle::Casual,   {.87f,.68f,.52f},{.06f,.06f,.06f},{.15f,.15f,.55f},{.92f,.92f,.94f},{.20f,.20f,.20f},{.08f,.08f,.10f},{.87f,.68f,.52f},{.85f,.15f,.15f}},
{"Urban Legend",     SkinStyle::Casual,   {.75f,.55f,.35f},{.10f,.08f,.06f},{.20f,.20f,.22f},{.85f,.85f,.88f},{.12f,.12f,.14f},{.08f,.08f,.10f},{.75f,.55f,.35f},{.40f,.65f,.90f}},
{"Camo Rider",       SkinStyle::Casual,   {.80f,.62f,.44f},{.22f,.18f,.12f},{.35f,.42f,.25f},{.42f,.50f,.30f},{.30f,.38f,.22f},{.22f,.26f,.15f},{.80f,.62f,.44f},{.90f,.72f,.20f}},
{"Wave Breaker",     SkinStyle::Casual,   {.88f,.75f,.60f},{.25f,.45f,.65f},{.15f,.40f,.70f},{.20f,.55f,.85f},{.12f,.35f,.60f},{.10f,.28f,.50f},{.88f,.75f,.60f},{.95f,.85f,.20f}},
{"Track Star",       SkinStyle::Casual,   {.80f,.60f,.42f},{.08f,.06f,.06f},{.80f,.15f,.15f},{.95f,.95f,.97f},{.12f,.12f,.15f},{.08f,.06f,.06f},{.80f,.60f,.42f},{.95f,.85f,.10f}},
{"Grunge Ace",       SkinStyle::Casual,   {.78f,.60f,.42f},{.20f,.16f,.12f},{.30f,.22f,.18f},{.70f,.62f,.54f},{.22f,.16f,.12f},{.16f,.12f,.08f},{.78f,.60f,.42f},{.60f,.60f,.65f}},
{"Summer Ghost",     SkinStyle::Casual,   {.92f,.80f,.65f},{.90f,.82f,.50f},{.95f,.72f,.30f},{.98f,.92f,.80f},{.88f,.68f,.25f},{.75f,.55f,.18f},{.92f,.80f,.65f},{.95f,.40f,.20f}},
{"Neon Surge",       SkinStyle::Casual,   {.85f,.75f,.62f},{.08f,.06f,.06f},{.06f,.06f,.08f},{.30f,.95f,.60f},{.06f,.06f,.08f},{.05f,.05f,.07f},{.85f,.75f,.62f},{.30f,.95f,.60f}},
{"City Hawk",        SkinStyle::Casual,   {.82f,.65f,.48f},{.15f,.12f,.10f},{.35f,.35f,.38f},{.80f,.80f,.85f},{.22f,.22f,.25f},{.14f,.14f,.16f},{.82f,.65f,.48f},{.50f,.75f,.95f}},
{"Rebel Fade",       SkinStyle::Casual,   {.78f,.58f,.40f},{.08f,.06f,.06f},{.40f,.08f,.08f},{.55f,.55f,.60f},{.10f,.08f,.08f},{.08f,.06f,.06f},{.78f,.58f,.40f},{.55f,.55f,.60f}},

// ── STYLE 4: ARMOR ─────────────────────────────────────────────────────────────
{"Iron Templar",     SkinStyle::Armor,    {.82f,.70f,.58f},{.40f,.40f,.44f},{.45f,.48f,.52f},{.62f,.65f,.70f},{.42f,.44f,.48f},{.38f,.40f,.44f},{.45f,.48f,.52f},{.84f,.62f,.08f}},
{"Storm Paladin",    SkinStyle::Armor,    {.85f,.72f,.60f},{.55f,.60f,.70f},{.40f,.48f,.65f},{.60f,.70f,.88f},{.38f,.44f,.60f},{.30f,.36f,.52f},{.40f,.48f,.65f},{.95f,.90f,.40f}},
{"Obsidian Guard",   SkinStyle::Armor,    {.60f,.45f,.30f},{.08f,.08f,.08f},{.10f,.10f,.12f},{.16f,.16f,.18f},{.08f,.08f,.10f},{.06f,.06f,.08f},{.10f,.10f,.12f},{.50f,.10f,.90f}},
{"Gold Sentinel",    SkinStyle::Armor,    {.87f,.72f,.58f},{.70f,.52f,.08f},{.72f,.54f,.10f},{.85f,.68f,.15f},{.68f,.50f,.08f},{.60f,.44f,.06f},{.72f,.54f,.10f},{.95f,.78f,.20f}},
{"Dragon Warden",    SkinStyle::Armor,    {.82f,.68f,.52f},{.35f,.12f,.06f},{.40f,.14f,.08f},{.55f,.20f,.10f},{.36f,.12f,.06f},{.28f,.10f,.05f},{.40f,.14f,.08f},{.95f,.50f,.10f}},
{"Crystal Knight",   SkinStyle::Armor,    {.88f,.88f,.92f},{.75f,.85f,.95f},{.70f,.82f,.95f},{.85f,.92f,.98f},{.68f,.80f,.92f},{.60f,.72f,.88f},{.70f,.82f,.95f},{.95f,.98f,.98f}},
{"Ember Warden",     SkinStyle::Armor,    {.85f,.68f,.50f},{.30f,.14f,.06f},{.35f,.16f,.06f},{.55f,.24f,.08f},{.30f,.12f,.05f},{.22f,.08f,.04f},{.35f,.16f,.06f},{.95f,.60f,.10f}},
{"Void Crusader",    SkinStyle::Armor,    {.65f,.60f,.70f},{.10f,.08f,.15f},{.12f,.10f,.18f},{.20f,.16f,.28f},{.10f,.08f,.16f},{.08f,.06f,.12f},{.12f,.10f,.18f},{.60f,.20f,.95f}},
{"Silver Shield",    SkinStyle::Armor,    {.90f,.80f,.70f},{.75f,.75f,.78f},{.70f,.70f,.75f},{.85f,.85f,.88f},{.65f,.65f,.70f},{.58f,.58f,.62f},{.70f,.70f,.75f},{.45f,.65f,.90f}},
{"Thunder Plate",    SkinStyle::Armor,    {.82f,.72f,.62f},{.35f,.32f,.28f},{.38f,.35f,.30f},{.52f,.48f,.42f},{.35f,.32f,.28f},{.28f,.25f,.22f},{.38f,.35f,.30f},{.95f,.90f,.20f}},

// ── STYLE 5: NINJA ─────────────────────────────────────────────────────────────
{"Shadow Dancer",    SkinStyle::Ninja,    {.50f,.35f,.22f},{.08f,.06f,.06f},{.08f,.06f,.06f},{.12f,.10f,.10f},{.06f,.05f,.05f},{.05f,.04f,.04f},{.08f,.06f,.06f},{.20f,.95f,.60f}},
{"Crimson Phantom",  SkinStyle::Ninja,    {.50f,.35f,.22f},{.08f,.06f,.06f},{.40f,.06f,.06f},{.55f,.10f,.10f},{.38f,.05f,.05f},{.30f,.04f,.04f},{.40f,.06f,.06f},{.95f,.90f,.90f}},
{"Silent Viper",     SkinStyle::Ninja,    {.50f,.35f,.22f},{.08f,.10f,.06f},{.08f,.12f,.06f},{.12f,.18f,.08f},{.06f,.10f,.05f},{.05f,.08f,.04f},{.08f,.12f,.06f},{.60f,.95f,.20f}},
{"Dark Lotus",       SkinStyle::Ninja,    {.55f,.40f,.28f},{.10f,.05f,.10f},{.12f,.06f,.14f},{.18f,.09f,.20f},{.10f,.05f,.12f},{.08f,.04f,.10f},{.12f,.06f,.14f},{.90f,.30f,.90f}},
{"Ghost Blade",      SkinStyle::Ninja,    {.88f,.88f,.90f},{.85f,.86f,.88f},{.82f,.84f,.86f},{.90f,.92f,.94f},{.80f,.82f,.84f},{.75f,.78f,.80f},{.82f,.84f,.86f},{.95f,.95f,.98f}},
{"Night Whisper",    SkinStyle::Ninja,    {.50f,.35f,.22f},{.06f,.05f,.08f},{.06f,.05f,.08f},{.10f,.08f,.12f},{.05f,.04f,.07f},{.04f,.04f,.06f},{.06f,.05f,.08f},{.20f,.20f,.95f}},
{"Azure Shadow",     SkinStyle::Ninja,    {.55f,.40f,.28f},{.08f,.10f,.20f},{.08f,.12f,.25f},{.12f,.18f,.35f},{.06f,.10f,.22f},{.05f,.08f,.18f},{.08f,.12f,.25f},{.40f,.70f,.95f}},
{"Steel Wraith",     SkinStyle::Ninja,    {.60f,.45f,.30f},{.45f,.46f,.50f},{.42f,.44f,.48f},{.55f,.57f,.62f},{.40f,.42f,.46f},{.35f,.36f,.40f},{.42f,.44f,.48f},{.78f,.80f,.85f}},
{"Eclipse Runner",   SkinStyle::Ninja,    {.50f,.35f,.22f},{.08f,.08f,.06f},{.10f,.10f,.06f},{.18f,.18f,.10f},{.08f,.08f,.05f},{.06f,.06f,.04f},{.10f,.10f,.06f},{.95f,.80f,.10f}},
{"Toxic Shade",      SkinStyle::Ninja,    {.55f,.40f,.28f},{.10f,.14f,.06f},{.10f,.16f,.06f},{.16f,.24f,.08f},{.08f,.14f,.05f},{.06f,.10f,.04f},{.10f,.16f,.06f},{.55f,.95f,.15f}},

// ── STYLE 6: COWBOY ────────────────────────────────────────────────────────────
{"Dusty Ranger",     SkinStyle::Cowboy,   {.87f,.68f,.52f},{.52f,.35f,.18f},{.55f,.38f,.22f},{.72f,.54f,.32f},{.48f,.32f,.18f},{.38f,.24f,.12f},{.87f,.68f,.52f},{.80f,.55f,.20f}},
{"Lone Marshal",     SkinStyle::Cowboy,   {.82f,.65f,.48f},{.20f,.14f,.08f},{.20f,.16f,.12f},{.40f,.32f,.22f},{.16f,.12f,.08f},{.12f,.09f,.06f},{.82f,.65f,.48f},{.70f,.50f,.15f}},
{"Rustbelt Sheriff", SkinStyle::Cowboy,   {.80f,.62f,.44f},{.42f,.28f,.15f},{.55f,.28f,.12f},{.70f,.38f,.18f},{.50f,.24f,.10f},{.40f,.18f,.08f},{.80f,.62f,.44f},{.90f,.65f,.15f}},
{"Iron Outlaw",      SkinStyle::Cowboy,   {.78f,.58f,.40f},{.12f,.08f,.05f},{.12f,.10f,.08f},{.20f,.16f,.12f},{.10f,.08f,.06f},{.08f,.06f,.04f},{.78f,.58f,.40f},{.65f,.45f,.10f}},
{"Desert Hawk",      SkinStyle::Cowboy,   {.85f,.70f,.54f},{.60f,.48f,.28f},{.62f,.50f,.28f},{.75f,.62f,.38f},{.58f,.46f,.26f},{.48f,.36f,.18f},{.85f,.70f,.54f},{.92f,.72f,.20f}},
{"Boot Hill",        SkinStyle::Cowboy,   {.88f,.72f,.56f},{.35f,.22f,.10f},{.38f,.24f,.14f},{.55f,.38f,.22f},{.32f,.20f,.10f},{.25f,.15f,.07f},{.88f,.72f,.56f},{.75f,.52f,.18f}},
{"Gold Rush",        SkinStyle::Cowboy,   {.90f,.75f,.58f},{.72f,.52f,.10f},{.72f,.52f,.10f},{.88f,.68f,.16f},{.68f,.48f,.08f},{.58f,.40f,.06f},{.90f,.75f,.58f},{.95f,.78f,.20f}},
{"Prairie Ghost",    SkinStyle::Cowboy,   {.90f,.82f,.70f},{.88f,.82f,.72f},{.88f,.82f,.70f},{.95f,.90f,.80f},{.85f,.78f,.66f},{.78f,.70f,.58f},{.90f,.82f,.70f},{.85f,.72f,.40f}},
{"Scarecrow",        SkinStyle::Cowboy,   {.82f,.68f,.50f},{.35f,.28f,.16f},{.55f,.42f,.25f},{.70f,.56f,.32f},{.48f,.36f,.20f},{.38f,.28f,.14f},{.82f,.68f,.50f},{.85f,.62f,.18f}},
{"Red Ridge",        SkinStyle::Cowboy,   {.85f,.68f,.50f},{.40f,.20f,.10f},{.48f,.18f,.10f},{.65f,.25f,.14f},{.42f,.15f,.08f},{.35f,.12f,.06f},{.85f,.68f,.50f},{.90f,.65f,.15f}},

// ── STYLE 7: ROBOT ─────────────────────────────────────────────────────────────
{"Unit X-7",         SkinStyle::Robot,    {.60f,.65f,.70f},{.30f,.32f,.35f},{.35f,.38f,.42f},{.50f,.54f,.58f},{.32f,.35f,.38f},{.25f,.28f,.30f},{.55f,.58f,.62f},{.10f,.90f,.95f}},
{"Mech Prime",       SkinStyle::Robot,    {.55f,.58f,.62f},{.20f,.20f,.22f},{.22f,.22f,.25f},{.35f,.36f,.40f},{.20f,.20f,.22f},{.16f,.16f,.18f},{.45f,.48f,.52f},{.95f,.50f,.10f}},
{"Steel Nexus",      SkinStyle::Robot,    {.70f,.72f,.75f},{.48f,.50f,.54f},{.50f,.52f,.56f},{.65f,.68f,.72f},{.48f,.50f,.54f},{.40f,.42f,.46f},{.65f,.68f,.72f},{.20f,.20f,.95f}},
{"Binary Storm",     SkinStyle::Robot,    {.50f,.55f,.60f},{.12f,.12f,.14f},{.14f,.14f,.16f},{.22f,.22f,.26f},{.12f,.12f,.14f},{.09f,.09f,.11f},{.28f,.30f,.35f},{.10f,.95f,.30f}},
{"Circuit King",     SkinStyle::Robot,    {.65f,.68f,.72f},{.38f,.38f,.40f},{.40f,.40f,.44f},{.55f,.56f,.60f},{.38f,.38f,.40f},{.30f,.30f,.32f},{.50f,.52f,.55f},{.95f,.90f,.20f}},
{"Omega Frame",      SkinStyle::Robot,    {.55f,.52f,.58f},{.18f,.14f,.22f},{.20f,.16f,.25f},{.32f,.26f,.38f},{.18f,.14f,.22f},{.14f,.10f,.18f},{.35f,.28f,.42f},{.80f,.20f,.95f}},
{"Titan Core",       SkinStyle::Robot,    {.68f,.65f,.62f},{.30f,.28f,.26f},{.35f,.32f,.28f},{.48f,.44f,.40f},{.32f,.28f,.26f},{.25f,.22f,.20f},{.45f,.42f,.38f},{.95f,.65f,.10f}},
{"Delta Array",      SkinStyle::Robot,    {.60f,.70f,.65f},{.22f,.28f,.25f},{.24f,.30f,.28f},{.36f,.44f,.40f},{.22f,.28f,.25f},{.18f,.22f,.20f},{.38f,.46f,.42f},{.20f,.95f,.70f}},
{"Vex Protocol",     SkinStyle::Robot,    {.55f,.55f,.60f},{.10f,.10f,.15f},{.12f,.12f,.18f},{.20f,.20f,.28f},{.10f,.10f,.16f},{.08f,.08f,.12f},{.25f,.25f,.35f},{.90f,.10f,.30f}},
{"Alpha Node",       SkinStyle::Robot,    {.72f,.75f,.78f},{.55f,.58f,.60f},{.58f,.62f,.65f},{.72f,.76f,.80f},{.55f,.58f,.62f},{.48f,.50f,.54f},{.68f,.72f,.75f},{.95f,.95f,.95f}},

// ── STYLE 8: MEDIC ─────────────────────────────────────────────────────────────
{"Field Medic",      SkinStyle::Medic,    {.87f,.68f,.52f},{.95f,.95f,.97f},{.90f,.90f,.92f},{.95f,.15f,.15f},{.85f,.85f,.88f},{.78f,.78f,.80f},{.87f,.68f,.52f},{.95f,.15f,.15f}},
{"Combat Doc",       SkinStyle::Medic,    {.80f,.62f,.44f},{.85f,.82f,.70f},{.72f,.70f,.60f},{.95f,.15f,.15f},{.68f,.66f,.56f},{.58f,.56f,.46f},{.80f,.62f,.44f},{.95f,.15f,.15f}},
{"Trauma Hawk",      SkinStyle::Medic,    {.85f,.70f,.56f},{.38f,.32f,.28f},{.30f,.28f,.26f},{.95f,.15f,.15f},{.28f,.26f,.24f},{.22f,.20f,.18f},{.85f,.70f,.56f},{.95f,.15f,.15f}},
{"Patch Master",     SkinStyle::Medic,    {.90f,.78f,.64f},{.90f,.88f,.80f},{.88f,.88f,.90f},{.95f,.15f,.15f},{.82f,.82f,.85f},{.75f,.75f,.78f},{.90f,.78f,.64f},{.15f,.75f,.95f}},
{"LifeLine",         SkinStyle::Medic,    {.88f,.75f,.60f},{.10f,.10f,.12f},{.12f,.12f,.14f},{.95f,.15f,.15f},{.10f,.10f,.12f},{.08f,.08f,.10f},{.88f,.75f,.60f},{.20f,.95f,.50f}},
{"Angel Squad",      SkinStyle::Medic,    {.94f,.88f,.80f},{.95f,.95f,.97f},{.90f,.90f,.95f},{.95f,.15f,.15f},{.88f,.88f,.92f},{.82f,.82f,.86f},{.94f,.88f,.80f},{.60f,.80f,.95f}},
{"Code Blue",        SkinStyle::Medic,    {.82f,.68f,.52f},{.55f,.60f,.75f},{.48f,.56f,.80f},{.95f,.15f,.15f},{.44f,.52f,.74f},{.36f,.44f,.66f},{.82f,.68f,.52f},{.95f,.15f,.15f}},
{"Vitals",           SkinStyle::Medic,    {.88f,.75f,.60f},{.20f,.20f,.22f},{.22f,.22f,.24f},{.95f,.15f,.15f},{.18f,.18f,.20f},{.14f,.14f,.16f},{.88f,.75f,.60f},{.95f,.85f,.10f}},
{"Iron Cross",       SkinStyle::Medic,    {.80f,.65f,.50f},{.30f,.28f,.26f},{.55f,.50f,.44f},{.95f,.15f,.15f},{.50f,.46f,.40f},{.40f,.36f,.30f},{.80f,.65f,.50f},{.95f,.15f,.15f}},
{"Remedy",           SkinStyle::Medic,    {.90f,.80f,.68f},{.90f,.90f,.92f},{.85f,.88f,.90f},{.95f,.15f,.15f},{.80f,.84f,.88f},{.72f,.76f,.80f},{.90f,.80f,.68f},{.25f,.95f,.65f}},

// ── STYLE 9: RAIDER ────────────────────────────────────────────────────────────
{"Scrap King",       SkinStyle::Raider,   {.72f,.55f,.38f},{.35f,.25f,.15f},{.40f,.30f,.20f},{.55f,.42f,.28f},{.38f,.28f,.18f},{.28f,.20f,.12f},{.60f,.55f,.42f},{.90f,.55f,.10f}},
{"Road Razer",       SkinStyle::Raider,   {.78f,.60f,.42f},{.20f,.18f,.15f},{.30f,.25f,.18f},{.45f,.38f,.25f},{.28f,.22f,.15f},{.20f,.16f,.10f},{.55f,.50f,.38f},{.85f,.40f,.10f}},
{"Rust Reaper",      SkinStyle::Raider,   {.70f,.52f,.35f},{.40f,.25f,.12f},{.45f,.28f,.14f},{.62f,.38f,.18f},{.42f,.25f,.12f},{.32f,.18f,.08f},{.55f,.42f,.28f},{.90f,.35f,.10f}},
{"Junk Lord",        SkinStyle::Raider,   {.75f,.56f,.38f},{.48f,.38f,.25f},{.52f,.40f,.26f},{.65f,.50f,.32f},{.48f,.36f,.22f},{.38f,.28f,.15f},{.60f,.52f,.38f},{.92f,.70f,.15f}},
{"Wasteland Wolf",   SkinStyle::Raider,   {.78f,.60f,.42f},{.30f,.25f,.20f},{.35f,.28f,.22f},{.50f,.42f,.32f},{.32f,.25f,.18f},{.24f,.18f,.12f},{.58f,.52f,.40f},{.85f,.60f,.10f}},
{"Iron Punk",        SkinStyle::Raider,   {.72f,.55f,.38f},{.15f,.12f,.10f},{.20f,.16f,.12f},{.35f,.28f,.20f},{.18f,.14f,.10f},{.12f,.09f,.07f},{.50f,.45f,.32f},{.95f,.85f,.10f}},
{"Wreck Rider",      SkinStyle::Raider,   {.80f,.62f,.44f},{.38f,.28f,.16f},{.42f,.32f,.18f},{.58f,.44f,.26f},{.40f,.28f,.16f},{.30f,.20f,.10f},{.62f,.55f,.40f},{.88f,.50f,.12f}},
{"Scorch Outlaw",    SkinStyle::Raider,   {.75f,.55f,.36f},{.20f,.14f,.08f},{.35f,.18f,.08f},{.55f,.28f,.10f},{.32f,.16f,.06f},{.22f,.10f,.04f},{.55f,.45f,.30f},{.95f,.55f,.10f}},
{"Dust Demon",       SkinStyle::Raider,   {.70f,.50f,.32f},{.28f,.20f,.12f},{.35f,.26f,.16f},{.50f,.38f,.22f},{.32f,.22f,.12f},{.22f,.15f,.08f},{.52f,.46f,.32f},{.90f,.45f,.10f}},
{"Chain Wrecker",    SkinStyle::Raider,   {.76f,.58f,.40f},{.22f,.18f,.12f},{.30f,.24f,.15f},{.48f,.38f,.22f},{.28f,.20f,.12f},{.20f,.14f,.08f},{.55f,.48f,.35f},{.88f,.62f,.12f}},
};

inline constexpr int kSkinCount = 100;

inline const char* skinStyleName(SkinStyle s) {
    switch (s) {
        case SkinStyle::Suit:     return "SUIT";
        case SkinStyle::Military: return "MILITARY";
        case SkinStyle::SciFi:    return "SCI-FI";
        case SkinStyle::Casual:   return "CASUAL";
        case SkinStyle::Armor:    return "ARMOR";
        case SkinStyle::Ninja:    return "NINJA";
        case SkinStyle::Cowboy:   return "COWBOY";
        case SkinStyle::Robot:    return "ROBOT";
        case SkinStyle::Medic:    return "MEDIC";
        case SkinStyle::Raider:   return "RAIDER";
        default:                  return "UNKNOWN";
    }
}
