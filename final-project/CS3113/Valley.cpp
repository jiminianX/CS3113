/**
* Author: Joseph Jiminian
* Assignment: seven star rising
* Date due: n/a, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Valley.h"

Valley::Valley()                                      : Scene{ {},     nullptr   } {}
Valley::Valley(Vector2 origin, const char *bgHexCode) : Scene{ origin, bgHexCode } {}

Valley::~Valley() { shutdown(); }

// Centre of a { column, row } tile in world coordinates.
Vector2 Valley::tilePosition(Vector2 tile)
{
    return {
        mGroundMap->getLeftBoundary() + tile.x * VALLEY_TILE_SIZE + VALLEY_TILE_SIZE / 2.0f,
        mGroundMap->getTopBoundary()  + tile.y * VALLEY_TILE_SIZE + VALLEY_TILE_SIZE / 2.0f
    };
}

void Valley::initialise()
{
    mGameState.nextSceneID = NO_SCENE;

    // The gate may have been opened during a previous run.
    for (int i = 0; i < GATE_COUNT; i++)
        mSolidData[(int) GATE_TILES[i].y * VALLEY_WIDTH + (int) GATE_TILES[i].x] = GATE_TILE;

    /* ----------- MAP (two layers of the same tileset) ----------- */
    mGroundMap = new Map(
        VALLEY_WIDTH, VALLEY_HEIGHT,          // map grid cols & rows
        (unsigned int *) mGroundData,         // grid data
        "assets/sprites/valley_tiles.png",    // texture filepath
        VALLEY_TILE_SIZE,                     // tile size
        8, 1,                                 // texture cols & rows
        mOrigin                               // in-game origin
    );

    mGameState.map = new Map(
        VALLEY_WIDTH, VALLEY_HEIGHT,
        (unsigned int *) mSolidData,
        "assets/sprites/valley_tiles.png",
        VALLEY_TILE_SIZE,
        8, 1,
        mOrigin
    );

    // Ninja Adventure character sheets: 4 columns (down, up, left, right),
    // walking frames run down the first 4 rows.
    std::map<Direction, std::vector<int>> characterAtlas = {
        { DOWN,  { 0, 4,  8, 12 } },
        { UP,    { 1, 5,  9, 13 } },
        { LEFT,  { 2, 6, 10, 14 } },
        { RIGHT, { 3, 7, 11, 15 } },
    };

    std::map<Direction, std::vector<int>> blastAtlas = {
        { DOWN,  { 0, 1, 2, 3 } },
        { UP,    { 0, 1, 2, 3 } },
        { LEFT,  { 0, 1, 2, 3 } },
        { RIGHT, { 0, 1, 2, 3 } },
    };

    /* ----------- HERO ----------- */
    mGameState.hero = new Entity(
        tilePosition(HERO_TILE),           // position
        { 48.0f, 48.0f },                  // scale
        "assets/sprites/hero.png",         // texture file address
        ATLAS,                             // single image or atlas?
        { 7, 4 },                          // atlas dimensions (rows, cols)
        characterAtlas,                    // actual atlas
        PLAYER                             // entity type
    );
    mGameState.hero->setSpeed(HERO_SPEED);
    mGameState.hero->setColliderDimensions({ 26.0f, 26.0f });

    /* ----------- SOLDIERS (patrol, then chase) ----------- */
    const char *soldierTextures[2] = {
        "assets/sprites/soldier_knight.png",
        "assets/sprites/soldier_gladiator.png"
    };

    for (int i = 0; i < SOLDIER_COUNT; i++)
    {
        Vector2 position = tilePosition(SOLDIER_TILES[i]);
        Vector2 patrol   = Vector2Scale(PATROL_AXES[i], PATROL_DISTANCE);

        mEnemies[i] = new Entity(position, { 48.0f, 48.0f }, soldierTextures[i % 2],
                                 ATLAS, { 7, 4 }, characterAtlas, NPC);
        mEnemies[i]->setSpeed(SOLDIER_SPEED);
        mEnemies[i]->setHealth(SOLDIER_HEALTH);
        mEnemies[i]->setColliderDimensions({ 26.0f, 26.0f });
        mEnemies[i]->setAIType(WANDERER);
        mEnemies[i]->setAIState(WALKING);
        mEnemies[i]->setPatrolPoints(Vector2Subtract(position, patrol), Vector2Add(position, patrol));
    }

    /* ----------- SCOUTS (hover, then fly at you) ----------- */
    for (int i = 0; i < SCOUT_COUNT; i++)
    {
        Entity *scout = new Entity(tilePosition(SCOUT_TILES[i]), { 48.0f, 48.0f },
                                   "assets/sprites/scout_mage.png", ATLAS, { 7, 4 },
                                   characterAtlas, NPC);
        scout->setSpeed(SCOUT_SPEED);
        scout->setHealth(SCOUT_HEALTH);
        scout->setColliderDimensions({ 26.0f, 26.0f });
        scout->setAIType(FOLLOWER);
        scout->setAIState(IDLE);
        scout->setBobHeight(5.0f);

        mEnemies[SOLDIER_COUNT + i] = scout;
    }

    /* ----------- STAR ORBS ----------- */
    for (int i = 0; i < ORB_COUNT; i++)
    {
        mOrbs[i] = new Entity(tilePosition(ORB_TILES[i]), { 42.0f, 42.0f },
                              "assets/sprites/orb.png", ORB);
        mOrbs[i]->setColliderDimensions({ 30.0f, 30.0f });
        mOrbs[i]->setBobHeight(4.0f);
    }

    /* ----------- KI BLASTS (a fixed set, reused) ----------- */
    for (int i = 0; i < BLAST_COUNT; i++)
    {
        mBlasts[i] = new Entity({ 0.0f, 0.0f }, { 32.0f, 32.0f }, "assets/sprites/ki_blast.png",
                                ATLAS, { 1, 4 }, blastAtlas, BLAST);
        mBlasts[i]->setColliderDimensions({ 16.0f, 16.0f });
        mBlasts[i]->setSpeed(BLAST_SPEED);
        mBlasts[i]->setFrameSpeed(12);
        mBlasts[i]->deactivate();
    }

    /* ----------- HUD ----------- */
    mHeartTexture = LoadTexture("assets/sprites/hearts.png");
    mFaceTexture  = LoadTexture("assets/sprites/hero_face.png");
    mOrbIcon      = LoadTexture("assets/sprites/orb.png");

    mHealth         = MAX_HEALTH;
    mLives          = STARTING_LIVES;
    mOrbsCollected  = 0;
    mKi             = MAX_KI;
    mIsCharging     = false;
    mIsGateOpen     = false;
    mTime           = 0.0f;
    mBlastCooldown  = 0.0f;
    mKiWarningTimer = 0.0f;

    showBanner("Mountain Valley: find 3 star orbs");
}

