#include "game.h"
#include "raymath.h"
#include <cmath>
#include <stdio.h>

void GenerateCity(Game& game) {
    game.buildingCount = 0;

    for (int i = 0; i < 50 && game.buildingCount < MAX_BUILDINGS; i++) {
        int w = GetRandomValue(60, 160);
        int h = GetRandomValue(60, 160);
        int x = GetRandomValue(50, ARENA_WIDTH - 200);
        int y = GetRandomValue(50, ARENA_HEIGHT - 200);

        Rectangle newRect = { (float)x, (float)y, (float)w, (float)h };

        bool overlap = false;
        for (int j = 0; j < game.buildingCount; j++) {
            if (CheckCollisionRecs(newRect, game.buildings[j].rect)) {
                overlap = true;
                break;
            }
        }

        float dx = x + w / 2 - game.player.position.x;
        float dy = y + h / 2 - game.player.position.y;
        if (sqrtf(dx * dx + dy * dy) < 150) overlap = true;

        if (!overlap) {
            game.buildings[game.buildingCount].rect = newRect;

            Color c;
            int colorRoll = GetRandomValue(0, 4);
            switch (colorRoll) {
                case 0: c = {60, 60, 70, 255}; break;
                case 1: c = {50, 55, 65, 255}; break;
                case 2: c = {70, 65, 60, 255}; break;
                case 3: c = {55, 60, 55, 255}; break;
                default: c = {65, 60, 70, 255}; break;
            }
            game.buildings[game.buildingCount].color = c;

            game.buildingCount++;
        }
    }
}

bool CheckBuildingCollision(Game& game, Vector2 pos, float radius) {
    for (int i = 0; i < game.buildingCount; i++) {
        Rectangle r = game.buildings[i].rect;
        float closestX = fmaxf(r.x, fminf(pos.x, r.x + r.width));
        float closestY = fmaxf(r.y, fminf(pos.y, r.y + r.height));
        float dx = pos.x - closestX;
        float dy = pos.y - closestY;
        if (dx * dx + dy * dy < radius * radius) {
            return true;
        }
    }
    return false;
}

void InitGame(Game& game) {
    game.state = MENU;
    game.currentLevel = 1;
    game.playTime = 0;
    game.camera = { 0 };
    game.camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    game.camera.target = { ARENA_WIDTH / 2.0f, ARENA_HEIGHT / 2.0f };
    game.camera.rotation = 0;
    game.camera.zoom = 1.0f;
    game.zombieCount = 0;
    game.projectileCount = 0;
    game.lootCount = 0;
    game.particleCount = 0;
    game.levelTimer = 0;
    game.zombiesKilledThisLevel = 0;
    game.screenShake = false;
    game.shakeIntensity = 0;
    game.chestSpawnedThisLevel = false;
    game.chest = {};
    game.cureSamples = 0;
    game.choiceSelection = 0;
    game.endingTimer = 0;

    InitPlayer(game.player);
    InitWeapons(game.weapons);
    game.inventory[0] = game.weapons[PISTOL];
    game.slotUnlocked[0] = true;
    for (int i = 1; i < MAX_WEAPON_SLOTS; i++) {
        game.slotUnlocked[i] = false;
        game.inventory[i] = {};
    }
    game.currentWeapon = 0;
    InitLevels(game.levels);
    InitStory(game.story);
    InitUI(game.ui);
    InitZombies();
    GenerateCity(game);
}

