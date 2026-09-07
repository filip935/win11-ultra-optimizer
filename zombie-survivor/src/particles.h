#pragma once
#include "raylib.h"

#define MAX_PARTICLES 500

struct Particle {
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    float maxLifetime;
    float size;
    Color color;
    bool active;
};

void SpawnParticle(Particle* particles, int& count, Vector2 pos, Color color, float speed, float life);
void SpawnBloodEffect(Particle* particles, int& count, Vector2 pos);
void SpawnExplosion(Particle* particles, int& count, Vector2 pos);
void SpawnMuzzleFlash(Particle* particles, int& count, Vector2 pos, float rotation);
void UpdateParticles(Particle* particles, int& count, float dt);
void DrawParticles(Particle* particles, int count);