/*
    ----------- INPUT -----------
*/

void Valley::processInput()
{
    Entity *hero = mGameState.hero;
    hero->resetMovement();

    // Charging: stand still and refill ki. Leaves you open to attack.
    mIsCharging = IsKeyDown(KEY_K);
    if (mIsCharging)
    {
        hero->showPose(CHARGE_POSE_FRAME, 0.1f);
        return;
    }

    if      (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))   hero->moveUp();
    else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) hero->moveDown();

    if      (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  hero->moveLeft();
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) hero->moveRight();

    // to avoid faster diagonal speed
    if (GetLength(hero->getMovement()) > 1.0f) hero->normaliseMovement();

    if (IsKeyPressed(KEY_J) || IsKeyPressed(KEY_SPACE)) fireBlast();
}

void Valley::fireBlast()
{
    if (mBlastCooldown > 0.0f) return;

    if (mKi < BLAST_KI_COST)
    {
        mKiWarningTimer = KI_WARNING_TIME;
        return;
    }

    Entity   *hero   = mGameState.hero;
    Direction facing = hero->getDirection();

    for (int i = 0; i < BLAST_COUNT; i++)
    {
        if (mBlasts[i]->isActive()) continue;

        Entity *blast = mBlasts[i];
        blast->resetMovement();

        switch (facing)
        {
        case UP:    blast->moveUp();    break;
        case DOWN:  blast->moveDown();  break;
        case LEFT:  blast->moveLeft();  break;
        case RIGHT: blast->moveRight(); break;
        }

        blast->setPosition(Vector2Add(hero->getPosition(),
                                      Vector2Scale(blast->getMovement(), BLAST_SPAWN_OFFSET)));
        blast->setLifeTimer(BLAST_LIFETIME);
        blast->activate();

        mKi            -= BLAST_KI_COST;
        mBlastCooldown  = BLAST_COOLDOWN;
        hero->showPose(PUNCH_FIRST_FRAME + facing, PUNCH_POSE_TIME);
        return;
    }
}

