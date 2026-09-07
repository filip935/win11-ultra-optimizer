#pragma once
#include "raylib.h"

#define MAX_PROJECTILES 500
#define WEAPON_COUNT 10
#define MAX_WEAPON_SLOTS 5
#define RARITY_COUNT 5

enum Rarity {
    RARITY_COMMON,
    RARITY_UNCOMMON,
    RARITY_RARE,
    RARITY_EPIC,
    RARITY_LEGENDARY
};

enum WeaponType {
    PISTOL,
    SHOTGUN,
    SMG,
    RIFLE,
    SNIPER,
    DUAL_PISTOLS,
    PLASMA_RIFLE,
    FLAMETHROWER,
    ROCKET_LAUNCHER,
    CHAIN_LIGHTNING
};

struct Weapon {
    WeaponType type;
    const char* name;
    Rarity rarity;
    int damage;
    float fireRate;
    float range;
    int projectilesPerShot;
    float spread;
    float projectileSpeed;
    int ammo;
    int maxAmmo;
    Color rarityColor;
};

struct Projectile {
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    int damage;
    bool active;
    Color color;
    bool piercing;
    float aoeRadius;
};

void InitWeapons(Weapon weapons[]);
const char* GetRarityName(Rarity r);
Color GetRarityColor(Rarity r);
Weapon GenerateRandomWeapon(int level);
void Shoot(Weapon& w, Vector2 pos, float rotation, Projectile* projectiles, int& count);
void UpdateProjectiles(Projectile* projectiles, int& count, float dt);
void DrawProjectiles(Projectile* projectiles, int count);
