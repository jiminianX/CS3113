/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

#include "Scene.h"
#include "LevelData.h"

// A scrolling platformer level built from a LevelConfig. Levels 1-3 are
// three GameLevel objects with different configs; the boss level extends it.
class GameLevel : public Scene
{
protected:
    LevelConfig  mConfig;
    ParsedLayout mLayout;
    Vector2      mPlayerStart = { 0.0f, 0.0f };

    Vector2 tileCentre(int row, int col) const;
    float   fallLimit() const;

    void spawnPlayer();
    void spawnGoal();
    void spawnEnemies();

    void respawnPlayer();
    void hurtPlayer();
    void keepPlayerInsideMap();

    void renderHearts() const;

    // Hooks the boss level overrides.
    virtual Entity *createEnemy(const Spawn &spawn);
    virtual void    checkEnemyContacts();

public:
    static constexpr float TILE_SIZE             = 54.0f;
    static constexpr int   TILESET_COLUMNS       = 20;
    static constexpr int   TILESET_ROWS          = 9;
    static constexpr float GRAVITY               = 1500.0f;
    static constexpr float PLAYER_SPEED          = 270.0f;
    static constexpr float PLAYER_JUMP_POWER     = 760.0f;
    static constexpr float RESPAWN_INVINCIBILITY = 1.5f;

    GameLevel(const LevelConfig &config, const SoundBank *sounds);
    ~GameLevel();

    void initialise()            override;
    void processInput()          override;
    void update(float deltaTime) override;
    void render()                override;
    void renderUI()              override;
    void shutdown()              override;
};

#endif // GAME_LEVEL_H
