#include "player.h"
#include "zombie.h"
#include "weapon.h"
#include "level.h"
#include "particles.h"
#include "raymath.h"
#include <cmath>

void InitPlayer(Player& p) {
    p.position = { ARENA_WIDTH / 2.0f, ARENA_HEIGHT / 2.0f };
    p.velocity = { 0, 0 };
    p.rotation = 0;
    p.health = 100;
    p.maxHealth = 100;
    p.armor = 0;
    p.speed = 200;
    p.shootCooldown = 0;
    p.shootTimer = 0;
    p.score = 0;
    p.xp = 0;
    p.level = 1;
    p.alive = true;
    p.invincibleTimer = 0;
    p.dashCooldown = 0;
    p.isDashing = false;
    p.dashTimer = 0;
    p.dashDir = { 0, 0 };
    p.animTimer = 0;
    p.isMoving = false;
    p.walkCycle = 0;
    p.muzzleFlash = 0;
}

void UpdatePlayer(Player& p, float dt) {
    if (!p.alive) return;

    Vector2 moveDir = { 0, 0 };
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) moveDir.y -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) moveDir.y += 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) moveDir.x -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveDir.x += 1;

    p.isMoving = (moveDir.x != 0 || moveDir.y != 0);

    if (p.isMoving) {
        float len = sqrtf(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
        moveDir.x /= len;
        moveDir.y /= len;
        p.walkCycle += dt * 10.0f;
    }

    if (p.isDashing) {
        p.dashTimer -= dt;
        p.position.x += p.dashDir.x * 600 * dt;
        p.position.y += p.dashDir.y * 600 * dt;
        if (p.dashTimer <= 0) {
            p.isDashing = false;
        }
    } else {
        p.velocity.x = moveDir.x * p.speed;
        p.velocity.y = moveDir.y * p.speed;
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;
    }

    p.position.x = Clamp(p.position.x, 20.0f, (float)(ARENA_WIDTH - 20));
    p.position.y = Clamp(p.position.y, 20.0f, (float)(ARENA_HEIGHT - 20));

    Vector2 mousePos = GetMousePosition();
    p.rotation = atan2f(mousePos.y - GetScreenHeight() / 2.0f, mousePos.x - GetScreenWidth() / 2.0f);

    if (p.invincibleTimer > 0) p.invincibleTimer -= dt;
    if (p.shootTimer > 0) p.shootTimer -= dt;
    if (p.dashCooldown > 0) p.dashCooldown -= dt;
    if (p.muzzleFlash > 0) p.muzzleFlash -= dt;

    if (IsKeyDown(KEY_SPACE) && p.dashCooldown <= 0 && !p.isDashing) {
        PlayerDash(p);
    }

    int xpNeeded = p.level * 100;
    if (p.xp >= xpNeeded) {
        p.xp -= xpNeeded;
        p.level++;
        p.maxHealth += 10;
        p.health = p.maxHealth;
        p.speed += 5;
    }
}

void DrawPlayer(Player& p) {
    if (!p.alive) return;

    Color bodyColor = (p.invincibleTimer > 0 && fmodf(p.invincibleTimer, 0.1f) > 0.05f) ? YELLOW : WHITE;
    Color gunColor = {200, 200, 215, 255};

    Vector2 pos = p.position;

    DrawCircleV(pos, 12, bodyColor);

    float gunLen = 18;
    float cosR = cosf(p.rotation);
    float sinR = sinf(p.rotation);
    float gunTipX = pos.x + cosR * gunLen;
    float gunTipY = pos.y + sinR * gunLen;
    DrawLineEx(pos, { gunTipX, gunTipY }, 3, gunColor);

    if (p.muzzleFlash > 0) {
        DrawCircleV({ gunTipX + cosR * 4, gunTipY + sinR * 4 }, 6, YELLOW);
        DrawCircleV({ gunTipX + cosR * 4, gunTipY + sinR * 4 }, 3, WHITE);
    }

    if (p.isDashing) {
        for (int i = 1; i <= 3; i++) {
            float alpha = 0.5f - i * 0.12f;
            DrawCircleV({ pos.x - cosR * i * 14, pos.y - sinR * i * 14 }, 10 - i * 2, ColorAlpha(SKYBLUE, alpha));
        }
    }
}

void PlayerShoot(Player& p, Vector2 target, Weapon& w, Projectile* projectiles, int& count) {
    if (p.shootTimer > 0 || !p.alive) return;
    if (w.ammo <= 0) return;

    Shoot(w, p.position, p.rotation, projectiles, count);
    p.shootTimer = w.fireRate;
    w.ammo--;
    p.muzzleFlash = 0.08f;
}

void PlayerDash(Player& p) {
    p.isDashing = true;
    p.dashTimer = 0.15f;
    p.dashCooldown = 1.0f;
    p.invincibleTimer = 0.15f;

    Vector2 moveDir = { 0, 0 };
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) moveDir.y -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) moveDir.y += 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) moveDir.x -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveDir.x += 1;

    if (moveDir.x == 0 && moveDir.y == 0) {
        moveDir.x = cosf(p.rotation);
        moveDir.y = sinf(p.rotation);
    } else {
        float len = sqrtf(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
        moveDir.x /= len;
        moveDir.y /= len;
    }
    p.dashDir = moveDir;
}

void DamagePlayer(Player& p, int damage) {
    if (p.invincibleTimer > 0) return;

    int actualDamage = damage;
    if (p.armor > 0) {
        int absorbed = p.armor > damage / 2 ? damage / 2 : p.armor;
        p.armor -= absorbed;
        actualDamage -= absorbed;
    }

    p.health -= actualDamage;
    p.invincibleTimer = 0.5f;

    if (p.health <= 0) {
        p.health = 0;
        p.alive = false;
    }
}
