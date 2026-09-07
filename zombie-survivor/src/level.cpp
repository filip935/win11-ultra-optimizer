#include "level.h"
#include <cmath>
#include <stdio.h>
#include <string.h>

void InitLevels(Level levels[]) {
    for (int i = 0; i < MAX_LEVEL; i++) {
        Level& l = levels[i];
        l.levelNumber = i + 1;
        l.completed = false;
        l.spawnTimer = 0;
        l.timeLimit = 60 + i * 5;

        if ((i + 1) % 10 == 0) {
            l.bossLevel = 1;
            l.zombiesRequired = 1 + (i / 10);
            l.maxZombies = 5;
            l.spawnRate = 3.0f;
            l.primaryZombie = ZOMBIE_BOSS;
        } else {
            l.bossLevel = 0;
            l.zombiesRequired = 10 + i * 3;
            l.maxZombies = 20 + i;
            float rate = 2.0f - i * 0.015f;
            l.spawnRate = rate > 0.3f ? rate : 0.3f;
            l.primaryZombie = (i < 20) ? ZOMBIE_WALKER :
                              (i < 50) ? ZOMBIE_RUNNER :
                              (i < 80) ? ZOMBIE_BRUTE : ZOMBIE_SPITTER;
        }

        l.difficultyMultiplier = 1.0f + i * 0.05f;
        l.zombiesSpawned = 0;
    }
}

void UpdateLevel(Level& level, float dt) {
    level.spawnTimer -= dt;
}

void GetLevelDescription(int levelNum, char* buffer, int bufSize) {
    if (levelNum == 1) {
        snprintf(buffer, bufSize, "The city streets run red. Find your footing.");
    } else if (levelNum == 5) {
        snprintf(buffer, bufSize, "They're learning. Adapting.");
    } else if (levelNum == 10) {
        snprintf(buffer, bufSize, "Mutations detected. Brutes incoming.");
    } else if (levelNum == 15) {
        snprintf(buffer, bufSize, "Fast ones now. Don't let them flank you.");
    } else if (levelNum == 20) {
        snprintf(buffer, bufSize, "Acid spitters. Keep your distance.");
    } else if (levelNum == 25) {
        snprintf(buffer, bufSize, "The horde is organizing. Stay sharp.");
    } else if (levelNum == 30) {
        snprintf(buffer, bufSize, "1.5x tougher. They're evolving fast.");
    } else if (levelNum == 40) {
        snprintf(buffer, bufSize, "Other zones have fallen. You're all that's left.");
    } else if (levelNum == 50) {
        snprintf(buffer, bufSize, "Halfway. They're 2x stronger now.");
    } else if (levelNum == 60) {
        snprintf(buffer, bufSize, "The Alpha is directing the horde.");
    } else if (levelNum == 70) {
        snprintf(buffer, bufSize, "2.5x mutation level. Extreme danger.");
    } else if (levelNum == 75) {
        snprintf(buffer, bufSize, "The cure needs one final sample.");
    } else if (levelNum == 80) {
        snprintf(buffer, bufSize, "3x tougher. But you're tougher.");
    } else if (levelNum == 90) {
        snprintf(buffer, bufSize, "The Alpha knows you're coming.");
    } else if (levelNum == 95) {
        snprintf(buffer, bufSize, "Five more levels. For Adam.");
    } else if (levelNum == 100) {
        snprintf(buffer, bufSize, "THE ALPHA. End this.");
    } else if (levelNum % 10 == 0) {
        snprintf(buffer, bufSize, "BOSS LEVEL %d - Mutation surge", levelNum);
    } else {
        snprintf(buffer, bufSize, "Level %d - The city fights back", levelNum);
    }
}
