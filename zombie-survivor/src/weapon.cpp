#include "weapon.h"
#include "level.h"
#include <cmath>

static const char* rarityNames[] = { "Common", "Uncommon", "Rare", "Epic", "Legendary" };
static Color rarityColors[] = { WHITE, GREEN, BLUE, PURPLE, GOLD };

void InitWeapons(Weapon weapons[]) {
    weapons[PISTOL] = { PISTOL, "Pistol", RARITY_COMMON, 15, 0.3f, 500, 1, 0, 600, 999, 999, WHITE };
    weapons[SHOTGUN] = { SHOTGUN, "Shotgun", RARITY_COMMON, 8, 0.8f, 300, 5, 0.3f, 500, 50, 50, WHITE };
    weapons[SMG] = { SMG, "SMG", RARITY_COMMON, 8, 0.08f, 400, 1, 0.1f, 700, 120, 120, WHITE };
    weapons[RIFLE] = { RIFLE, "Assault Rifle", RARITY_UNCOMMON, 12, 0.12f, 600, 1, 0.05f, 800, 150, 150, GREEN };
    weapons[SNIPER] = { SNIPER, "Sniper Rifle", RARITY_RARE, 75, 1.5f, 1200, 1, 0, 1200, 20, 20, BLUE };
    weapons[DUAL_PISTOLS] = { DUAL_PISTOLS, "Dual Pistols", RARITY_UNCOMMON, 12, 0.15f, 450, 2, 0.15f, 650, 160, 160, GREEN };
    weapons[PLASMA_RIFLE] = { PLASMA_RIFLE, "Plasma Rifle", RARITY_RARE, 25, 0.4f, 800, 1, 0, 900, 80, 80, BLUE };
    weapons[FLAMETHROWER] = { FLAMETHROWER, "Flamethrower", RARITY_EPIC, 6, 0.04f, 250, 3, 0.4f, 400, 200, 200, PURPLE };
    weapons[ROCKET_LAUNCHER] = { ROCKET_LAUNCHER, "Rocket Launcher", RARITY_EPIC, 120, 1.2f, 900, 1, 0, 400, 15, 15, PURPLE };
    weapons[CHAIN_LIGHTNING] = { CHAIN_LIGHTNING, "Chain Lightning", RARITY_LEGENDARY, 40, 0.5f, 700, 5, 0.2f, 1000, 60, 60, GOLD };
}

const char* GetRarityName(Rarity r) {
    if (r >= 0 && r < RARITY_COUNT) return rarityNames[r];
    return "Unknown";
}

Color GetRarityColor(Rarity r) {
    if (r >= 0 && r < RARITY_COUNT) return rarityColors[r];
    return WHITE;
}

