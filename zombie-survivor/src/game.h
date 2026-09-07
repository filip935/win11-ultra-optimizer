#pragma once
#include "raylib.h"
#include "player.h"
#include "zombie.h"
#include "weapon.h"
#include "level.h"
#include "loot.h"
#include "story.h"
#include "ui.h"
#include "particles.h"

#define MAX_BUILDINGS 60
#define REQUIRED_SAMPLES 10

enum GameState {
    INTRO,
    MENU,
    PLAYING,
    PAUSED,
    LEVEL_COMPLETE,
    STORY,
    GAME_OVER,
    VICTORY,
    CURE_CHOICE,
    ENDING_KILL,
    ENDING_CURE
};

struct Building {
    Rectangle rect;
    Color color;
};

struct Game {
    GameState state;
    int currentLevel;
    float playTime;
    Camera2D camera;
    Player player;
    Zombie zombies[MAX_ZOMBIES];
    int zombieCount;
    Projectile projectiles[MAX_PROJECTILES];
    int projectileCount;
    Weapon weapons[WEAPON_COUNT];
    Weapon inventory[MAX_WEAPON_SLOTS];
    bool slotUnlocked[MAX_WEAPON_SLOTS];
    int currentWeapon;
    Loot loots[MAX_LOOTS];
    int lootCount;
    Level levels[MAX_LEVEL];
    Particle particles[MAX_PARTICLES];
    int particleCount;
    Story story;
    UI ui;
    float levelTimer;
    int zombiesKilledThisLevel;
    int zombiesRequiredThisLevel;
    bool screenShake;
    float shakeIntensity;
    FallingChest chest;
    bool chestSpawnedThisLevel;
    Building buildings[MAX_BUILDINGS];
    int buildingCount;
    int cureSamples;
    int choiceSelection;
    float endingTimer;
};

void InitGame(Game& game);
void GenerateCity(Game& game);
void UpdateGame(Game& game, float dt);
void DrawGame(Game& game);
void CleanupGame(Game& game);
bool CheckBuildingCollision(Game& game, Vector2 pos, float radius);
