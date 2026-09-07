#include "loot.h"
#include "player.h"
#include <cmath>

bool IsInsideBuilding(Vector2 pos, BuildingRef* buildings, int buildingCount) {
    for (int i = 0; i < buildingCount; i++) {
        Rectangle r = buildings[i].rect;
        if (pos.x >= r.x && pos.x <= r.x + r.width &&
            pos.y >= r.y && pos.y <= r.y + r.height) {
            return true;
        }
    }
    return false;
}

static bool IsNearBuilding(Vector2 pos, BuildingRef* buildings, int buildingCount, float margin) {
    for (int i = 0; i < buildingCount; i++) {
        Rectangle r = buildings[i].rect;
        if (pos.x >= r.x - margin && pos.x <= r.x + r.width + margin &&
            pos.y >= r.y - margin && pos.y <= r.y + r.height + margin) {
            return true;
        }
    }
    return false;
}

Vector2 FindClearPosition(Vector2 pos, BuildingRef* buildings, int buildingCount, float radius) {
    if (!IsNearBuilding(pos, buildings, buildingCount, radius)) return pos;

    for (float dist = 20; dist < 300; dist += 20) {
        for (float angle = 0; angle < 6.28f; angle += 0.5f) {
            Vector2 test = { pos.x + cosf(angle) * dist, pos.y + sinf(angle) * dist };
            if (!IsNearBuilding(test, buildings, buildingCount, radius)) return test;
        }
    }
    return pos;
}

void SpawnLoot(Loot* loots, int& count, Vector2 pos, ZombieType fromZombie, int level, BuildingRef* buildings, int buildingCount) {
    if (count >= MAX_LOOTS) return;

    float dropChance = 0.35f;
    if (fromZombie == ZOMBIE_BRUTE) dropChance = 0.65f;
    if (fromZombie == ZOMBIE_BOSS) dropChance = 1.0f;

    int maxDrops = 1;
    if (fromZombie == ZOMBIE_BRUTE) maxDrops = 2;
    if (fromZombie == ZOMBIE_BOSS) maxDrops = 4;

    for (int d = 0; d < maxDrops; d++) {
        if ((float)GetRandomValue(0, 100) / 100.0f > dropChance) continue;
        if (count >= MAX_LOOTS) break;

        Loot& l = loots[count];
        l.position = { pos.x + (float)GetRandomValue(-20, 20), pos.y + (float)GetRandomValue(-20, 20) };
        l.position = FindClearPosition(l.position, buildings, buildingCount, 15);
        l.active = true;
        l.lifetime = 45.0f;
        l.bobOffset = (float)GetRandomValue(0, 628) / 100.0f;
        l.weapon = {};

        int roll = GetRandomValue(0, 100);

        if (fromZombie == ZOMBIE_BOSS) {
            l.type = LOOT_WEAPON;
            l.weapon = GenerateRandomWeapon(level);
            l.color = l.weapon.rarityColor;
        } else if (roll < 15) {
            l.type = LOOT_CURE_SAMPLE;
            l.amount = 1;
            l.color = SKYBLUE;
        } else if (roll < 35) {
            l.type = LOOT_AMMO;
            l.amount = GetRandomValue(15, 40);
            l.color = ORANGE;
        } else if (roll < 50) {
            l.type = LOOT_HEALTH_POTION;
            l.amount = GetRandomValue(30, 60);
            l.color = PINK;
        } else if (roll < 65) {
            l.type = LOOT_HEALTH;
            l.amount = GetRandomValue(10, 25);
            l.color = RED;
        } else if (roll < 75) {
            l.type = LOOT_ARMOR;
            l.amount = GetRandomValue(10, 30);
            l.color = BLUE;
        } else if (roll < 88) {
            l.type = LOOT_XP;
            l.amount = GetRandomValue(10, 30);
            l.color = PURPLE;
        } else {
            l.type = LOOT_COIN;
            l.amount = GetRandomValue(15, 60);
            l.color = GOLD;
        }

        count++;
    }
}

