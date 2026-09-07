#include "ui.h"
#include "weapon.h"
#include <cmath>
#include <cstring>
#include <stdio.h>

void InitUI(UI& ui) {
    ui.showControls = false;
    ui.transitionAlpha = 0;
    ui.transitioning = false;
    ui.transitionTarget = 0;
    ui.messageTimer = 0;
    ui.message[0] = '\0';
    ui.xpBarAnim = 0;
    ui.healthBarAnim = 1.0f;
}

void DrawHUD(int health, int maxHealth, int armor, int score, int xp, int level, Weapon inventory[], bool slotUnlocked[], int currentSlot, int currentLevel, float time, int zombiesKilled, int zombiesRequired, bool chestNearby) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float healthPct = (float)health / maxHealth;
    DrawRectangle(10, 10, 204, 24, ColorAlpha(BLACK, 0.6f));
    DrawRectangle(12, 12, 200 * healthPct, 20, healthPct > 0.5f ? GREEN : healthPct > 0.25f ? YELLOW : RED);

    char healthText[32];
    snprintf(healthText, sizeof(healthText), "%d/%d", health, maxHealth);
    DrawText(healthText, 80, 13, 14, WHITE);

    if (armor > 0) {
        DrawRectangle(10, 40, 204, 18, ColorAlpha(BLACK, 0.6f));
        float armorPct = (float)armor / 100;
        DrawRectangle(12, 42, 200 * armorPct, 14, BLUE);
        DrawText(TextFormat("Armor: %d", armor), 80, 43, 12, WHITE);
    }

    DrawText(TextFormat("Score: %d", score), 10, 65, 16, WHITE);
    DrawText(TextFormat("Level: %d", level), 10, 85, 16, YELLOW);
    DrawText(TextFormat("Stage: %d/100", currentLevel), 10, 105, 16, SKYBLUE);

    DrawText(TextFormat("Kills: %d/%d", zombiesKilled, zombiesRequired), screenWidth - 180, 10, 16, WHITE);

    int mins = (int)time / 60;
    int secs = (int)time % 60;
    DrawText(TextFormat("Time: %02d:%02d", mins, secs), screenWidth - 180, 30, 16, WHITE);

    Weapon& currentWeapon = inventory[currentSlot];

    int slotW = 110;
    int slotH = 42;
    int gap = 6;
    int totalW = MAX_WEAPON_SLOTS * slotW + (MAX_WEAPON_SLOTS - 1) * gap;
    int slotsX = screenWidth / 2 - totalW / 2;
    int slotsY = screenHeight - 54;
    for (int i = 0; i < MAX_WEAPON_SLOTS; i++) {
        int x = slotsX + i * (slotW + gap);
        DrawRectangle(x, slotsY, slotW, slotH, i == currentSlot ? ColorAlpha(WHITE, 0.22f) : ColorAlpha(BLACK, 0.7f));
        DrawText(TextFormat("%d", i + 1), x + 4, slotsY + 2, 12, GRAY);
        if (slotUnlocked[i]) {
            Color rc = inventory[i].rarityColor;
            DrawRectangle(x, slotsY + slotH - 3, slotW, 3, rc);
            DrawText(inventory[i].name, x + 20, slotsY + 4, 10, rc);
            DrawText(TextFormat("Ammo %d", inventory[i].ammo), x + 20, slotsY + 20, 12, ORANGE);
        } else {
            DrawText("LOCKED", x + 28, slotsY + 15, 12, DARKGRAY);
        }
    }

    if (chestNearby) {
        float pulse = 0.6f + 0.4f * sinf(GetTime() * 4.0f);
        DrawText("[E] Open Chest", screenWidth / 2 - 60, screenHeight / 2 + 50, 18, ColorAlpha(GOLD, pulse));
    }
}

void DrawMenu() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);

    const char* title = "ZOMBIE SURVIVOR";
    int titleSize = 60;
    DrawText(title, screenWidth / 2 - MeasureText(title, titleSize) / 2, screenHeight / 4, titleSize, RED);

    const char* subtitle = "100 Levels of Horror";
    DrawText(subtitle, screenWidth / 2 - MeasureText(subtitle, 20) / 2, screenHeight / 4 + 80, 20, DARKGRAY);

    float pulse = 0.7f + 0.3f * sinf(GetTime() * 2.0f);
    const char* start = "Press ENTER to Start";
    DrawText(start, screenWidth / 2 - MeasureText(start, 24) / 2, screenHeight / 2, 24, ColorAlpha(WHITE, pulse));

    const char* controls = "WASD: Move | Mouse: Aim/Shoot | SPACE: Dash | 1-5: Weapon slots | ESC: Pause";
    DrawText(controls, screenWidth / 2 - MeasureText(controls, 14) / 2, screenHeight / 2 + 60, 14, GRAY);

    const char* credit = "A game about survival";
    DrawText(credit, screenWidth / 2 - MeasureText(credit, 12) / 2, screenHeight - 40, 12, DARKGRAY);
}

