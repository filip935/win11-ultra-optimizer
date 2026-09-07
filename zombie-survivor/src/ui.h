#pragma once
#include "raylib.h"
#include "weapon.h"

struct UI {
    bool showControls;
    float transitionAlpha;
    bool transitioning;
    int transitionTarget;
    float messageTimer;
    char message[256];
    float xpBarAnim;
    float healthBarAnim;
};

void InitUI(UI& ui);
void DrawHUD(int health, int maxHealth, int armor, int score, int xp, int level, Weapon inventory[], bool slotUnlocked[], int currentSlot, int currentLevel, float time, int zombiesKilled, int zombiesRequired, bool chestNearby);
void DrawMenu();
void DrawPauseMenu();
void DrawGameOver(int score, int level, float time);
void DrawVictory(int score, float time);
void DrawLevelComplete(int level, int score, int killed);
void DrawMessage(UI& ui);
void ShowMessage(UI& ui, const char* msg);
