/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#include "GameLevel.h"

namespace
{
    const char *TILES_TEXTURE     = "assets/tiles/tiles.png";
    const char *CREATURES_TEXTURE = "assets/sprites/creatures.png";
    const char *HERO_TEXTURE      = "assets/sprites/hero.png";

    // Sprite sheet dimensions are { rows, columns } (see getUVRectangle).
    const Vector2 TILES_GRID     = { 9.0f, 20.0f };
    const Vector2 CREATURES_GRID = { 3.0f,  9.0f };
    const Vector2 HERO_GRID      = { 5.0f,  6.0f };

    const Vector2 HERO_SIZE      = { 48.0f, 48.0f };
    const Vector2 HERO_COLLIDER  = { 26.0f, 44.0f };
    const Vector2 ENEMY_SIZE     = { 48.0f, 48.0f };
    const Vector2 ENEMY_COLLIDER = { 36.0f, 44.0f };

    const int FLAG_FRAME_A  = 111;
    const int FLAG_FRAME_B  = 112;
    const int POLE_TILE     = 131;
    const int HEART_FULL    = 44;
    const int HEART_EMPTY   = 46;
    const int HEART_SIZE    = 40;
    const int HUD_MARGIN    = 16;
    const int HUD_FONT_SIZE = 24;
    const int HUD_PANEL_PADDING = 6;

    const int GOAL_FRAME_SPEED  = 4;
    const int ENEMY_FRAME_SPEED = 8;

    struct EnemyStyle
    {
        AIType           aiType;
        std::vector<int> frames;
        float            speed;
    };

    EnemyStyle styleFor(char kind)
    {
        switch (kind)
        {
        case Layout::FOLLOWER: return { FOLLOWER, { 15, 16, 17 }, 140.0f };
        case Layout::FLYER:    return { FLYER,    { 24, 25, 26 }, 110.0f };
        default:               return { WANDERER, { 18, 19, 20 }, 100.0f };
        }
    }

    Entity *makeTileEntity(Vector2 position, std::vector<int> frames, EntityType type)
    {
        AnimationAtlas atlas = { { ANIM_IDLE, frames } };
        Entity *entity = new Entity(
            position, { GameLevel::TILE_SIZE, GameLevel::TILE_SIZE },
            TILES_TEXTURE, ATLAS, TILES_GRID, atlas, type
        );
        entity->setFrameSpeed(GOAL_FRAME_SPEED);
        return entity;
    }
}

GameLevel::GameLevel(const LevelConfig &config, const SoundBank *sounds)
    : Scene{ { 0.0f, 0.0f }, config.bgColour, sounds },
      mConfig{ config },
      mLayout{ parseLayout(config.layout, config.theme) }
{
    // The map is centred on mOrigin; put its top-left corner at (0, 0).
    mOrigin = {
        mLayout.columns * TILE_SIZE / 2.0f,
        mLayout.rows    * TILE_SIZE / 2.0f
    };
}

GameLevel::~GameLevel() { shutdown(); }

Vector2 GameLevel::tileCentre(int row, int col) const
{
    return {
        mGameState.map->getLeftBoundary() + col * TILE_SIZE + TILE_SIZE / 2.0f,
        mGameState.map->getTopBoundary()  + row * TILE_SIZE + TILE_SIZE / 2.0f
    };
}

float GameLevel::fallLimit() const
{
    return mGameState.map->getBottomBoundary() + TILE_SIZE;
}

/*
    ----------- SETUP -----------
*/
void GameLevel::initialise()
{
    shutdown(); // safe if already unloaded; avoids leaks on re-entry

    mGameState.nextSceneID = NO_SCENE;
    startMusic(mConfig.musicPath);

    mGameState.map = new Map(
        mLayout.columns, mLayout.rows,
        mLayout.tiles.data(),
        TILES_TEXTURE, TILE_SIZE,
        TILESET_COLUMNS, TILESET_ROWS,
        mOrigin
    );

    spawnPlayer();
    spawnGoal();
    spawnEnemies();

    mIsLoaded = true;
}