void UpdateGame(Game& game, float dt) {
    if (game.state == MENU) {
        if (IsKeyPressed(KEY_ENTER)) {
            game.state = STORY;
            TriggerIntro(game.story);
        }
        return;
    }

    if (game.state == STORY) {
        UpdateStory(game.story, dt);
        if (!game.story.active) {
            game.state = PLAYING;
            GetLevelDescription(game.currentLevel, game.ui.message, sizeof(game.ui.message));
            game.ui.messageTimer = 3.0f;
        }
        return;
    }

    if (game.state == GAME_OVER) {
        if (IsKeyPressed(KEY_ENTER)) {
            InitGame(game);
        }
        return;
    }

    if (game.state == VICTORY) {
        if (IsKeyPressed(KEY_ENTER)) {
            game.state = CURE_CHOICE;
            game.choiceSelection = 0;
        }
        return;
    }

    if (game.state == CURE_CHOICE) {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            game.choiceSelection = 0;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            game.choiceSelection = 1;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            if (game.choiceSelection == 0) {
                game.state = ENDING_CURE;
                game.endingTimer = 0;
            } else {
                game.state = ENDING_KILL;
                game.endingTimer = 0;
            }
        }
        return;
    }

    if (game.state == ENDING_CURE || game.state == ENDING_KILL) {
        game.endingTimer += dt;
        if (IsKeyPressed(KEY_ENTER) && game.endingTimer > 2.0f) {
            InitGame(game);
        }
        return;
    }

    if (game.state == LEVEL_COMPLETE) {
        if (IsKeyPressed(KEY_ENTER)) {
            game.currentLevel++;
            if (game.currentLevel > MAX_LEVEL) {
                game.state = VICTORY;
                TriggerStory(game.story, 101);
                return;
            }
            game.zombiesKilledThisLevel = 0;
            game.zombieCount = 0;
            game.lootCount = 0;
            game.projectileCount = 0;
            game.levelTimer = 0;
            game.chestSpawnedThisLevel = false;
            game.chest = {};

            if (game.currentLevel % 10 == 0) {
                GenerateCity(game);
            }

            GetLevelDescription(game.currentLevel, game.ui.message, sizeof(game.ui.message));
            game.ui.messageTimer = 3.0f;
            game.state = PLAYING;
            TriggerStory(game.story, game.currentLevel);
        }
        return;
    }

    if (game.state == PAUSED) {
        if (IsKeyPressed(KEY_ESCAPE)) game.state = PLAYING;
        if (IsKeyPressed(KEY_Q)) InitGame(game);
        return;
    }

    if (game.state == PLAYING) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            game.state = PAUSED;
            return;
        }

        game.playTime += dt;
        game.levelTimer += dt;

        for (int i = 0; i < MAX_WEAPON_SLOTS; i++) {
            if (IsKeyPressed(KEY_ONE + i)) {
                if (game.slotUnlocked[i]) {
                    game.currentWeapon = i;
                }
            }
        }

        Vector2 oldPos = game.player.position;
        UpdatePlayer(game.player, dt);

        if (CheckBuildingCollision(game, game.player.position, 14)) {
            game.player.position = oldPos;
        }

        game.camera.target = game.player.position;

        Level& level = game.levels[game.currentLevel - 1];

        if (!game.chestSpawnedThisLevel && game.levelTimer > 15.0f) {
            float angle = (float)GetRandomValue(0, 628) / 100.0f;
            float dist = (float)GetRandomValue(200, 500);
            Vector2 chestPos = {
                game.player.position.x + cosf(angle) * dist,
                game.player.position.y + sinf(angle) * dist
            };
            chestPos.x = Clamp(chestPos.x, 100.0f, (float)(ARENA_WIDTH - 100));
            chestPos.y = Clamp(chestPos.y, 100.0f, (float)(ARENA_HEIGHT - 100));

            InitChest(game.chest, chestPos, (BuildingRef*)game.buildings, game.buildingCount);
            game.chestSpawnedThisLevel = true;
            ShowMessage(game.ui, "A supply drop is incoming!");
        }

        UpdateChest(game.chest, dt, game.player.position, game.loots, &game.lootCount, game.currentLevel, (BuildingRef*)game.buildings, game.buildingCount);

        if (level.zombiesSpawned < level.zombiesRequired) {
            UpdateLevel(level, dt);
            if (level.spawnTimer <= 0) {
                level.spawnTimer = level.spawnRate;

                float angle = (float)GetRandomValue(0, 628) / 100.0f;
                float dist = 250 + (float)GetRandomValue(0, 100);
                Vector2 spawnPos = {
                    game.player.position.x + cosf(angle) * dist,
                    game.player.position.y + sinf(angle) * dist
                };
                spawnPos.x = Clamp(spawnPos.x, 20.0f, (float)(ARENA_WIDTH - 20));
                spawnPos.y = Clamp(spawnPos.y, 20.0f, (float)(ARENA_HEIGHT - 20));

                int attempts = 0;
                while (CheckBuildingCollision(game, spawnPos, 15) && attempts < 10) {
                    spawnPos.x += 30;
                    spawnPos.y += 30;
                    spawnPos.x = Clamp(spawnPos.x, 20.0f, (float)(ARENA_WIDTH - 20));
                    spawnPos.y = Clamp(spawnPos.y, 20.0f, (float)(ARENA_HEIGHT - 20));
                    attempts++;
                }

                ZombieType type = level.primaryZombie;
                if (!level.bossLevel && GetRandomValue(0, 100) < 15) {
                    type = (ZombieType)GetRandomValue(0, 3);
                }

                SpawnZombie(game.zombies, game.zombieCount, type, spawnPos, game.currentLevel);
                level.zombiesSpawned++;
            }
        }

        for (int i = 0; i < game.zombieCount; i++) {
            Zombie& z = game.zombies[i];
            if (!z.active) continue;

            Vector2 oldZPos = z.position;
            Vector2 dir = { game.player.position.x - z.position.x, game.player.position.y - z.position.y };
            float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (len > 0) { dir.x /= len; dir.y /= len; }

            float attackDist = (z.type == ZOMBIE_BOSS) ? 50 : 30;
            if (len > attackDist) {
                z.position.x += dir.x * z.speed * dt;
                z.position.y += dir.y * z.speed * dt;
            }

            if (CheckBuildingCollision(game, z.position, 12)) {
                z.position = oldZPos;
            }

            z.position.x = Clamp(z.position.x, 10.0f, (float)(ARENA_WIDTH - 10));
            z.position.y = Clamp(z.position.y, 10.0f, (float)(ARENA_HEIGHT - 10));
            z.rotation = atan2f(dir.y, dir.x);

            if (z.attackTimer > 0) z.attackTimer -= dt;
            if (z.hitFlash > 0) z.hitFlash -= dt;
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mouseWorld = GetMousePosition();
            PlayerShoot(game.player, mouseWorld, game.inventory[game.currentWeapon], game.projectiles, game.projectileCount);
        }

        UpdateProjectiles(game.projectiles, game.projectileCount, dt);

        for (int i = 0; i < game.projectileCount; i++) {
            Projectile& proj = game.projectiles[i];
            if (!proj.active) continue;

            if (CheckBuildingCollision(game, proj.position, 2)) {
                proj.active = false;
                continue;
            }

            for (int j = 0; j < game.zombieCount; j++) {
                Zombie& z = game.zombies[j];
                if (!z.active) continue;

                float dx = proj.position.x - z.position.x;
                float dy = proj.position.y - z.position.y;
                float dist = sqrtf(dx * dx + dy * dy);

                float hitRadius = (z.type == ZOMBIE_BOSS) ? 24 : (z.type == ZOMBIE_BRUTE) ? 18 : 12;
                if (proj.aoeRadius > 0) {
                    if (dist < hitRadius + proj.aoeRadius) {
                        DamageZombie(z, proj.damage);
                        SpawnExplosion(game.particles, game.particleCount, proj.position);
                        game.shakeIntensity = 6;
                        game.screenShake = true;

                        for (int k = 0; k < game.zombieCount; k++) {
                            if (k == j || !game.zombies[k].active) continue;
                            float adx = game.zombies[k].position.x - proj.position.x;
                            float ady = game.zombies[k].position.y - proj.position.y;
                            float adist = sqrtf(adx * adx + ady * ady);
                            if (adist < proj.aoeRadius) {
                                DamageZombie(game.zombies[k], proj.damage / 2);
                                if (!game.zombies[k].active) {
                                    game.player.score += game.zombies[k].scoreValue;
                                    game.player.xp += game.zombies[k].xpValue;
                                    game.zombiesKilledThisLevel++;
                                    SpawnLoot(game.loots, game.lootCount, game.zombies[k].position, game.zombies[k].type, game.currentLevel, (BuildingRef*)game.buildings, game.buildingCount);
                                }
                            }
                        }

                        if (!z.active) {
                            game.player.score += z.scoreValue;
                            game.player.xp += z.xpValue;
                            game.zombiesKilledThisLevel++;
                            SpawnLoot(game.loots, game.lootCount, z.position, z.type, game.currentLevel, (BuildingRef*)game.buildings, game.buildingCount);
                        }

                        if (!proj.piercing) {
                            proj.active = false;
                        }
                        break;
                    }
                } else {
                    if (dist < hitRadius + 3) {
                        DamageZombie(z, proj.damage);
                        proj.active = false;

                        SpawnBloodEffect(game.particles, game.particleCount, proj.position);

                        if (!z.active) {
                            game.player.score += z.scoreValue;
                            game.player.xp += z.xpValue;
                            game.zombiesKilledThisLevel++;
                            SpawnLoot(game.loots, game.lootCount, z.position, z.type, game.currentLevel, (BuildingRef*)game.buildings, game.buildingCount);

                            if (z.type == ZOMBIE_BOSS) {
                                SpawnExplosion(game.particles, game.particleCount, z.position);
                                game.shakeIntensity = 10;
                                game.screenShake = true;
                            }

                            game.shakeIntensity = game.shakeIntensity > 3 ? game.shakeIntensity : 3;
                            game.screenShake = true;
                        }

                        break;
                    }
                }
            }
        }

        for (int i = 0; i < game.zombieCount; i++) {
            Zombie& z = game.zombies[i];
            if (!z.active) continue;

            float dx = game.player.position.x - z.position.x;
            float dy = game.player.position.y - z.position.y;
            float dist = sqrtf(dx * dx + dy * dy);

            float attackDist = (z.type == ZOMBIE_BOSS) ? 50 : 30;
            if (dist < attackDist && z.attackTimer <= 0) {
                DamagePlayer(game.player, z.damage);
                z.attackTimer = 1.0f;

                SpawnBloodEffect(game.particles, game.particleCount, game.player.position);
                game.shakeIntensity = 5;
                game.screenShake = true;
            }
        }

        UpdateLoots(game.loots, game.lootCount, dt);

        for (int i = 0; i < game.lootCount; i++) {
            Loot& l = game.loots[i];
            if (!l.active) continue;

            float dx = game.player.position.x - l.position.x;
            float dy = game.player.position.y - l.position.y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist < 30) {
                CollectLoot(l, game.inventory, game.slotUnlocked, game.currentWeapon, &game.player.health, game.player.maxHealth, &game.cureSamples);

                if (l.type == LOOT_XP) {
                    game.player.xp += l.amount;
                } else if (l.type == LOOT_COIN) {
                    game.player.score += l.amount;
                } else if (l.type == LOOT_WEAPON) {
                    ShowMessage(game.ui, TextFormat("Slot %d: %s!", game.currentWeapon + 1, game.inventory[game.currentWeapon].name));
                } else if (l.type == LOOT_CURE_SAMPLE) {
                    ShowMessage(game.ui, TextFormat("Cure Sample! (%d/%d)", game.cureSamples, REQUIRED_SAMPLES));
                }
            }
        }

        UpdateParticles(game.particles, game.particleCount, dt);

        if (game.ui.messageTimer > 0) game.ui.messageTimer -= dt;
        if (game.shakeIntensity > 0) {
            game.shakeIntensity *= 0.9f;
            if (game.shakeIntensity < 0.1f) {
                game.screenShake = false;
                game.shakeIntensity = 0;
            }
        }

        if (!game.player.alive) {
            game.state = GAME_OVER;
            return;
        }

        if (game.zombiesKilledThisLevel >= level.zombiesRequired) {
            level.completed = true;
            game.state = LEVEL_COMPLETE;
        }
    }
}