/*
    ----------- UPDATE -----------
*/

void Valley::update(float deltaTime)
{
    mTime           += deltaTime;
    mBlastCooldown  -= deltaTime;
    mKiWarningTimer -= deltaTime;
    mBannerTimer    -= deltaTime;

    if (mIsCharging)
    {
        mKi += CHARGE_RATE * deltaTime;
        if (mKi > MAX_KI) mKi = MAX_KI;
    }

    Entity *hero = mGameState.hero;
    hero->update(deltaTime, nullptr, mGameState.map);

    // Scouts fly, so they ignore the solid tiles (and can cross the river).
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        Map *map = mEnemies[i]->getAIType() == FOLLOWER ? nullptr : mGameState.map;
        mEnemies[i]->update(deltaTime, hero, map);
    }

    for (int i = 0; i < ORB_COUNT; i++) mOrbs[i]->update(deltaTime, nullptr, nullptr);

    for (int i = 0; i < BLAST_COUNT; i++)
    {
        mBlasts[i]->update(deltaTime, nullptr, mGameState.map);

        bool hitWall = mBlasts[i]->isCollidingLeft() || mBlasts[i]->isCollidingRight() ||
                       mBlasts[i]->isCollidingTop()  || mBlasts[i]->isCollidingBottom();
        if (hitWall) mBlasts[i]->deactivate();
    }

    hitEnemiesWithBlasts();
    checkEnemyContact();
    if (mGameState.nextSceneID != NO_SCENE) return;

    collectOrbs();
    if (mIsGateOpen && isHeroOnExit()) mGameState.nextSceneID = VALLEY_CLEARED;
}

void Valley::hitEnemiesWithBlasts()
{
    for (int b = 0; b < BLAST_COUNT; b++)
    {
        if (!mBlasts[b]->isActive()) continue;

        for (int e = 0; e < ENEMY_COUNT; e++)
        {
            if (!mBlasts[b]->isColliding(mEnemies[e])) continue;

            mEnemies[e]->takeDamage(1);
            mEnemies[e]->flashRed(HURT_FLASH_TIME);
            mEnemies[e]->knockBack(mBlasts[b]->getPosition(), ENEMY_KNOCKBACK);
            mBlasts[b]->deactivate();
            break;
        }
    }
}

void Valley::checkEnemyContact()
{
    Entity *hero = mGameState.hero;
    if (hero->isInvincible()) return;

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (!hero->isColliding(mEnemies[i])) continue;

        // Hits hurt twice as much while charging.
        mHealth -= mIsCharging ? 2 : 1;

        if (mHealth <= 0)
        {
            mLives--;
            if (mLives <= 0) mGameState.nextSceneID = GAME_OVER;
            else             respawnHero();
            return;
        }

        hero->knockBack(mEnemies[i]->getPosition(), HERO_KNOCKBACK);
        hero->flashRed(HURT_FLASH_TIME);
        hero->startInvincibility(HIT_INVINCIBILITY);
        return;
    }
}

