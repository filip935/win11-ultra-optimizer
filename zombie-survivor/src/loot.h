#pragma once
#include "raylib.h"
#include "weapon.h"
#include "zombie.h"

#define MAX_LOOTS 50
#define MAX_BUILDINGS_CHECK 60

struct BuildingRef {
    Rectangle rect;
};

enum LootType {
    LOOT_AMMO,
    LOOT_HEALTH,
    LOOT_HEALTH_POTION,
    LOOT_ARMOR,
    LOOT_WEAPON,
    LOOT_XP,
    LOOT_COIN,
    LOOT_CURE_SAMPLE
};

struct Loot {
    Vector2 position;
    LootType type;
    Weapon weapon;
    int amount;
    bool active;
    float lifetime;
    float bobOffset;
    Color color;
};

struct FallingChest {
    bool active;
    bool falling;
    bool opened;
    Vector2 position;
    Vector2 targetPos;
    float fallSpeed;
    float openTimer;
    float lifetime;
    int lootCount;
    Loot loots[8];
};

bool IsInsideBuilding(Vector2 pos, BuildingRef* buildings, int buildingCount);
Vector2 FindClearPosition(Vector2 pos, BuildingRef* buildings, int buildingCount, float radius);
void SpawnLoot(Loot* loots, int& count, Vector2 pos, ZombieType fromZombie, int level, BuildingRef* buildings, int buildingCount);
void SpawnChestLoot(FallingChest& chest, int level, BuildingRef* buildings, int buildingCount);
void UpdateLoots(Loot* loots, int& count, float dt);
void DrawLoots(Loot* loots, int count);
void CollectLoot(Loot& loot, Weapon inventory[], bool slotUnlocked[], int& currentWeapon, int* health, int maxHealth, int* cureSamples);
void InitChest(FallingChest& chest, Vector2 pos, BuildingRef* buildings, int buildingCount);
void UpdateChest(FallingChest& chest, float dt, Vector2 playerPos, Loot* loots, int* lootCount, int level, BuildingRef* buildings, int buildingCount);
void DrawChest(FallingChest& chest);