void DrawCityEnvironment(Game& game) {
    Color arenaColor = {25, 25, 30, 255};
    DrawRectangle(0, 0, ARENA_WIDTH, ARENA_HEIGHT, arenaColor);

    Color gridLine = {40, 40, 45};
    Color roadLine = {50, 50, 55};
    for (int x = 0; x < ARENA_WIDTH; x += 100) {
        DrawRectangle(x - 1, 0, 2, ARENA_HEIGHT, ColorAlpha(gridLine, 0.4f));
    }
    for (int y = 0; y < ARENA_HEIGHT; y += 100) {
        DrawRectangle(0, y - 1, ARENA_WIDTH, 2, ColorAlpha(gridLine, 0.4f));
    }

    for (int x = 0; x < ARENA_WIDTH; x += 100) {
        DrawRectangle(x - 8, 0, 16, ARENA_HEIGHT, ColorAlpha(roadLine, 0.3f));
    }
    for (int y = 0; y < ARENA_HEIGHT; y += 100) {
        DrawRectangle(0, y - 8, ARENA_WIDTH, 16, ColorAlpha(roadLine, 0.3f));
    }

    for (int x = 50; x < ARENA_WIDTH; x += 100) {
        for (int y = 50; y < ARENA_HEIGHT; y += 100) {
            DrawCircle(x, y, 2, ColorAlpha(YELLOW, 0.15f));
        }
    }

    for (int i = 0; i < game.buildingCount; i++) {
        Building& b = game.buildings[i];
        DrawRectangleRec(b.rect, b.color);

        Color windowColor = {200, 180, 80, 80};
        int winSeed = (int)(b.rect.x * 7 + b.rect.y * 13);
        for (int wx = (int)b.rect.x + 8; wx < b.rect.x + b.rect.width - 8; wx += 16) {
            for (int wy = (int)b.rect.y + 8; wy < b.rect.y + b.rect.height - 8; wy += 16) {
                int hash = (wx * 7 + wy * 13 + winSeed) % 100;
                if (hash > 30) {
                    DrawRectangle(wx, wy, 8, 8, windowColor);
                }
            }
        }

        Color outlineColor = {80, 80, 90};
        DrawRectangleLinesEx(b.rect, 1, ColorAlpha(outlineColor, 0.6f));
    }

    DrawRectangle(0, 0, ARENA_WIDTH, 4, RED);
    DrawRectangle(0, ARENA_HEIGHT - 4, ARENA_WIDTH, 4, RED);
    DrawRectangle(0, 0, 4, ARENA_HEIGHT, RED);
    DrawRectangle(ARENA_WIDTH - 4, 0, 4, ARENA_HEIGHT, RED);
}