void Valley::respawnHero()
{
    Entity *hero = mGameState.hero;
    hero->setPosition(tilePosition(HERO_TILE));
    hero->resetMovement();
    hero->setKnockback({ 0.0f, 0.0f });
    hero->startInvincibility(RESPAWN_INVINCIBILITY);

    for (int i = 0; i < BLAST_COUNT; i++) mBlasts[i]->deactivate();

    mHealth = MAX_HEALTH;
    mKi     = MAX_KI;
    showBanner("You were knocked out! Lives left: " + std::to_string(mLives));
}

void Valley::collectOrbs()
{
    for (int i = 0; i < ORB_COUNT; i++)
    {
        if (!mGameState.hero->isColliding(mOrbs[i])) continue;

        mOrbs[i]->deactivate();
        mOrbsCollected++;
        showBanner("Star orb found!  " + std::to_string(mOrbsCollected) + " / " +
                   std::to_string(TOTAL_ORBS));
    }

    if (!mIsGateOpen && mOrbsCollected == ORB_COUNT) openGate();
}

// Removing the gate tiles from the solid layer lets the hero walk through.
void Valley::openGate()
{
    for (int i = 0; i < GATE_COUNT; i++)
        mSolidData[(int) GATE_TILES[i].y * VALLEY_WIDTH + (int) GATE_TILES[i].x] = 0;

    mIsGateOpen = true;
    showBanner("All orbs here found - the stone gate to the east is open!");
}

bool Valley::isHeroOnExit()
{
    Vector2 position = mGameState.hero->getPosition();
    int column = (int) floor((position.x - mGroundMap->getLeftBoundary()) / VALLEY_TILE_SIZE);
    int row    = (int) floor((position.y - mGroundMap->getTopBoundary())  / VALLEY_TILE_SIZE);

    for (int i = 0; i < EXIT_COUNT; i++)
        if (column == (int) EXIT_TILES[i].x && row == (int) EXIT_TILES[i].y) return true;

    return false;
}

void Valley::showBanner(std::string text)
{
    mBanner      = text;
    mBannerTimer = BANNER_TIME;
}

/*
    ----------- RENDER -----------
*/

void Valley::renderAura()
{
    Vector2 centre = mGameState.hero->getPosition();
    float   pulse  = 4.0f * sinf(mTime * 14.0f);

    DrawPoly(centre, 8, 36.0f + pulse,  mTime * 90.0f,  Fade(GOLD,   0.35f));
    DrawPoly(centre, 8, 26.0f - pulse, -mTime * 120.0f, Fade(YELLOW, 0.35f));
}

void Valley::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    mGroundMap->render();
    mGameState.map->render();

    // The way out glows once the gate is open.
    if (mIsGateOpen)
    {
        for (int i = 0; i < EXIT_COUNT; i++)
        {
            Vector2 centre = tilePosition(EXIT_TILES[i]);
            DrawRectangle((int) (centre.x - VALLEY_TILE_SIZE / 2.0f),
                          (int) (centre.y - VALLEY_TILE_SIZE / 2.0f),
                          (int) VALLEY_TILE_SIZE, (int) VALLEY_TILE_SIZE,
                          Fade(GOLD, 0.25f + 0.15f * sinf(mTime * 4.0f)));
        }
    }

    for (int i = 0; i < ORB_COUNT;   i++) mOrbs[i]->render();
    for (int i = 0; i < ENEMY_COUNT; i++) mEnemies[i]->render();

    if (mIsCharging) renderAura();
    mGameState.hero->render();

    for (int i = 0; i < BLAST_COUNT; i++) mBlasts[i]->render();
}

