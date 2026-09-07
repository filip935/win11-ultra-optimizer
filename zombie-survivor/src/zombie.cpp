#include "zombie.h"
#include "level.h"
#include "raymath.h"
#include <cmath>

void InitZombies() {
}

void SpawnZombie(Zombie* zombies, int& count, ZombieType type, Vector2 pos, int level) {
    if (count >= MAX_ZOMBIES) return;

    float mult = 1.0f + (level - 1) * 0.08f;

    Zombie& z = zombies[count];
    z.position = pos;
    z.velocity = { 0, 0 };
    z.rotation = 0;
    z.type = type;
    z.active = true;
    z.attackCooldown = 0;
    z.attackTimer = 0;
    z.hitFlash = 0;
    z.animTimer = 0;
    z.walkCycle = 0;
    z.armSwing = 0;

    switch (type) {
        case ZOMBIE_WALKER:
            z.health = (int)(30 * mult);
            z.maxHealth = z.health;
            z.damage = 10;
            z.speed = 60;
            z.xpValue = 10;
            z.scoreValue = 50;
            break;
        case ZOMBIE_RUNNER:
            z.health = (int)(20 * mult);
            z.maxHealth = z.health;
            z.damage = 8;
            z.speed = 150;
            z.xpValue = 15;
            z.scoreValue = 75;
            break;
        case ZOMBIE_BRUTE:
            z.health = (int)(100 * mult);
            z.maxHealth = z.health;
            z.damage = 25;
            z.speed = 40;
            z.xpValue = 30;
            z.scoreValue = 150;
            break;
        case ZOMBIE_SPITTER:
            z.health = (int)(25 * mult);
            z.maxHealth = z.health;
            z.damage = 5;
            z.speed = 50;
            z.xpValue = 20;
            z.scoreValue = 100;
            break;
        case ZOMBIE_BOSS:
            z.health = (int)(500 * mult);
            z.maxHealth = z.health;
            z.damage = 40;
            z.speed = 35;
            z.xpValue = 200;
            z.scoreValue = 1000;
            break;
    }

    count++;
}

static void DrawWalkerSprite(Vector2 pos, float rotation, float walkCycle, float armSwing, Color baseColor) {
    DrawCircleV(pos, 15, {80, 200, 80, 255});
}

static void DrawRunnerSprite(Vector2 pos, float rotation, float walkCycle, float armSwing, Color baseColor) {
    DrawCircleV(pos, 12, {200, 200, 50, 255});
}

static void DrawBruteSprite(Vector2 pos, float rotation, float walkCycle, float armSwing, Color baseColor) {
    DrawCircleV(pos, 25, {180, 120, 70, 255});
}

static void DrawSpitterSprite(Vector2 pos, float rotation, float walkCycle, float armSwing, Color baseColor) {
    DrawCircleV(pos, 14, {100, 230, 80, 255});
}

static void DrawBossSprite(Vector2 pos, float rotation, float walkCycle, float armSwing, Color baseColor) {
    DrawCircleV(pos, 35, {200, 50, 50, 255});
}

void UpdateZombies(Zombie* zombies, int count, Vector2 playerPos, float dt) {
    for (int i = 0; i < count; i++) {
        Zombie& z = zombies[i];
        if (!z.active) continue;

        z.animTimer += dt;

        Vector2 dir = { playerPos.x - z.position.x, playerPos.y - z.position.y };
        float len = sqrtf(dir.x * dir.x + dir.y * dir.y);

        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
        }

        z.rotation = atan2f(dir.y, dir.x);

        float attackDist = (z.type == ZOMBIE_BOSS) ? 50 : 30;
        bool isMoving = len > attackDist;

        if (isMoving) {
            z.position.x += dir.x * z.speed * dt;
            z.position.y += dir.y * z.speed * dt;
            z.walkCycle += dt * z.speed * 0.08f;
            z.armSwing += dt * z.speed * 0.1f;
        }

        z.position.x = Clamp(z.position.x, 10.0f, (float)(ARENA_WIDTH - 10));
        z.position.y = Clamp(z.position.y, 10.0f, (float)(ARENA_HEIGHT - 10));

        if (z.attackTimer > 0) z.attackTimer -= dt;
        if (z.hitFlash > 0) z.hitFlash -= dt;
    }
}

void DrawZombies(Zombie* zombies, int count) {
    for (int i = 0; i < count; i++) {
        Zombie& z = zombies[i];
        if (!z.active) continue;

        Color drawColor = WHITE;
        if (z.hitFlash > 0) drawColor = RED;

        switch (z.type) {
            case ZOMBIE_WALKER:
                DrawWalkerSprite(z.position, z.rotation, z.walkCycle, z.armSwing, drawColor);
                break;
            case ZOMBIE_RUNNER:
                DrawRunnerSprite(z.position, z.rotation, z.walkCycle, z.armSwing, drawColor);
                break;
            case ZOMBIE_BRUTE:
                DrawBruteSprite(z.position, z.rotation, z.walkCycle, z.armSwing, drawColor);
                break;
            case ZOMBIE_SPITTER:
                DrawSpitterSprite(z.position, z.rotation, z.walkCycle, z.armSwing, drawColor);
                break;
            case ZOMBIE_BOSS:
                DrawBossSprite(z.position, z.rotation, z.walkCycle, z.armSwing, drawColor);
                break;
        }

        float barWidth = (z.type == ZOMBIE_BOSS) ? 50 : (z.type == ZOMBIE_BRUTE) ? 40 : 24;
        float barY = z.position.y - (z.type == ZOMBIE_BOSS ? 40 : z.type == ZOMBIE_BRUTE ? 30 : 22);
        float healthPct = (float)z.health / z.maxHealth;

        DrawRectangleV({ z.position.x - barWidth / 2, barY }, { barWidth, 3 }, RED);
        DrawRectangleV({ z.position.x - barWidth / 2, barY }, { barWidth * healthPct, 3 }, GREEN);

        if (z.type == ZOMBIE_BOSS) {
            float pulse = 0.3f + 0.2f * sinf(GetTime() * 4.0f);
            DrawCircleV(z.position, 30, ColorAlpha(RED, pulse));
        }
    }
}

void DamageZombie(Zombie& z, int damage) {
    z.health -= damage;
    z.hitFlash = 0.1f;
    if (z.health <= 0) {
        z.active = false;
    }
}
