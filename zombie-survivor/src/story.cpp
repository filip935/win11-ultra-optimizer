#include "story.h"
#include <cmath>
#include <string.h>

static const char* introTexts[] = {
    "This city was fine.",
    "Until.",
    "Your friend, Adam, got sick two weeks ago.",
    "You watched him change. His eyes went dark.",
    "He bit someone. Then they bit someone else.",
    "Now 50% of the town is zombies.",
    "You need to find a cure... or kill them all.",
    "100 levels stand between you and survival."
};

static const char* introSpeakers[] = {
    "", "", "", "", "", "", "", ""
};

#define INTRO_LINES (sizeof(introTexts) / sizeof(introTexts[0]))

void InitStory(Story& story) {
    story.lineCount = 0;
    story.currentLine = 0;
    story.active = false;
    story.timer = 0;
    story.waitingForInput = false;
    story.introPlaying = false;
    story.introStep = 0;
    story.introTimer = 0;
    story.introPhase = 0;
    story.fadeAlpha = 0;

    struct { int level; const char* speaker; const char* text; float time; bool skip; } lines[] = {
        { 0, "RADIO", "This is Commander Reyes. If anyone hears this, get to the safe zone.", 3.0f, true },
        { 0, "RADIO", "Use WASD to move. Mouse to aim and shoot. SPACE to dash.", 3.0f, true },
        { 0, "RADIO", "Walk over loot to collect it. Press E near supply chests.", 3.0f, true },

        { 3, "RADIO", "Good. You're alive. Stay sharp out there.", 2.5f, true },
        { 5, "RADIO", "The horde is getting thicker. They're drawn to noise.", 2.5f, true },
        { 5, "RADIO", "Kill zombies to get loot. Walk over items to collect them.", 2.5f, true },

        { 10, "SCIENTIST", "Hello? Is anyone there? This is Dr. Chen.", 3.0f, true },
        { 10, "SCIENTIST", "I was working at the bio lab before the outbreak.", 3.0f, true },
        { 10, "SCIENTIST", "The zombies... they're mutating. Evolving.", 3.0f, true },
        { 10, "SCIENTIST", "Their skin is getting thicker. Harder to damage.", 3.0f, true },
        { 10, "SCIENTIST", "But their DNA is unstable. We can still reverse it.", 3.0f, true },
        { 10, "SCIENTIST", "Bring me samples from the stronger zombies.", 3.0f, true },

        { 15, "RADIO", "Runners incoming! They're fast, don't let them close in.", 2.5f, true },
        { 15, "SCIENTIST", "The fast ones... they adapted to chase prey more efficiently.", 2.5f, true },

        { 20, "SCIENTIST", "I've been studying the tissue samples.", 3.0f, true },
        { 20, "SCIENTIST", "The zombies are developing acid sacs.", 3.0f, true },
        { 20, "SCIENTIST", "Spitters can attack from range now. Be careful.", 3.0f, true },
        { 20, "SCIENTIST", "I need more samples. Keep collecting them.", 3.0f, true },

        { 25, "RADIO", "The mutations are getting worse. Spitters on the field.", 2.5f, true },

        { 30, "SCIENTIST", "They're getting smarter. Organizing.", 3.0f, true },
        { 30, "SCIENTIST", "The Brutes... massive mutations. Heavy armor.", 3.0f, true },
        { 30, "SCIENTIST", "Their DNA samples are key to the cure.", 3.0f, true },

        { 40, "RADIO", "We've lost contact with other safe zones.", 2.5f, true },
        { 40, "SCIENTIST", "I think you're the last one out there.", 2.5f, true },

        { 50, "SCIENTIST", "Halfway there. I can't believe you've made it this far.", 3.0f, true },
        { 50, "SCIENTIST", "The mutations are accelerating.", 3.0f, true },
        { 50, "SCIENTIST", "I've isolated a potential cure compound.", 3.0f, true },
        { 50, "SCIENTIST", "But I need a sample from the Alpha to complete it.", 3.0f, true },

        { 60, "SCIENTIST", "The Alpha has been spotted. It's organizing them.", 3.0f, true },
        { 60, "SCIENTIST", "Every zombie you kill weakens the hive.", 3.0f, true },

        { 70, "SCIENTIST", "They've adapted again. Tougher now.", 3.0f, true },
        { 70, "SCIENTIST", "But the cure is almost ready. I just need that sample.", 3.0f, true },

        { 75, "RADIO", "This is it. The final stretch.", 2.5f, true },
        { 75, "SCIENTIST", "Every zombie you kill brings us closer to ending this.", 2.5f, true },

        { 80, "SCIENTIST", "You've got this. I believe in you.", 3.0f, true },

        { 90, "SCIENTIST", "The Alpha knows you're coming.", 3.0f, true },
        { 90, "SCIENTIST", "It's sending everything it has.", 3.0f, true },
        { 90, "SCIENTIST", "The cure is ready. Just need that sample.", 3.0f, true },

        { 95, "RADIO", "One more push. For everyone we've lost.", 2.5f, true },
        { 95, "SCIENTIST", "For Adam. For the city. For humanity.", 2.5f, true },

        { 100, "SCIENTIST", "This is it. The Alpha is in your sector.", 3.0f, true },
        { 100, "SCIENTIST", "Get the sample. Bring it to my lab.", 3.0f, true },
        { 100, "SCIENTIST", "We can still save them. All of them.", 3.0f, true },

        { 101, "SCIENTIST", "You did it. You actually did it.", 3.0f, true },
        { 101, "SCIENTIST", "I have the sample. The cure is ready.", 3.0f, true },
        { 101, "SCIENTIST", "But now you have a choice...", 3.0f, true },
    };

    int count = sizeof(lines) / sizeof(lines[0]);
    for (int i = 0; i < count && i < MAX_STORY_LINES; i++) {
        story.lines[i].levelTrigger = lines[i].level;
        story.lines[i].speaker = lines[i].speaker;
        story.lines[i].text = lines[i].text;
        story.lines[i].displayTime = lines[i].time;
        story.lines[i].skipable = lines[i].skip;
        story.lineCount++;
    }
}