void DrawCureChoice(Game& game) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    ClearBackground(BLACK);

    Color bgColor = {10, 10, 15, 255};
    DrawRectangle(0, 0, sw, sh, bgColor);

    DrawText("THE CHOICE", sw / 2 - MeasureText("THE CHOICE", 40) / 2, 80, 40, WHITE);

    DrawText(TextFormat("Cure Samples: %d/%d", game.cureSamples, REQUIRED_SAMPLES),
             sw / 2 - 80, 140, 18, SKYBLUE);

    int boxY = 200;
    int boxH = 120;
    int boxW = 400;
    int boxX = sw / 2 - boxW / 2;

    Color cureBg = (game.choiceSelection == 0) ? ColorAlpha(SKYBLUE, 0.2f) : ColorAlpha(SKYBLUE, 0.05f);
    Color killBg = (game.choiceSelection == 1) ? ColorAlpha(RED, 0.2f) : ColorAlpha(RED, 0.05f);

    DrawRectangle(boxX, boxY, boxW, boxH, cureBg);
    DrawRectangleLinesEx({(float)boxX, (float)boxY, (float)boxW, (float)boxH}, 2,
                         (game.choiceSelection == 0) ? SKYBLUE : GRAY);
    DrawText("USE THE CURE", sw / 2 - MeasureText("USE THE CURE", 24) / 2, boxY + 20, 24, SKYBLUE);
    DrawText("Turn the zombies back into humans.", sw / 2 - MeasureText("Turn the zombies back into humans.", 14) / 2, boxY + 55, 14, WHITE);
    DrawText("Save Adam. Save everyone.", sw / 2 - MeasureText("Save Adam. Save everyone.", 14) / 2, boxY + 75, 14, WHITE);

    int killY = boxY + boxH + 30;
    DrawRectangle(boxX, killY, boxW, boxH, killBg);
    DrawRectangleLinesEx({(float)boxX, (float)killY, (float)boxW, (float)boxH}, 2,
                         (game.choiceSelection == 1) ? RED : GRAY);
    DrawText("DESTROY THEM ALL", sw / 2 - MeasureText("DESTROY THEM ALL", 24) / 2, killY + 20, 24, RED);
    DrawText("Eradicate every zombie.", sw / 2 - MeasureText("Eradicate every zombie.", 14) / 2, killY + 55, 14, WHITE);
    DrawText("Make sure they never come back.", sw / 2 - MeasureText("Make sure they never come back.", 14) / 2, killY + 75, 14, WHITE);

    float pulse = 0.5f + 0.5f * sinf(GetTime() * 3.0f);
    DrawText("[W/S] Select  [ENTER] Confirm", sw / 2 - 140, sh - 60, 16, ColorAlpha(WHITE, pulse));
}