void GameLevel::spawnPlayer()
{
    AnimationAtlas heroAtlas = {
        { ANIM_IDLE, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 3 } }, // blink now and then
        { ANIM_RUN,  { 6, 7, 8, 9 } },
        { ANIM_JUMP, { 10 } },
        { ANIM_FALL, { 11 } },
    };

    mPlayerStart = tileCentre(mLayout.playerRow, mLayout.playerCol);

    // Hero sprite: "Classic Hero" by GrafxKid (CC0), via OpenGameArt.
    Entity *player = new Entity(
        mPlayerStart, HERO_SIZE, HERO_TEXTURE, ATLAS, HERO_GRID, heroAtlas, PLAYER
    );
    player->setColliderDimensions(HERO_COLLIDER);
    player->setAcceleration({ 0.0f, GRAVITY });
    player->setJumpingPower(PLAYER_JUMP_POWER);
    player->setSpeed(PLAYER_SPEED);
    player->setFlipsWhenFacingLeft(true);

    mGameState.player = player;
}

void GameLevel::spawnGoal()
{
    if (!mLayout.hasGoal) return;

    Vector2 flagPosition = tileCentre(mLayout.goalRow,     mLayout.goalCol);
    Vector2 polePosition = tileCentre(mLayout.goalRow + 1, mLayout.goalCol);

    mGameState.goalParts.push_back(
        makeTileEntity(flagPosition, { FLAG_FRAME_A, FLAG_FRAME_B }, GOAL));
    mGameState.goalParts.push_back(
        makeTileEntity(polePosition, { POLE_TILE }, GOAL));
}

void GameLevel::spawnEnemies()
{
    for (const Spawn &spawn : mLayout.spawns)
    {
        Entity *enemy = createEnemy(spawn);
        if (enemy != nullptr) mGameState.enemies.push_back(enemy);
    }
}

Entity *GameLevel::createEnemy(const Spawn &spawn)
{
    EnemyStyle     style = styleFor(spawn.kind);
    AnimationAtlas atlas = { { ANIM_RUN, style.frames } };

    // Enemy sprites: Kenney "Pixel Platformer" (CC0).
    Entity *enemy = new Entity(
        tileCentre(spawn.row, spawn.col), ENEMY_SIZE,
        CREATURES_TEXTURE, ATLAS, CREATURES_GRID, atlas, NPC
    );
    enemy->setAIType(style.aiType);
    enemy->setAIState(IDLE);
    enemy->setSpeed(style.speed * mConfig.enemySpeedScale);
    enemy->setColliderDimensions(ENEMY_COLLIDER);
    enemy->setFrameSpeed(ENEMY_FRAME_SPEED);
    enemy->setPatrolBounds(
        tileCentre(spawn.row, spawn.minCol).x,
        tileCentre(spawn.row, spawn.maxCol).x
    );

    if (style.aiType != FLYER) enemy->setAcceleration({ 0.0f, GRAVITY });

    return enemy;
}

/*
    ----------- GAMEPLAY -----------
*/
void GameLevel::processInput()
{
    Entity *player = mGameState.player;
    if (player == nullptr) return;

    player->resetMovement();

    if      (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  player->moveLeft();
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) player->moveRight();

    bool isJumpPressed = IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) ||
                         IsKeyPressed(KEY_SPACE);

    if (isJumpPressed && player->isCollidingBottom())
    {
        player->jump();
        playSound(mSounds->jump);
    }
}

void GameLevel::update(float deltaTime)
{
    updateMusic();

    // Once we've decided to leave, freeze the level until main switches.
    if (mGameState.nextSceneID != NO_SCENE) return;

    Entity *player = mGameState.player;
    Map    *map    = mGameState.map;

    player->update(deltaTime, nullptr, map, nullptr, 0);
    keepPlayerInsideMap();

    for (Entity *enemy : mGameState.enemies)
    {
        enemy->update(deltaTime, player, map, nullptr, 0);
        if (enemy->getPosition().y > fallLimit()) enemy->deactivate();
    }

    for (Entity *part : mGameState.goalParts)
        part->update(deltaTime, nullptr, nullptr, nullptr, 0);

    if (player->getPosition().y > fallLimit())
    {
        hurtPlayer();
        return;
    }

    checkEnemyContacts();

    if (mGameState.nextSceneID == NO_SCENE &&
        player->findCollision(mGameState.goalParts) != nullptr)
    {
        playSound(mSounds->clear);
        mGameState.nextSceneID = mConfig.nextScene;
    }
}

