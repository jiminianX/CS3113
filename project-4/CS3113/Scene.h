/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#ifndef SCENE_H
#define SCENE_H

#include "Entity.h"

// Sound effects are loaded once by main and shared by every scene, so a
// sound that starts right before a scene switch is not cut off when the old
// scene unloads its resources.
struct SoundBank
{
    Sound jump;
    Sound hurt;
    Sound clear;
    Sound bossHit;
    Sound win;
    Sound lose;
};

struct GameState
{
    Entity               *player = nullptr;
    Map                  *map    = nullptr;
    std::vector<Entity *> enemies;
    std::vector<Entity *> goalParts;

    Music bgm      = {};
    bool  hasMusic = false;

    SceneID nextSceneID = NO_SCENE;
};

class Scene
{
protected:
    GameState        mGameState;
    Vector2          mOrigin;
    const char      *mBGColourHexCode = "#000000";
    const SoundBank *mSounds          = nullptr;
    bool             mIsLoaded        = false;

    // Lives belong to the whole run, not to one level, so they are shared
    // by every scene object.
    static int sLives;

    static void loseLife() { if (sLives > 0) sLives--; }

    void startMusic(const char *musicPath);
    void updateMusic();
    void stopMusic();
    void playSound(const Sound &sound) const;

public:
    static constexpr int   STARTING_LIVES = 3;
    static constexpr float MUSIC_VOLUME   = 0.33f;

    Scene(Vector2 origin, const char *bgHexCode, const SoundBank *sounds);
    virtual ~Scene() {}

    Scene(const Scene &)            = delete;
    Scene &operator=(const Scene &) = delete;

    virtual void initialise()            = 0;
    virtual void processInput()          = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render()                = 0; // world space (inside camera)
    virtual void renderUI()              = 0; // screen space (HUD / text)
    virtual void shutdown()              = 0;

    const GameState &getState()  const { return mGameState;                 }
    Vector2          getOrigin() const { return mOrigin;                    }
    bool             hasPlayer() const { return mGameState.player != nullptr; }

    static int  getLives()   { return sLives;          }
    static void resetLives() { sLives = STARTING_LIVES; }
};

#endif // SCENE_H
