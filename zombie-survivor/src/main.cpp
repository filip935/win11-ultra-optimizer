#include "raylib.h"
#include "game.h"

int main(void) {
    InitWindow(1280, 720, "Zombie Survivor - 100 Levels of Horror");
    SetTargetFPS(60);

    Game game;
    InitGame(game);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;

        UpdateGame(game, dt);

        BeginDrawing();
        DrawGame(game);
        EndDrawing();
    }

    CleanupGame(game);
    CloseWindow();

    return 0;
}