// Collisions are checked from the player's point of view.
void GameLevel::checkEnemyContacts()
{
    Entity *player = mGameState.player;
    if (player->isInvincible()) return;

    if (player->findCollision(mGameState.enemies) != nullptr) hurtPlayer();
}

void GameLevel::hurtPlayer()
{
    loseLife();
    playSound(mSounds->hurt);

    if (getLives() <= 0)
    {
        mGameState.nextSceneID = LOSE;
        return;
    }

    respawnPlayer();
}

void GameLevel::respawnPlayer()
{
    Entity *player = mGameState.player;
    player->setPosition(mPlayerStart);
    player->setVelocity({ 0.0f, 0.0f });
    player->startInvincibility(RESPAWN_INVINCIBILITY);
}

void GameLevel::keepPlayerInsideMap()
{
    Entity *player    = mGameState.player;
    Vector2 position  = player->getPosition();
    float   halfWidth = player->getColliderDimensions().x / 2.0f;
    float   minX      = mGameState.map->getLeftBoundary()  + halfWidth;
    float   maxX      = mGameState.map->getRightBoundary() - halfWidth;

    if (position.x < minX || position.x > maxX)
        player->setPosition({ Clamp(position.x, minX, maxX), position.y });
}

/*
    ----------- DRAWING -----------
*/
void GameLevel::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    mGameState.map->render();

    for (Entity *part  : mGameState.goalParts) part->render();
    for (Entity *enemy : mGameState.enemies)   enemy->render();

    mGameState.player->render();
}

void GameLevel::renderHearts() const
{
    Texture2D tiles = mGameState.map->getTextureAtlas();

    // Faint panel so the hearts stay readable over any background.
    DrawRectangle(
        HUD_MARGIN - HUD_PANEL_PADDING, HUD_MARGIN - HUD_PANEL_PADDING,
        STARTING_LIVES * (HEART_SIZE + 4) + HUD_PANEL_PADDING,
        HEART_SIZE + 2 * HUD_PANEL_PADDING,
        Fade(BLACK, 0.3f)
    );

    for (int i = 0; i < STARTING_LIVES; i++)
    {
        int       tile   = i < getLives() ? HEART_FULL : HEART_EMPTY;
        Rectangle source = getUVRectangle(&tiles, tile, TILESET_ROWS, TILESET_COLUMNS);
        Rectangle dest   = {
            static_cast<float>(HUD_MARGIN + i * (HEART_SIZE + 4)),
            static_cast<float>(HUD_MARGIN),
            static_cast<float>(HEART_SIZE),
            static_cast<float>(HEART_SIZE)
        };
        DrawTexturePro(tiles, source, dest, { 0.0f, 0.0f }, 0.0f, WHITE);
    }
}

void GameLevel::renderUI()
{
    renderHearts();

    int nameWidth = MeasureText(mConfig.name, HUD_FONT_SIZE);
    int nameX     = GetScreenWidth() - nameWidth - HUD_MARGIN;
    DrawText(mConfig.name, nameX + 2, HUD_MARGIN + 10, HUD_FONT_SIZE, BLACK);
    DrawText(mConfig.name, nameX,     HUD_MARGIN + 8,  HUD_FONT_SIZE, RAYWHITE);
}

void GameLevel::shutdown()
{
    if (!mIsLoaded) return;

    delete mGameState.player;
    mGameState.player = nullptr;

    for (Entity *enemy : mGameState.enemies) delete enemy;
    mGameState.enemies.clear();

    for (Entity *part : mGameState.goalParts) delete part;
    mGameState.goalParts.clear();

    delete mGameState.map;
    mGameState.map = nullptr;

    stopMusic();
    mIsLoaded = false;
}