void DrawEnding(Game& game, bool isCure) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    ClearBackground(BLACK);

    float alpha = game.endingTimer / 3.0f;
    if (alpha > 1.0f) alpha = 1.0f;

    if (isCure) {
        Color skyTop = {20, 30, 60};
        Color skyBot = {40, 60, 100};
        DrawRectangleGradientV(0, 0, sw, sh, skyTop, skyBot);

        Color buildingColor = {30, 35, 45};
        int buildings[] = {100, 160, 130, 200, 110, 180, 140, 170, 120, 150};
        int numBuildings = sizeof(buildings) / sizeof(buildings[0]);
        int bw = sw / numBuildings;
        for (int i = 0; i < numBuildings; i++) {
            int bh = buildings[i];
            DrawRectangle(i * bw, sh - bh, bw - 4, bh, buildingColor);
        }

        DrawCircle(sw / 2, sh / 2 - 40, 60 + sinf(GetTime()) * 5, ColorAlpha(GREEN, 0.4f * alpha));
        DrawCircle(sw / 2, sh / 2 - 40, 30, ColorAlpha(SKYBLUE, 0.8f * alpha));

        DrawText("THE CURE WORKS", sw / 2 - MeasureText("THE CURE WORKS", 36) / 2, 60, 36, SKYBLUE);

        const char* lines[] = {
            "The cure spreads through the city.",
            "Zombies collapse... then stand up again.",
            "Their eyes clear. They remember who they were.",
            "Adam looks at you. He remembers your name.",
            "\"What... what happened?\" he whispers.",
            "\"You saved us. You saved everyone.\"",
            "The city begins to rebuild.",
            "Not with walls. With people.",
            "Together."
        };
        int numLines = sizeof(lines) / sizeof(lines[0]);
        for (int i = 0; i < numLines; i++) {
            float lineAlpha = alpha - i * 0.1f;
            if (lineAlpha > 0) {
                if (lineAlpha > 1.0f) lineAlpha = 1.0f;
                DrawText(lines[i], sw / 2 - MeasureText(lines[i], 16) / 2,
                         150 + i * 35, 16, ColorAlpha(WHITE, lineAlpha));
            }
        }
    } else {
        Color skyTop = {40, 10, 10};
        Color skyBot = {60, 20, 20};
        DrawRectangleGradientV(0, 0, sw, sh, skyTop, skyBot);

        Color buildingColor = {25, 20, 20};
        int buildings[] = {100, 160, 130, 200, 110, 180, 140, 170, 120, 150};
        int numBuildings = sizeof(buildings) / sizeof(buildings[0]);
        int bw = sw / numBuildings;
        for (int i = 0; i < numBuildings; i++) {
            int bh = buildings[i];
            DrawRectangle(i * bw, sh - bh, bw - 4, bh, buildingColor);
        }

        for (int i = 0; i < 5; i++) {
            float fx = sw / 6.0f * (i + 1);
            float fy = sh - 80;
            float flicker = sinf(GetTime() * 6 + i * 1.5f) * 8;
            DrawCircle(fx, fy, 12 + flicker, ColorAlpha(RED, 0.6f * alpha));
            DrawCircle(fx, fy - 8, 6 + flicker * 0.5f, ColorAlpha(ORANGE, 0.5f * alpha));
        }

        DrawText("THEY'RE ALL DEAD", sw / 2 - MeasureText("THEY'RE ALL DEAD", 36) / 2, 60, 36, RED);

        const char* lines[] = {
            "The last zombie falls.",
            "Silence fills the streets.",
            "You did it. They're all gone.",
            "But Adam... he's gone too.",
            "You stand alone in the empty city.",
            "The buildings stand hollow.",
            "No more groans. No more screams.",
            "Just wind through broken windows.",
            "Was this the right choice?"
        };
        int numLines = sizeof(lines) / sizeof(lines[0]);
        for (int i = 0; i < numLines; i++) {
            float lineAlpha = alpha - i * 0.1f;
            if (lineAlpha > 0) {
                if (lineAlpha > 1.0f) lineAlpha = 1.0f;
                DrawText(lines[i], sw / 2 - MeasureText(lines[i], 16) / 2,
                         150 + i * 35, 16, ColorAlpha(WHITE, lineAlpha));
            }
        }
    }

    if (game.endingTimer > 3.0f) {
        float pulse = 0.5f + 0.5f * sinf(GetTime() * 3.0f);
        DrawText("Press ENTER", sw / 2 - 50, sh - 50, 18, ColorAlpha(WHITE, pulse));
    }
}

