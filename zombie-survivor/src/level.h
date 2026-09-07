#pragma once
#include "raylib.h"
#include "zombie.h"

#define MAX_LEVEL 100
#define ARENA_WIDTH 2000
#define ARENA_HEIGHT 2000

struct Level {
    int levelNumber;
    int zombiesRequired;
    int zombiesSpawned;
    int maxZombies;
    float spawnRate;
    float spawnTimer;
    int bossLevel;
    ZombieType primaryZombie;
    float difficultyMultiplier;
    float timeLimit;
    bool completed;
};

void InitLevels(Level levels[]);
void UpdateLevel(Level& level, float dt);
void GetLevelDescription(int levelNum, char* buffer, int bufSize);
