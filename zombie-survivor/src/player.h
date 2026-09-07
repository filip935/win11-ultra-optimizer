#pragma once
#include "raylib.h"
#include "weapon.h"

struct Player {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    int health;
    int maxHealth;
    int armor;
    float speed;
    float shootCooldown;
    float shootTimer;
    int score;
    int xp;
    int level;
    bool alive;
    float invincibleTimer;
    float dashCooldown;
    bool isDashing;
    float dashTimer;
    Vector2 dashDir;
    float animTimer;
    bool isMoving;
    float walkCycle;
    float muzzleFlash;
};

void InitPlayer(Player& p);
void UpdatePlayer(Player& p, float dt);
void DrawPlayer(Player& p);
void PlayerShoot(Player& p, Vector2 target, Weapon& w, Projectile* projectiles, int& count);
void PlayerDash(Player& p);
void DamagePlayer(Player& p, int damage);