Weapon GenerateRandomWeapon(int level) {
    Weapon baseWeapons[] = {
        { PISTOL, "Pistol", RARITY_COMMON, 15, 0.3f, 500, 1, 0, 600, 30, 30, WHITE },
        { SHOTGUN, "Shotgun", RARITY_COMMON, 8, 0.8f, 300, 5, 0.3f, 500, 20, 20, WHITE },
        { SMG, "SMG", RARITY_COMMON, 8, 0.08f, 400, 1, 0.1f, 700, 60, 60, WHITE },
        { RIFLE, "Assault Rifle", RARITY_UNCOMMON, 12, 0.12f, 600, 1, 0.05f, 800, 80, 80, GREEN },
        { SNIPER, "Sniper Rifle", RARITY_RARE, 75, 1.5f, 1200, 1, 0, 1200, 10, 10, BLUE },
        { DUAL_PISTOLS, "Dual Pistols", RARITY_UNCOMMON, 12, 0.15f, 450, 2, 0.15f, 650, 80, 80, GREEN },
        { PLASMA_RIFLE, "Plasma Rifle", RARITY_RARE, 25, 0.4f, 800, 1, 0, 900, 40, 40, BLUE },
        { FLAMETHROWER, "Flamethrower", RARITY_EPIC, 6, 0.04f, 250, 3, 0.4f, 400, 100, 100, PURPLE },
        { ROCKET_LAUNCHER, "Rocket Launcher", RARITY_EPIC, 120, 1.2f, 900, 1, 0, 400, 8, 8, PURPLE },
        { CHAIN_LIGHTNING, "Chain Lightning", RARITY_LEGENDARY, 40, 0.5f, 700, 5, 0.2f, 1000, 30, 30, GOLD },
    };

    int idx = GetRandomValue(0, WEAPON_COUNT - 1);
    Weapon w = baseWeapons[idx];

    float legendaryCh = (level >= 50) ? 2.5f : 1.5f;
    float epicCh = (level >= 30) ? 10.0f : 7.0f;
    float roll100 = (float)GetRandomValue(0, 1000) / 10.0f;

    Rarity forcedRarity = RARITY_COMMON;
    if (roll100 < legendaryCh) forcedRarity = RARITY_LEGENDARY;
    else if (roll100 < epicCh) forcedRarity = RARITY_EPIC;
    else if (roll100 < 25.0f) forcedRarity = RARITY_RARE;
    else if (roll100 < 55.0f) forcedRarity = RARITY_UNCOMMON;

    int pool[3] = { 0, 0, 0 };
    int poolSize = 0;
    switch (forcedRarity) {
        case RARITY_COMMON: pool[0] = 0; pool[1] = 1; pool[2] = 2; poolSize = 3; break;
        case RARITY_UNCOMMON: pool[0] = 3; pool[1] = 5; poolSize = 2; break;
        case RARITY_RARE: pool[0] = 4; pool[1] = 6; poolSize = 2; break;
        case RARITY_EPIC: pool[0] = 7; pool[1] = 8; poolSize = 2; break;
        case RARITY_LEGENDARY: pool[0] = 9; poolSize = 1; break;
    }
    w = baseWeapons[pool[GetRandomValue(0, poolSize - 1)]];
    w.rarity = forcedRarity;

    float mult = 1.0f + (int)w.rarity * 0.4f;
    w.damage = (int)(w.damage * mult);
    w.fireRate *= (1.0f - (int)w.rarity * 0.08f);
    w.projectileSpeed *= (1.0f + (int)w.rarity * 0.1f);
    w.maxAmmo = (int)(w.maxAmmo * (1.0f + (int)w.rarity * 0.3f));
    w.ammo = w.maxAmmo;

    if (w.rarity == RARITY_LEGENDARY) {
        w.name = TextFormat("Legendary %s", w.name);
    } else if (w.rarity == RARITY_EPIC) {
        w.name = TextFormat("Epic %s", w.name);
    } else if (w.rarity == RARITY_RARE) {
        w.name = TextFormat("Rare %s", w.name);
    } else if (w.rarity == RARITY_UNCOMMON) {
        w.name = TextFormat("Uncommon %s", w.name);
    }

    w.rarityColor = GetRarityColor(w.rarity);
    return w;
}

void Shoot(Weapon& w, Vector2 pos, float rotation, Projectile* projectiles, int& count) {
    for (int i = 0; i < w.projectilesPerShot && count < MAX_PROJECTILES; i++) {
        float spread = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * w.spread;
        float angle = rotation + spread;

        Projectile& p = projectiles[count];
        p.position = pos;
        p.velocity = { cosf(angle) * w.projectileSpeed, sinf(angle) * w.projectileSpeed };
        p.lifetime = w.range / w.projectileSpeed;
        p.damage = w.damage;
        p.active = true;
        p.piercing = false;
        p.aoeRadius = 0;

        if (w.type == PLASMA_RIFLE) { p.color = PURPLE; }
        else if (w.type == CHAIN_LIGHTNING) { p.color = SKYBLUE; p.piercing = true; }
        else if (w.type == ROCKET_LAUNCHER) { p.color = ORANGE; p.aoeRadius = 60; }
        else if (w.type == FLAMETHROWER) { p.color = RED; p.lifetime *= 0.5f; }
        else { p.color = w.rarityColor; }

        count++;
    }
}

void UpdateProjectiles(Projectile* projectiles, int& count, float dt) {
    for (int i = count - 1; i >= 0; i--) {
        Projectile& p = projectiles[i];
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;
        p.lifetime -= dt;

        if (p.lifetime <= 0 || p.position.x < 0 || p.position.x > ARENA_WIDTH ||
            p.position.y < 0 || p.position.y > ARENA_HEIGHT) {
            p.active = false;
        }

        if (!p.active) {
            projectiles[i] = projectiles[count - 1];
            count--;
        }
    }
}

void DrawProjectiles(Projectile* projectiles, int count) {
    for (int i = 0; i < count; i++) {
        Projectile& p = projectiles[i];
        if (p.aoeRadius > 0) {
            DrawCircleV(p.position, 5, p.color);
            DrawCircleV(p.position, 3, YELLOW);
        } else {
            DrawCircleV(p.position, 3, p.color);
        }
    }
}
