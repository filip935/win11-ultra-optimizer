#include "particles.h"
#include <cmath>

void SpawnParticle(Particle* particles, int& count, Vector2 pos, Color color, float speed, float life) {
    if (count >= MAX_PARTICLES) return;

    Particle& p = particles[count];
    p.position = pos;
    float angle = (float)GetRandomValue(0, 628) / 100.0f;
    float spd = (float)GetRandomValue(50, (int)(speed * 100)) / 100.0f;
    p.velocity = { cosf(angle) * spd, sinf(angle) * spd };
    p.lifetime = life;
    p.maxLifetime = life;
    p.size = (float)GetRandomValue(2, 6);
    p.color = color;
    p.active = true;
    count++;
}

void SpawnBloodEffect(Particle* particles, int& count, Vector2 pos) {
    for (int i = 0; i < 8; i++) {
        SpawnParticle(particles, count, pos, RED, 150, 0.5f);
    }
}

void SpawnExplosion(Particle* particles, int& count, Vector2 pos) {
    for (int i = 0; i < 20; i++) {
        SpawnParticle(particles, count, pos, ORANGE, 250, 0.8f);
        SpawnParticle(particles, count, pos, YELLOW, 200, 0.6f);
        SpawnParticle(particles, count, pos, RED, 180, 0.7f);
    }
}

void SpawnMuzzleFlash(Particle* particles, int& count, Vector2 pos, float rotation) {
    for (int i = 0; i < 3; i++) {
        float spread = ((float)GetRandomValue(-100, 100) / 100.0f) * 0.5f;
        float angle = rotation + spread;
        Particle& p = particles[count];
        p.position = pos;
        p.velocity = { cosf(angle) * 300, sinf(angle) * 300 };
        p.lifetime = 0.1f;
        p.maxLifetime = 0.1f;
        p.size = 3;
        p.color = YELLOW;
        p.active = true;
        count++;
    }
}

void UpdateParticles(Particle* particles, int& count, float dt) {
    for (int i = count - 1; i >= 0; i--) {
        Particle& p = particles[i];
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;
        p.velocity.x *= 0.95f;
        p.velocity.y *= 0.95f;
        p.lifetime -= dt;
        p.size *= 0.98f;

        if (p.lifetime <= 0) {
            p.active = false;
            particles[i] = particles[count - 1];
            count--;
        }
    }
}

void DrawParticles(Particle* particles, int count) {
    for (int i = 0; i < count; i++) {
        Particle& p = particles[i];
        float alpha = p.lifetime / p.maxLifetime;
        Color c = ColorAlpha(p.color, alpha);
        DrawCircleV(p.position, p.size, c);
    }
}