void InitChest(FallingChest& chest, Vector2 pos, BuildingRef* buildings, int buildingCount) {
    pos = FindClearPosition(pos, buildings, buildingCount, 25);
    chest.active = true;
    chest.falling = true;
    chest.opened = false;
    chest.position = { pos.x, -100 };
    chest.targetPos = pos;
    chest.fallSpeed = 300;
    chest.openTimer = 0;
    chest.lifetime = 60;
    chest.lootCount = 0;
}

static void PlaceChestLoot(Loot& l, Vector2 chestPos, BuildingRef* buildings, int buildingCount) {
    float angle = (float)GetRandomValue(0, 628) / 100.0f;
    float dist = (float)GetRandomValue(20, 80);
    l.position = { chestPos.x + cosf(angle) * dist, chestPos.y + sinf(angle) * dist };
    l.position = FindClearPosition(l.position, buildings, buildingCount, 15);
    l.active = true;
    l.lifetime = 60.0f;
    l.bobOffset = (float)GetRandomValue(0, 628) / 100.0f;
    l.weapon = {};
    l.amount = 0;
}

void SpawnChestLoot(FallingChest& chest, int level, BuildingRef* buildings, int buildingCount) {
    chest.lootCount = 0;
    int potionHeal = 50 + (level - 1) * 5;

    if (GetRandomValue(0, 100) < 50 && chest.lootCount < 8) {
        Loot& l = chest.loots[chest.lootCount++];
        PlaceChestLoot(l, chest.position, buildings, buildingCount);
        l.type = LOOT_WEAPON;
        l.weapon = GenerateRandomWeapon(level);
        l.color = l.weapon.rarityColor;
    }

    int potions = GetRandomValue(1, 2);
    for (int i = 0; i < potions && chest.lootCount < 8; i++) {
        Loot& l = chest.loots[chest.lootCount++];
        PlaceChestLoot(l, chest.position, buildings, buildingCount);
        l.type = LOOT_HEALTH_POTION;
        l.amount = potionHeal;
        l.color = PINK;
    }

    int fillers = GetRandomValue(2, 4);
    for (int i = 0; i < fillers && chest.lootCount < 8; i++) {
        Loot& l = chest.loots[chest.lootCount++];
        PlaceChestLoot(l, chest.position, buildings, buildingCount);
        int roll = GetRandomValue(0, 100);
        if (roll < 45) {
            l.type = LOOT_AMMO;
            l.amount = GetRandomValue(30, 80);
            l.color = ORANGE;
        } else if (roll < 70) {
            l.type = LOOT_ARMOR;
            l.amount = GetRandomValue(20, 50);
            l.color = BLUE;
        } else {
            l.type = LOOT_COIN;
            l.amount = GetRandomValue(50, 150);
            l.color = GOLD;
        }
    }
}

void UpdateChest(FallingChest& chest, float dt, Vector2 playerPos, Loot* loots, int* lootCount, int level, BuildingRef* buildings, int buildingCount) {
    if (!chest.active) return;

    chest.lifetime -= dt;

    if (chest.falling) {
        chest.position.y += chest.fallSpeed * dt;
        if (chest.position.y >= chest.targetPos.y) {
            chest.position.y = chest.targetPos.y;
            chest.falling = false;
            chest.openTimer = 1.5f;
        }
    } else if (!chest.opened) {
        chest.openTimer -= dt;
        float dx = playerPos.x - chest.position.x;
        float dy = playerPos.y - chest.position.y;
        float dist = sqrtf(dx * dx + dy * dy);

        if (dist < 50 && IsKeyPressed(KEY_E)) {
            chest.opened = true;
            SpawnChestLoot(chest, level, buildings, buildingCount);

            for (int i = 0; i < chest.lootCount; i++) {
                if (*lootCount < MAX_LOOTS) {
                    loots[*lootCount] = chest.loots[i];
                    (*lootCount)++;
                }
            }
        }
    }

    if (chest.lifetime <= 0) {
        chest.active = false;
    }
}

