#pragma once
#include "raylib.h"

#define MAX_STORY_LINES 80

struct StoryLine {
    int levelTrigger;
    const char* speaker;
    const char* text;
    float displayTime;
    bool skipable;
};

struct Story {
    StoryLine lines[MAX_STORY_LINES];
    int lineCount;
    int currentLine;
    bool active;
    float timer;
    bool waitingForInput;
    bool introPlaying;
    int introStep;
    float introTimer;
    int introPhase;
    float fadeAlpha;
};

void InitStory(Story& story);
void TriggerStory(Story& story, int level);
void TriggerIntro(Story& story);
void UpdateStory(Story& story, float dt);
void DrawStory(Story& story);
void SkipStory(Story& story);
