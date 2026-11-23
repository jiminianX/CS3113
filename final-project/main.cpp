/**
* Author: Joseph Jiminian
* Assignment: Seven Star Rising (CS3113 final project)
* Date due: 12/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
*
* Generative-AI disclosure: written with help from Claude (Anthropic),
* used with instructor permission for this project.
**/

#include "CS3113/EndScreen.h"
#include "CS3113/Menu.h"
#include "CS3113/Valley.h"

// Global Constants
constexpr int FPS = 120;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Scene                     *gCurrentScene = nullptr;
std::map<SceneID, Scene *> gLevels       = {};

Camera2D gCamera = { 0 };

// Function Declarations
void switchToScene(Scene *scene);
void keepCameraInsideMap();
void initialise();
void processInput();
void update();
void render();
void shutdown();

void switchToScene(Scene *scene)
{
    if (gCurrentScene) gCurrentScene->shutdown();
    gCurrentScene = scene;
    gCurrentScene->initialise();

    // Menu screens have no hero, so the camera just looks at the screen.
    if (gCurrentScene->getState().hero)
    {
        gCamera.target = gCurrentScene->getState().hero->getPosition();
        keepCameraInsideMap();
    }
    else gCamera.target = ORIGIN;
}

// Stops the camera from showing anything past the edges of the map.
void keepCameraInsideMap()
{
    Map *map = gCurrentScene->getState().map;
    if (map == nullptr) return;

    float minX = map->getLeftBoundary()   + SCREEN_WIDTH  / 2.0f;
    float maxX = map->getRightBoundary()  - SCREEN_WIDTH  / 2.0f;
    float minY = map->getTopBoundary()    + SCREEN_HEIGHT / 2.0f;
    float maxY = map->getBottomBoundary() - SCREEN_HEIGHT / 2.0f;

    if (gCamera.target.x < minX) gCamera.target.x = minX;
    if (gCamera.target.x > maxX) gCamera.target.x = maxX;
    if (gCamera.target.y < minY) gCamera.target.y = minY;
    if (gCamera.target.y > maxY) gCamera.target.y = maxY;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Seven Star Rising");

    gLevels[MENU]           = new Menu(ORIGIN, "#141B33");
    gLevels[VALLEY]         = new Valley(VALLEY_ORIGIN, "#2E5E2A");
    gLevels[VALLEY_CLEARED] = new EndScreen(
        ORIGIN, "#173B2A", "VALLEY CLEARED",
        "The rocky wasteland lies ahead... (end of the Playtest 1 build)", GREEN
    );
    gLevels[GAME_OVER]      = new EndScreen(
        ORIGIN, "#3A1220", "GAME OVER",
        "The warlord gathers the orbs and makes his wish.", RED
    );

    gCamera.offset   = ORIGIN;
    gCamera.rotation = 0.0f;
    gCamera.zoom     = 1.0f;

    switchToScene(gLevels[MENU]);

    SetTargetFPS(FPS);
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

    gCurrentScene->processInput();
}

void update()
{
    float ticks     = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);

        if (gCurrentScene->getState().hero)
        {
            Vector2 cameraTarget = gCurrentScene->getState().hero->getPosition();
            panCamera(&gCamera, &cameraTarget);
            keepCameraInsideMap();
        }

        deltaTime -= FIXED_TIMESTEP;
    }

    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();

    BeginMode2D(gCamera);
    gCurrentScene->render();
    EndMode2D();

    gCurrentScene->renderUI();

    EndDrawing();
}

void shutdown()
{
    for (std::pair<const SceneID, Scene *> &entry : gLevels)
        delete entry.second;
    gLevels.clear();

    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();

        if (gCurrentScene->getState().nextSceneID != NO_SCENE)
        {
            SceneID id = gCurrentScene->getState().nextSceneID;
            switchToScene(gLevels[id]);
        }

        render();
    }

    shutdown();

    return 0;
}