void TriggerStory(Story& story, int level) {
    for (int i = 0; i < story.lineCount; i++) {
        if (story.lines[i].levelTrigger == level) {
            story.currentLine = i;
            story.active = true;
            story.timer = story.lines[i].displayTime;
            story.waitingForInput = false;
            break;
        }
    }
}

void TriggerIntro(Story& story) {
    story.introPlaying = true;
    story.introStep = 0;
    story.introPhase = 0;
    story.introTimer = 0;
    story.fadeAlpha = 0;
    story.active = true;
}

void UpdateStory(Story& story, float dt) {
    if (!story.active) return;

    if (story.introPlaying) {
        story.introTimer += dt;

        bool pressedEnter = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);

        if (story.introPhase == 0) {
            story.fadeAlpha += dt * 1.5f;
            if (story.fadeAlpha > 1.0f) story.fadeAlpha = 1.0f;
            if (story.introTimer > 1.5f || pressedEnter) {
                story.introPhase = 1;
                story.introTimer = 0;
            }
        } else if (story.introPhase == 1) {
            if (story.introTimer > 3.5f || pressedEnter) {
                story.introPhase = 2;
                story.introTimer = 0;
            }
        } else if (story.introPhase == 2) {
            story.fadeAlpha -= dt * 4.0f;
            if (story.fadeAlpha < 0) story.fadeAlpha = 0;
            if (story.introTimer > 0.3f) {
                story.introStep++;
                if (story.introStep >= (int)INTRO_LINES) {
                    story.introPhase = 4;
                    story.introTimer = 0;
                } else {
                    story.introPhase = 3;
                    story.introTimer = 0;
                }
            }
        } else if (story.introPhase == 3) {
            story.fadeAlpha += dt * 4.0f;
            if (story.fadeAlpha > 1.0f) story.fadeAlpha = 1.0f;
            if (story.introTimer > 0.3f || pressedEnter) {
                story.introPhase = 1;
                story.introTimer = 0;
            }
        } else if (story.introPhase == 4) {
            story.fadeAlpha -= dt * 0.8f;
            if (story.fadeAlpha < 0) {
                story.fadeAlpha = 0;
                story.introPlaying = false;
                story.active = false;
            }
        }
    } else {
        story.timer -= dt;

        if (story.timer <= 0) {
            story.waitingForInput = true;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            story.active = false;
        }
    }
}