void DrawChest(FallingChest& chest) {
    if (!chest.active) return;

    Color chestColor = GOLD;
    if (chest.opened) chestColor = DARKBROWN;
    else if (chest.falling) chestColor = YELLOW;

    DrawRectangleV({ chest.position.x - 20, chest.position.y - 15 }, { 40, 30 }, chestColor);
    DrawRectangleV({ chest.position.x - 22, chest.position.y - 17 }, { 44, 4 }, DARKBROWN);
    DrawRectangleV({ chest.position.x - 5, chest.position.y - 5 }, { 10, 10 }, DARKGRAY);

    if (!chest.opened && !chest.falling) {
        float pulse = 0.5f + 0.5f * sinf(GetTime() * 3.0f);
        DrawCircleV(chest.position, 30 + pulse * 10, ColorAlpha(GOLD, 0.15f));
    }

    if (chest.falling) {
        for (int i = 0; i < 5; i++) {
            float ox = (float)GetRandomValue(-15, 15);
            float oy = (float)GetRandomValue(-5, 5);
            DrawCircleV({ chest.position.x + ox, chest.position.y - 20 + oy }, 2, ColorAlpha(ORANGE, 0.6f));
        }
    }
}

void UpdateLoots(Loot* loots, int& count, float dt) {
    for (int i = count - 1; i >= 0; i--) {
        Loot& l = loots[i];
        l.lifetime -= dt;
        l.bobOffset += dt * 3.0f;

        if (l.lifetime <= 0 || !l.active) {
            loots[i] = loots[count - 1];
            count--;
        }
    }
}

void DrawLoots(Loot* loots, int count) {
    for (int i = 0; i < count; i++) {
        Loot& l = loots[i];
        if (!l.active) continue;

        float bob = sinf(l.bobOffset) * 3;
        Vector2 drawPos = { l.position.x, l.position.y + bob };

        if (l.type == LOOT_WEAPON) {
            DrawRectangleV({ drawPos.x - 10, drawPos.y - 6 }, { 20, 12 }, l.color);
            DrawRectangleV({ drawPos.x - 12, drawPos.y - 8 }, { 24, 4 }, l.weapon.rarityColor);

            const char* rarityLetter = GetRarityName(l.weapon.rarity);
            DrawText(TextSubtext(rarityLetter, 0, 1), drawPos.x - 3, drawPos.y - 4, 10, WHITE);
        } else {
            DrawCircleV(drawPos, 8, l.color);

            const char* icon = "?";
            switch (l.type) {
                case LOOT_AMMO: icon = "A"; break;
                case LOOT_HEALTH: icon = "+"; break;
                case LOOT_HEALTH_POTION: icon = "P"; break;
                case LOOT_ARMOR: icon = "S"; break;
                case LOOT_XP: icon = "X"; break;
                case LOOT_COIN: icon = "$"; break;
                case LOOT_CURE_SAMPLE: icon = "C"; break;
                default: break;
            }
            DrawText(icon, drawPos.x - 3, drawPos.y - 4, 10, WHITE);
        }

        if (l.lifetime < 5.0f && fmodf(l.lifetime, 0.5f) > 0.25f) {
            DrawCircleV(drawPos, 12, ColorAlpha(l.color, 0.3f));
        }
    }
}

void CollectLoot(Loot& loot, Weapon inventory[], bool slotUnlocked[], int& currentWeapon, int* health, int maxHealth, int* cureSamples) {
    loot.active = false;

    switch (loot.type) {
        case LOOT_AMMO:
            inventory[currentWeapon].ammo += loot.amount;
            break;
        case LOOT_HEALTH:
            *health = (*health + loot.amount) < maxHealth ? (*health + loot.amount) : maxHealth;
            break;
        case LOOT_HEALTH_POTION:
            *health = (*health + loot.amount) < maxHealth ? (*health + loot.amount) : maxHealth;
            break;
        case LOOT_ARMOR:
            break;
        case LOOT_WEAPON: {
            int slot = -1;
            for (int i = 0; i < MAX_WEAPON_SLOTS; i++) {
                if (!slotUnlocked[i]) {
                    slot = i;
                    break;
                }
            }
            if (slot < 0) slot = currentWeapon;
            inventory[slot] = loot.weapon;
            slotUnlocked[slot] = true;
            currentWeapon = slot;
            break;
        }
        case LOOT_XP:
            break;
        case LOOT_COIN:
            break;
        case LOOT_CURE_SAMPLE:
            if (cureSamples) *cureSamples += loot.amount;
            break;
    }
}