void DrawGame(Game& game) {
    if (game.state == MENU) {
        DrawMenu();
        return;
    }

    if (game.state == STORY) {
        ClearBackground(BLACK);
        DrawStory(game.story);
        return;
    }

    if (game.state == GAME_OVER) {
        DrawGameOver(game.player.score, game.currentLevel, game.playTime);
        return;
    }

    if (game.state == VICTORY) {
        DrawVictory(game.player.score, game.playTime);
        return;
    }

    if (game.state == CURE_CHOICE) {
        DrawCureChoice(game);
        return;
    }

    if (game.state == ENDING_CURE) {
        DrawEnding(game, true);
        return;
    }

    if (game.state == ENDING_KILL) {
        DrawEnding(game, false);
        return;
    }

    if (game.state == LEVEL_COMPLETE) {
        DrawLevelComplete(game.currentLevel, game.player.score, game.zombiesKilledThisLevel);
        return;
    }

    BeginMode2D(game.camera);

    if (game.screenShake) {
        game.camera.offset.x = GetScreenWidth() / 2.0f + ((float)GetRandomValue(-100, 100) / 100.0f) * game.shakeIntensity;
        game.camera.offset.y = GetScreenHeight() / 2.0f + ((float)GetRandomValue(-100, 100) / 100.0f) * game.shakeIntensity;
    }

    ClearBackground(DARKGRAY);

    DrawCityEnvironment(game);
    DrawProjectiles(game.projectiles, game.projectileCount);
    DrawLoots(game.loots, game.lootCount);
    DrawChest(game.chest);
    DrawZombies(game.zombies, game.zombieCount);
    DrawPlayer(game.player);
    DrawParticles(game.particles, game.particleCount);

    EndMode2D();

    bool chestNearby = false;
    if (game.chest.active && !game.chest.opened && !game.chest.falling) {
        float cdx = game.player.position.x - game.chest.position.x;
        float cdy = game.player.position.y - game.chest.position.y;
        chestNearby = sqrtf(cdx * cdx + cdy * cdy) < 80;
    }

    DrawHUD(game.player.health, game.player.maxHealth, game.player.armor,
            game.player.score, game.player.xp, game.player.level,
            game.inventory, game.slotUnlocked, game.currentWeapon, game.currentLevel,
            game.playTime, game.zombiesKilledThisLevel,
            game.levels[game.currentLevel - 1].zombiesRequired, chestNearby);

    if (game.cureSamples > 0) {
        DrawText(TextFormat("Cure: %d/%d", game.cureSamples, REQUIRED_SAMPLES), 10, GetScreenHeight() - 30, 16, SKYBLUE);
    }

    DrawStory(game.story);
    DrawMessage(game.ui);

    if (game.state == PAUSED) {
        DrawPauseMenu();
    }
}

void CleanupGame(Game& game) {
}