void DrawStory(Story& story) {
    if (!story.active) return;

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    if (story.introPlaying) {
        ClearBackground(BLACK);

        Color buildingColor = {20, 20, 25};
        Color windowDim = {40, 35, 20};
        Color windowLit = {180, 160, 60};
        Color skyColor = {10, 10, 30};

        DrawRectangle(0, 0, screenWidth, screenHeight, skyColor);

        int buildings[] = {80, 150, 120, 200, 90, 170, 130, 180, 100, 160, 140, 190, 110, 155, 125};
        int numBuildings = sizeof(buildings) / sizeof(buildings[0]);
        int bw = screenWidth / numBuildings;

        for (int i = 0; i < numBuildings; i++) {
            int bh = buildings[i];
            int bx = i * bw;
            int by = screenHeight - bh;

            DrawRectangle(bx, by, bw - 4, bh, buildingColor);

            for (int wy = by + 10; wy < screenHeight - 10; wy += 18) {
                for (int wx = bx + 8; wx < bx + bw - 12; wx += 14) {
                    int hash = (wx * 7 + wy * 13 + i * 31) % 100;
                    Color wc = (hash > 40) ? windowLit : windowDim;
                    DrawRectangle(wx, wy, 6, 8, wc);
                }
            }
        }

        Color fireColor = {200, 80, 20, 120};
        for (int i = 0; i < 3; i++) {
            int fx = screenWidth / 4 + i * screenWidth / 4;
            int fy = screenHeight - 60 - i * 20;
            float flicker = sinf(GetTime() * 8 + i * 2) * 10;
            DrawCircle(fx, fy, 15 + flicker, fireColor);
            DrawCircle(fx, fy - 10, 8 + flicker * 0.5, {255, 120, 30, 100});
        }

        if (story.introPhase == 0) {
            DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 1.0f - story.fadeAlpha));
        } else if (story.introPhase == 4) {
            DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 1.0f - story.fadeAlpha));
        }

        if (story.introStep < (int)INTRO_LINES && story.introPhase >= 1 && story.introPhase <= 3) {
            const char* text = introTexts[story.introStep];

            int textAlpha = (int)(story.fadeAlpha * 255);
            Color textColor = {255, 255, 255, (unsigned char)textAlpha};

            int textY = screenHeight / 2 - 10;
            if (story.introStep == 1) {
                textY = screenHeight / 2 - 30;
            }

            int fontSize = (story.introStep == 1) ? 48 : 24;
            int textW = MeasureText(text, fontSize);
            DrawText(text, screenWidth / 2 - textW / 2, textY, fontSize, textColor);
        }

        float progress = (float)story.introStep / (float)INTRO_LINES;
        DrawRectangle(0, screenHeight - 4, (int)(screenWidth * progress), 4, RED);
    } else {
        DrawRectangle(0, screenHeight - 120, screenWidth, 120, ColorAlpha(BLACK, 0.8f));

        const char* speaker = story.lines[story.currentLine].speaker;
        const char* text = story.lines[story.currentLine].text;

        Color speakerColor;
        if (speaker[0] == 'R') speakerColor = GREEN;
        else if (speaker[0] == 'S') speakerColor = SKYBLUE;
        else speakerColor = YELLOW;

        int textY = screenHeight - 80;
        DrawText(speaker, 40, screenHeight - 105, 16, speakerColor);

        int textWidth = MeasureText(text, 18);
        int maxWidth = screenWidth - 80;
        if (textWidth > maxWidth) {
            int half = (int)(strlen(text) / 2);
            char line1[128] = {0};
            char line2[128] = {0};
            strncpy(line1, text, half);
            line1[half] = '\0';
            strncpy(line2, text + half, strlen(text) - half);
            line2[strlen(text) - half] = '\0';
            DrawText(line1, 40, textY, 18, WHITE);
            DrawText(line2, 40, textY + 25, 18, WHITE);
        } else {
            DrawText(text, 40, textY, 18, WHITE);
        }

        if (story.waitingForInput) {
            float alpha = 0.5f + 0.5f * sinf(GetTime() * 3.0f);
            DrawText("[ENTER] Continue", screenWidth - 180, screenHeight - 30, 14, ColorAlpha(WHITE, alpha));
        }
    }
}

void SkipStory(Story& story) {
    story.active = false;
    story.introPlaying = false;
}