void DrawPauseMenu() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.7f));

    DrawText("PAUSED", screenWidth / 2 - MeasureText("PAUSED", 40) / 2, screenHeight / 3, 40, WHITE);
    DrawText("Press ESC to Resume", screenWidth / 2 - MeasureText("Press ESC to Resume", 18) / 2, screenHeight / 2, 18, GRAY);
    DrawText("Press Q to Quit", screenWidth / 2 - MeasureText("Press Q to Quit", 18) / 2, screenHeight / 2 + 40, 18, GRAY);
}

void DrawGameOver(int score, int level, float time) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(RED, 0.3f));

    DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 50) / 2, screenHeight / 4, 50, RED);

    DrawText(TextFormat("Score: %d", score), screenWidth / 2 - 60, screenHeight / 2 - 20, 24, WHITE);
    DrawText(TextFormat("Level Reached: %d", level), screenWidth / 2 - 80, screenHeight / 2 + 20, 24, WHITE);

    int mins = (int)time / 60;
    int secs = (int)time % 60;
    DrawText(TextFormat("Time: %02d:%02d", mins, secs), screenWidth / 2 - 50, screenHeight / 2 + 60, 24, WHITE);

    float pulse = 0.7f + 0.3f * sinf(GetTime() * 2.0f);
    DrawText("Press ENTER to Restart", screenWidth / 2 - MeasureText("Press ENTER to Restart", 20) / 2, screenHeight / 2 + 120, 20, ColorAlpha(WHITE, pulse));
}

void DrawVictory(int score, float time) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(GOLD, 0.1f));

    DrawText("VICTORY!", screenWidth / 2 - MeasureText("VICTORY!", 50) / 2, screenHeight / 4, 50, GOLD);
    DrawText("The Alpha has been defeated!", screenWidth / 2 - MeasureText("The Alpha has been defeated!", 20) / 2, screenHeight / 4 + 70, 20, WHITE);

    DrawText(TextFormat("Final Score: %d", score), screenWidth / 2 - 70, screenHeight / 2, 24, WHITE);

    int mins = (int)time / 60;
    int secs = (int)time % 60;
    DrawText(TextFormat("Time: %02d:%02d", mins, secs), screenWidth / 2 - 50, screenHeight / 2 + 40, 24, WHITE);

    float pulse = 0.7f + 0.3f * sinf(GetTime() * 2.0f);
    DrawText("Press ENTER for Menu", screenWidth / 2 - MeasureText("Press ENTER for Menu", 20) / 2, screenHeight / 2 + 100, 20, ColorAlpha(WHITE, pulse));
}

void DrawLevelComplete(int level, int score, int killed) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.5f));

    DrawText(TextFormat("LEVEL %d COMPLETE", level), screenWidth / 2 - MeasureText(TextFormat("LEVEL %d COMPLETE", level), 36) / 2, screenHeight / 3, 36, GREEN);
    DrawText(TextFormat("Zombies Killed: %d", killed), screenWidth / 2 - 80, screenHeight / 2, 20, WHITE);
    DrawText(TextFormat("Score: %d", score), screenWidth / 2 - 50, screenHeight / 2 + 30, 20, WHITE);

    float pulse = 0.7f + 0.3f * sinf(GetTime() * 2.0f);
    DrawText("Press ENTER to Continue", screenWidth / 2 - MeasureText("Press ENTER to Continue", 20) / 2, screenHeight / 2 + 80, 20, ColorAlpha(WHITE, pulse));
}

void DrawMessage(UI& ui) {
    if (ui.messageTimer > 0) {
        int screenWidth = GetScreenWidth();
        int alpha = (int)(255 * (ui.messageTimer / 3.0f));
        DrawText(ui.message, screenWidth / 2 - MeasureText(ui.message, 18) / 2, 100, 18, ColorAlpha(WHITE, alpha / 255.0f));
    }
}

void ShowMessage(UI& ui, const char* msg) {
    strncpy(ui.message, msg, sizeof(ui.message) - 1);
    ui.message[sizeof(ui.message) - 1] = '\0';
    ui.messageTimer = 3.0f;
}