void Valley::renderHud()
{
    // Hearts (hearts.png: 5 frames, empty -> full) and lives.
    DrawRectangle(16, 16, 300, 100, Fade(BLACK, 0.6f));

    for (int i = 0; i < MAX_HEALTH; i++)
    {
        int frame = i < mHealth ? 4 : 0;
        DrawTexturePro(mHeartTexture, getUVRectangle(&mHeartTexture, frame, 1, 5),
                       { 30.0f + i * 34.0f, 26.0f, 32.0f, 32.0f }, { 0.0f, 0.0f }, 0.0f, WHITE);
    }

    DrawTexturePro(mFaceTexture,
                   { 0.0f, 0.0f, (float) mFaceTexture.width, (float) mFaceTexture.height },
                   { 204.0f, 23.0f, 38.0f, 38.0f }, { 0.0f, 0.0f }, 0.0f, WHITE);
    DrawText(("x" + std::to_string(mLives)).c_str(), 248, 34, 20, RAYWHITE);

    // Ki bar. The black tick marks the cost of one blast.
    bool  isWarningOn = mKiWarningTimer > 0.0f && fmodf(mTime * 6.0f, 1.0f) < 0.5f;
    Color barColour   = mIsCharging ? YELLOW : GOLD;

    DrawText("KI", 30, 76, 20, isWarningOn ? RED : RAYWHITE);
    DrawRectangle(64, 78, 200, 16, DARKGRAY);
    DrawRectangle(64, 78, (int) (200 * mKi / MAX_KI), 16, barColour);
    DrawRectangle(64 + (int) (200 * BLAST_KI_COST / MAX_KI), 78, 2, 16, BLACK);
    DrawRectangleLines(64, 78, 200, 16, isWarningOn ? RED : LIGHTGRAY);
    if (mIsCharging) DrawText("CHARGING", 272, 79, 16, YELLOW);

    // Orb counter and area name.
    DrawRectangle(SCREEN_WIDTH - 236, 16, 220, 76, Fade(BLACK, 0.6f));
    DrawTexturePro(mOrbIcon,
                   { 0.0f, 0.0f, (float) mOrbIcon.width, (float) mOrbIcon.height },
                   { SCREEN_WIDTH - 222.0f, 26.0f, 28.0f, 28.0f }, { 0.0f, 0.0f }, 0.0f, WHITE);
    DrawText((std::to_string(mOrbsCollected) + " / " + std::to_string(TOTAL_ORBS)).c_str(),
             SCREEN_WIDTH - 182, 30, 24, RAYWHITE);
    DrawText("Mountain Valley", SCREEN_WIDTH - 222, 66, 16, LIGHTGRAY);
}

void Valley::renderMessages()
{
    if (mBannerTimer > 0.0f)
    {
        float alpha = mBannerTimer < 0.5f ? mBannerTimer / 0.5f : 1.0f;
        DrawRectangle(200, 132, 880, 52, Fade(BLACK, 0.6f * alpha));
        DrawText(mBanner.c_str(), 220, 144, 28, Fade(GOLD, alpha));
    }

    if (mTime < HINT_TIME)
    {
        float alpha = HINT_TIME - mTime < 2.0f ? (HINT_TIME - mTime) / 2.0f : 1.0f;
        DrawRectangle(310, SCREEN_HEIGHT - 52, 660, 34, Fade(BLACK, 0.6f * alpha));
        DrawText("WASD / Arrows: move    J / Space: ki blast    Hold K: charge ki    Q: quit",
                 326, SCREEN_HEIGHT - 43, 16, Fade(RAYWHITE, alpha));
    }
}

void Valley::renderUI()
{
    renderHud();
    renderMessages();
}

void Valley::shutdown()
{
    delete mGameState.hero;
    mGameState.hero = nullptr;
    delete mGameState.map;
    mGameState.map = nullptr;
    delete mGroundMap;
    mGroundMap = nullptr;

    for (int i = 0; i < ENEMY_COUNT; i++) { delete mEnemies[i]; mEnemies[i] = nullptr; }
    for (int i = 0; i < ORB_COUNT;   i++) { delete mOrbs[i];    mOrbs[i]    = nullptr; }
    for (int i = 0; i < BLAST_COUNT; i++) { delete mBlasts[i];  mBlasts[i]  = nullptr; }

    UnloadTexture(mHeartTexture);
    mHeartTexture = {};
    UnloadTexture(mFaceTexture);
    mFaceTexture = {};
    UnloadTexture(mOrbIcon);
    mOrbIcon = {};
}
