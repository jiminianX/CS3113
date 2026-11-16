/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#include "CS3113/GameLevel.h"
#include "CS3113/BossLevel.h"
#include "CS3113/TextScene.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 1080, // 20 tiles of 54px
              SCREEN_HEIGHT = 648,  // 12 tiles of 54px
              FPS           = 120;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

constexpr char GAME_TITLE[] = "Joseph's Trials";

// Global Variables
AppStatus gAppStatus       = RUNNING;
float     gPreviousTicks   = 0.0f,
          gTimeAccumulator = 0.0f;

Camera2D gCamera = {};

Scene                     *gCurrentScene   = nullptr;
std::map<SceneID, Scene *> gScenes         = {};
SceneID                    gRequestedScene = NO_SCENE; // set by debug keys
SoundBank                  gSounds         = {};

// Function Declarations
Sound loadSoundEffect(const char *path);
void  loadSounds();
void  unloadSounds();
void  createScenes();
void  focusCamera(bool shouldSnap);
void  switchToScene(SceneID id);
void  initialise();
void  processInput();
void  update();
void  render();
void  shutdown();

Sound loadSoundEffect(const char *path)
{
    Sound sound = LoadSound(path);
    if (sound.frameCount == 0) LOG("WARNING: could not load sound " << path);
    return sound;
}

// SFX: "The Essential Retro Video Game Sound Effects Collection" by
// Juhani Junkala (CC0), via OpenGameArt.
void loadSounds()
{
    gSounds.jump    = loadSoundEffect("assets/sfx/jump.wav");
    gSounds.hurt    = loadSoundEffect("assets/sfx/hurt.wav");
    gSounds.clear   = loadSoundEffect("assets/sfx/clear.wav");
    gSounds.bossHit = loadSoundEffect("assets/sfx/boss_hit.wav");
    gSounds.win     = loadSoundEffect("assets/sfx/win.wav");
    gSounds.lose    = loadSoundEffect("assets/sfx/lose.wav");
}

void unloadSounds()
{
    UnloadSound(gSounds.jump);
    UnloadSound(gSounds.hurt);
    UnloadSound(gSounds.clear);
    UnloadSound(gSounds.bossHit);
    UnloadSound(gSounds.win);
    UnloadSound(gSounds.lose);
    gSounds = {};
}

void createScenes()
{
    const char *controls =
        "A/D or arrows: move    W/Space: jump    1-4: level select    Q: quit";

    TextSceneConfig menu = {
        GAME_TITLE, "Press ENTER to start", controls,
        "#1E2A44", "#FFD34E", LEVEL_1, "assets/music/pixelland.mp3", nullptr
    };
    TextSceneConfig win = {
        "You Win!", "The Crate King is crushed. Press ENTER for the menu",
        nullptr, "#1F5135", "#9CFF8A", MENU, nullptr, &gSounds.win
    };
    TextSceneConfig lose = {
        "You Lose", "Out of lives. Press ENTER for the menu",
        nullptr, "#4A1622", "#FF6B6B", MENU, nullptr, &gSounds.lose
    };

    gScenes[MENU]    = new TextScene(ORIGIN, menu, &gSounds);
    gScenes[LEVEL_1] = new GameLevel(level1Config(), &gSounds);
    gScenes[LEVEL_2] = new GameLevel(level2Config(), &gSounds);
    gScenes[LEVEL_3] = new GameLevel(level3Config(), &gSounds);
    gScenes[LEVEL_4] = new BossLevel(bossLevelConfig(), &gSounds);
    gScenes[WIN]     = new TextScene(ORIGIN, win,  &gSounds);
    gScenes[LOSE]    = new TextScene(ORIGIN, lose, &gSounds);
}

// Follows the player horizontally, but never shows past the map's edges.
void focusCamera(bool shouldSnap)
{
    if (!gCurrentScene->hasPlayer())
    {
        gCamera.target = ORIGIN;
        return;
    }

    const GameState &state = gCurrentScene->getState();
    Map *map = state.map;

    Vector2 desired = {
        state.player->getPosition().x,
        (map->getTopBoundary() + map->getBottomBoundary()) / 2.0f
    };

    if (shouldSnap) gCamera.target = desired;
    else            panCamera(&gCamera, &desired);

    float minX = map->getLeftBoundary()  + SCREEN_WIDTH / 2.0f;
    float maxX = map->getRightBoundary() - SCREEN_WIDTH / 2.0f;

    gCamera.target.x = minX < maxX ? Clamp(gCamera.target.x, minX, maxX)
                                   : (minX + maxX) / 2.0f;
    gCamera.target.y = desired.y;
}

void switchToScene(SceneID id)
{
    std::map<SceneID, Scene *>::iterator entry = gScenes.find(id);
    if (entry == gScenes.end())
    {
        LOG("ERROR: no scene registered for id " << id);
        return;
    }

    if (gCurrentScene) gCurrentScene->shutdown();
    gCurrentScene = entry->second;
    gCurrentScene->initialise();
    focusCamera(true);
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    InitAudioDevice();

    loadSounds();
    createScenes();

    gCamera.offset   = ORIGIN;
    gCamera.rotation = 0.0f;
    gCamera.zoom     = 1.0f;

    switchToScene(MENU);

    SetTargetFPS(FPS);
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose())
    {
        gAppStatus = TERMINATED;
        return;
    }

    // Debug feature: number keys jump straight to a level.
    const std::pair<KeyboardKey, SceneID> LEVEL_KEYS[] = {
        { KEY_ONE, LEVEL_1 }, { KEY_TWO,  LEVEL_2 },
        { KEY_THREE, LEVEL_3 }, { KEY_FOUR, LEVEL_4 },
    };

    for (const std::pair<KeyboardKey, SceneID> &binding : LEVEL_KEYS)
    {
        if (!IsKeyPressed(binding.first)) continue;

        if (Scene::getLives() <= 0) Scene::resetLives();
        gRequestedScene = binding.second;
        return;
    }

    gCurrentScene->processInput();
}

void update()
{
    // Delta time
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;

        focusCamera(false);
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
    for (std::pair<const SceneID, Scene *> &entry : gScenes)
        delete entry.second;

    gScenes.clear();
    gCurrentScene = nullptr;

    unloadSounds();
    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        if (gAppStatus != RUNNING) break;

        update();

        SceneID nextScene = gRequestedScene != NO_SCENE
            ? gRequestedScene
            : gCurrentScene->getState().nextSceneID;
        gRequestedScene = NO_SCENE;

        if (nextScene != NO_SCENE) switchToScene(nextScene);

        render();
    }

    shutdown();

    return 0;
}
