#pragma once
#include "raylib.h"

#define MAX_ZOMBIES 200

enum ZombieType {
    ZOMBIE_WALKER,
    ZOMBIE_RUNNER,
    ZOMBIE_BRUTE,
    ZOMBIE_SPITTER,
    ZOMBIE_BOSS
};

struct Zombie {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    int health;
    int maxHealth;
    int damage;
    float speed;
    ZombieType type;
    bool active;
    float attackCooldown;
    float attackTimer;
    float hitFlash;
    int xpValue;
    int scoreValue;
    float animTimer;
    float walkCycle;
    float armSwing;
};

void InitZombies();
void SpawnZombie(Zombie* zombies, int& count, ZombieType type, Vector2 pos, int level);
void UpdateZombies(Zombie* zombies, int count, Vector2 playerPos, float dt);
void DrawZombies(Zombie* zombies, int count);
void DamageZombie(Zombie& z, int damage);
