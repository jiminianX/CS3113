/**
* Author: Joseph Jiminian
* Assignment: Lunar Lander
* Date due: 11/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
*
* Generative-AI disclosure: written with help from Claude (Anthropic),
* used with instructor permission for this project.
**/

#include "CS3113/Entity.h"
#include "CS3113/Hud.h"

// Global Constants
// SCREEN_WIDTH and SCREEN_HEIGHT live in Hud.h so the HUD can use them too.
constexpr int FPS           = 120,
              TERRAIN_COUNT = 6,
              PAD_COUNT     = 3, // two fixed pads + the moving barge
              BARGE_INDEX   = 2,
              METEOR_COUNT  = 2;

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f,
                MAX_FRAME_TIME = 0.25f; // don't simulate a huge jump after a stall

// ----- Physics (pixels / s²), 32 pixels = 1 metre ----- //
constexpr float PIXELS_PER_METER = 32.0f,
                GRAVITY          = 1.31f * PIXELS_PER_METER, // Europa's gravity
                MAIN_THRUST      = 3.4f  * PIXELS_PER_METER,
                SIDE_THRUST      = 3.6f  * PIXELS_PER_METER,
                HORIZONTAL_DRAG  = 1.1f; // per second: drift, then slow down

// ----- Fuel (units) ----- //
constexpr float MAX_FUEL       = 100.0f,
                MAIN_BURN_RATE = 5.0f, // per second while held
                SIDE_BURN_RATE = 3.5f, // per second per side thruster
                IGNITION_COST  = 0.5f; // per key press

// ----- Landing limits ----- //
constexpr float SAFE_DOWN_SPEED   = 2.0f * PIXELS_PER_METER,
                SAFE_SIDE_SPEED   = 1.2f * PIXELS_PER_METER,
                LANDING_TOLERANCE = 12.0f; // px the ship may sink into a pad

constexpr float BRIEF_DURATION = 5.0f, // seconds the mission brief stays up
                BRIEF_FADE     = 1.0f;

// ----- Lander: 5 rows x 4 columns of 190x190 frames ----- //
constexpr Vector2 SHIP_START    = { 640.0f, 80.0f },
                  SHIP_SIZE     = { 95.0f, 95.0f },
                  SHIP_COLLIDER = { 44.0f, 36.0f },  // the hull, not the flames
                  SHIP_SHEET    = { 5.0f, 4.0f };    // { rows, columns }
constexpr int     SHIP_FRAME_SPEED = 12;

// ----- Level layout: { left, top, width, height } ----- //
constexpr Rectangle TERRAIN_LAYOUT[TERRAIN_COUNT] = {
    {   0.0f, 500.0f, 170.0f, 220.0f }, // western cliff
    { 170.0f, 610.0f, 230.0f, 110.0f }, // lowland holding pad A
    { 400.0f, 430.0f, 100.0f, 290.0f }, // ice spire
    { 500.0f, 690.0f, 380.0f,  30.0f }, // canyon floor under the barge
    { 880.0f, 380.0f,  80.0f, 340.0f }, // tall ice spire
    { 960.0f, 560.0f, 320.0f, 160.0f }, // eastern plateau holding pad B
};

constexpr Rectangle PAD_LAYOUT[PAD_COUNT] = {
    {  250.0f, 594.0f,  80.0f, 16.0f }, // pad A: narrow
    { 1100.0f, 544.0f, 100.0f, 16.0f }, // pad B: wide but far away
    {  634.0f, 600.0f, 112.0f, 24.0f }, // supply barge (moves)
};

constexpr float BARGE_MIN_X = 590.0f,
                BARGE_MAX_X = 790.0f;
constexpr int   BARGE_SPEED = 60;

// Meteor 0 sweeps across pad A, meteor 1 bobs up and down before pad B.
constexpr Vector2 METEOR_STARTS[METEOR_COUNT] = { { 285.0f, 400.0f }, { 1020.0f, 300.0f } },
                  METEOR_SIZE     = { 44.0f, 44.0f },
                  METEOR_COLLIDER = { 31.0f, 31.0f }; // forgiving around the rock
constexpr float   METEOR_A_MIN_X  = 185.0f,
                  METEOR_A_MAX_X  = 385.0f,
                  METEOR_B_MIN_Y  = 190.0f,
                  METEOR_B_MAX_Y  = 410.0f,
                  METEOR_SPIN     = 90.0f; // degrees per second
constexpr int     METEOR_SPEED    = 90;

constexpr float FROST_THICKNESS = 4.0f,
                BARGE_DECK      = 6.0f;

constexpr char BG_COLOUR[]    = "#05060F",
               ICE_COLOUR[]   = "#9CC7E5",
               FROST_COLOUR[] = "#E2F4FF",
               PAD_COLOUR[]   = "#4DFF88",
               HULL_COLOUR[]  = "#55606E";

// ----- Assets (Kenney "Space Shooter Remastered", CC0) ----- //
constexpr char SHIP_SHEET_FP[] = "assets/ship_sheet.png",
               BACKGROUND_FP[] = "assets/kenney_space-shooter-remastered/Backgrounds/darkPurple.png",
               WIN_SOUND_FP[]  = "assets/kenney_space-shooter-remastered/Bonus/sfx_twoTone.ogg",
               LOSE_SOUND_FP[] = "assets/kenney_space-shooter-remastered/Bonus/sfx_lose.ogg";

const char *METEOR_FPS[METEOR_COUNT] = {
    "assets/kenney_space-shooter-remastered/PNG/Meteors/meteorGrey_med1.png",
    "assets/kenney_space-shooter-remastered/PNG/Meteors/meteorBrown_med1.png"
};

enum GameState { PLAYING, MISSION_ACCOMPLISHED, MISSION_FAILED };

// Global Variables
AppStatus gAppStatus       = RUNNING;
float     gPreviousTicks   = 0.0f,
          gTimeAccumulator = 0.0f;

Entity *gShip    = nullptr;
Entity *gTerrain = nullptr;
Entity *gPads    = nullptr;
Entity *gMeteors = nullptr;

Texture2D gBackground = {};
Sound     gWinSound   = {},
          gLoseSound  = {};

GameState      gGameState   = PLAYING;
MissionOutcome gOutcome     = LANDED_ON_PAD;
float          gFuel        = MAX_FUEL,
               gMissionTime = 0.0f;

bool gIsMainEngineOn    = false,
     gIsLeftThrusterOn  = false,
     gIsRightThrusterOn = false;

// Function Declarations
bool initialise();
void processInput();
void update();
void render();
void shutdown();

/* ------------------------------ SETUP ------------------------------ */

// Places a rectangle-shaped entity (terrain or pad) on the screen.
void setUpBlock(Entity *block, Rectangle area, EntityType type)
{
    Vector2 size = { area.width, area.height };

    block->setEntityType(type);
    block->setScale(size);
    block->setColliderDimensions(size);
    block->setPosition({ area.x + area.width / 2.0f, area.y + area.height / 2.0f });
}

void createShip()
{
    std::map<AnimationState, std::vector<int>> animationAtlas = {
        { IDLE,         {  0,  1,  2,  3 } },
        { THRUST_UP,    {  4,  5,  6,  7 } },
        { THRUST_LEFT,  {  8,  9, 10, 11 } },
        { THRUST_RIGHT, { 12, 13, 14, 15 } },
        { CRASHED,      { 16, 17, 18, 19 } },
    };

    gShip = new Entity(
        SHIP_START, SHIP_SIZE, SHIP_SHEET_FP, ATLAS, SHIP_SHEET, animationAtlas, PLAYER
    );
    gShip->setColliderDimensions(SHIP_COLLIDER);
    gShip->setFrameSpeed(SHIP_FRAME_SPEED);
}

void createLevel()
{
    gTerrain = new Entity[TERRAIN_COUNT];
    for (int i = 0; i < TERRAIN_COUNT; i++)
        setUpBlock(&gTerrain[i], TERRAIN_LAYOUT[i], BLOCK);

    gPads = new Entity[PAD_COUNT];
    for (int i = 0; i < PAD_COUNT; i++)
        setUpBlock(&gPads[i], PAD_LAYOUT[i], PLATFORM);
    gPads[BARGE_INDEX].setSpeed(BARGE_SPEED);

    gMeteors = new Entity[METEOR_COUNT];
    for (int i = 0; i < METEOR_COUNT; i++)
    {
        gMeteors[i].setTexture(METEOR_FPS[i]);
        gMeteors[i].setEntityType(HAZARD);
        gMeteors[i].setScale(METEOR_SIZE);
        gMeteors[i].setColliderDimensions(METEOR_COLLIDER);
        gMeteors[i].setSpeed(METEOR_SPEED);
    }
}

bool areTexturesLoaded()
{
    if (gShip->getTexture().id == 0 || gBackground.id == 0) return false;

    for (int i = 0; i < METEOR_COUNT; i++)
        if (gMeteors[i].getTexture().id == 0) return false;

    return true;
}

void cutThrusters()
{
    gIsMainEngineOn    = false;
    gIsLeftThrusterOn  = false;
    gIsRightThrusterOn = false;
}

// Puts everything back where it starts. Called at launch and on R.
void resetMission()
{
    gShip->setPosition(SHIP_START);
    gShip->setVelocity({ 0.0f, 0.0f });
    gShip->setAcceleration({ 0.0f, GRAVITY });
    gShip->setAnimation(IDLE);

    Rectangle barge = PAD_LAYOUT[BARGE_INDEX];
    gPads[BARGE_INDEX].setPosition({ barge.x + barge.width / 2.0f,
                                     barge.y + barge.height / 2.0f });
    gPads[BARGE_INDEX].resetMovement();
    gPads[BARGE_INDEX].moveRight();

    for (int i = 0; i < METEOR_COUNT; i++)
    {
        gMeteors[i].setPosition(METEOR_STARTS[i]);
        gMeteors[i].resetMovement();
        gMeteors[i].setAngle(0.0f);
    }
    gMeteors[0].moveRight();
    gMeteors[1].moveDown();

    gFuel        = MAX_FUEL;
    gGameState   = PLAYING;
    gMissionTime = 0.0f;
    cutThrusters();
}

bool initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Project 3");
    InitAudioDevice();

    createShip();
    createLevel();
    gBackground = LoadTexture(BACKGROUND_FP);
    gWinSound   = LoadSound(WIN_SOUND_FP);
    gLoseSound  = LoadSound(LOSE_SOUND_FP);

    bool hasHud = loadHud();
    if (!hasHud || !areTexturesLoaded())
    {
        LOG("Could not load the game's images. Run the game from the project-3 folder.");
        return false;
    }

    // Sound is a bonus: warn, but keep playing without it.
    if (gWinSound.frameCount == 0 || gLoseSound.frameCount == 0)
        LOG("Could not load the sound effects; playing without sound.");

    resetMission();
    SetTargetFPS(FPS);

    // Start the clock now so loading isn't simulated as one long frame.
    gPreviousTicks = static_cast<float>(GetTime());
    return true;
}

/* ------------------------------ INPUT ------------------------------ */

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose())
    {
        gAppStatus = TERMINATED;
        return;
    }

    if (IsKeyPressed(KEY_R))
    {
        resetMission();
        return;
    }

    if (gGameState != PLAYING) return;

    // Thrusters only respond while there is fuel left.
    bool hasFuel = gFuel > 0.0f;

    gIsMainEngineOn    = hasFuel && (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) ||
                                     IsKeyDown(KEY_SPACE));
    gIsLeftThrusterOn  = hasFuel && (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT));
    gIsRightThrusterOn = hasFuel && (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT));

    // Every new key press costs a little fuel on top of the continuous burn.
    int ignitions = (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) ||
                     IsKeyPressed(KEY_SPACE)) +
                    (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) +
                    (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT));

    if (hasFuel) gFuel -= ignitions * IGNITION_COST;
    if (gFuel < 0.0f) gFuel = 0.0f;
}

/* ------------------------------ UPDATE ----------------------------- */

// The keys never move the ship directly: they only change its acceleration.
Vector2 thrustAcceleration()
{
    Vector2 acceleration = { 0.0f, GRAVITY };

    if (gIsMainEngineOn)    acceleration.y -= MAIN_THRUST;
    if (gIsLeftThrusterOn)  acceleration.x -= SIDE_THRUST;
    if (gIsRightThrusterOn) acceleration.x += SIDE_THRUST;

    // Drag slows the sideways drift once the side thrusters let go.
    acceleration.x -= gShip->getVelocity().x * HORIZONTAL_DRAG;

    return acceleration;
}

AnimationState shipAnimation()
{
    if (gIsMainEngineOn)                          return THRUST_UP;
    if (gIsLeftThrusterOn && !gIsRightThrusterOn) return THRUST_LEFT;
    if (gIsRightThrusterOn && !gIsLeftThrusterOn) return THRUST_RIGHT;
    return IDLE;
}

void burnFuel(float deltaTime)
{
    float burnRate = 0.0f;

    if (gIsMainEngineOn)    burnRate += MAIN_BURN_RATE;
    if (gIsLeftThrusterOn)  burnRate += SIDE_BURN_RATE;
    if (gIsRightThrusterOn) burnRate += SIDE_BURN_RATE;

    gFuel -= burnRate * deltaTime;
    if (gFuel < 0.0f) gFuel = 0.0f;
}

void patrolHorizontally(Entity *entity, float minX, float maxX)
{
    if      (entity->getPosition().x <= minX) entity->moveRight();
    else if (entity->getPosition().x >= maxX) entity->moveLeft();
}

void patrolVertically(Entity *entity, float minY, float maxY)
{
    if      (entity->getPosition().y <= minY) entity->moveDown();
    else if (entity->getPosition().y >= maxY) entity->moveUp();
}

void moveObstacles(float deltaTime)
{
    patrolHorizontally(&gPads[BARGE_INDEX], BARGE_MIN_X, BARGE_MAX_X);
    patrolHorizontally(&gMeteors[0], METEOR_A_MIN_X, METEOR_A_MAX_X);
    patrolVertically(&gMeteors[1], METEOR_B_MIN_Y, METEOR_B_MAX_Y);

    for (int i = 0; i < PAD_COUNT; i++) gPads[i].update(deltaTime);

    for (int i = 0; i < METEOR_COUNT; i++)
    {
        gMeteors[i].update(deltaTime);
        gMeteors[i].setAngle(gMeteors[i].getAngle() + METEOR_SPIN * deltaTime);
    }
}

bool isSuccessfulLanding(MissionOutcome outcome)
{
    return outcome == LANDED_ON_PAD || outcome == LANDED_ON_BARGE;
}

void endMission(MissionOutcome outcome)
{
    bool isSuccess = isSuccessfulLanding(outcome);

    gOutcome   = outcome;
    gGameState = isSuccess ? MISSION_ACCOMPLISHED : MISSION_FAILED;
    cutThrusters();

    // Freeze the ship where it is; only its animation keeps playing.
    gShip->setVelocity({ 0.0f, 0.0f });
    gShip->setAcceleration({ 0.0f, 0.0f });
    gShip->setAnimation(isSuccess ? IDLE : CRASHED);

    PlaySound(isSuccess ? gWinSound : gLoseSound);
}

MissionOutcome judgeLanding(Entity *pad, bool isBarge)
{
    Vector2 shipPosition = gShip->getPosition(),
            shipCollider = gShip->getColliderDimensions(),
            padPosition  = pad->getPosition(),
            padCollider  = pad->getColliderDimensions();

    float shipBottom = shipPosition.y + shipCollider.y / 2.0f;
    float padTop     = padPosition.y  - padCollider.y  / 2.0f;

    // Sunk deep into the pad means we came in from the side or from below.
    if (shipBottom - padTop > LANDING_TOLERANCE) return CLIPPED_PAD_SIDE;

    // The middle of the ship has to be over the pad.
    if (fabs(shipPosition.x - padPosition.x) > padCollider.x / 2.0f)
        return MISSED_PAD_MIDDLE;

    // Speeds are measured relative to the pad, so the barge's motion counts.
    float downSpeed = gShip->getVelocity().y - pad->getVelocity().y;
    float sideSpeed = fabs(gShip->getVelocity().x - pad->getVelocity().x);

    if (downSpeed > SAFE_DOWN_SPEED) return TOO_FAST_DOWN;
    if (sideSpeed > SAFE_SIDE_SPEED) return TOO_FAST_SIDEWAYS;

    return isBarge ? LANDED_ON_BARGE : LANDED_ON_PAD;
}

// Sits the ship exactly on top of the pad after a good landing.
void settleOnPad(Entity *pad)
{
    float padTop = pad->getPosition().y - pad->getColliderDimensions().y / 2.0f;
    gShip->setPosition({ gShip->getPosition().x,
                         padTop - gShip->getColliderDimensions().y / 2.0f });
}

bool isTouchingAny(Entity *entities, int count)
{
    for (int i = 0; i < count; i++)
        if (gShip->isColliding(&entities[i])) return true;

    return false;
}

bool isOutOfBounds()
{
    Vector2 position = gShip->getPosition();
    Vector2 halfSize = { gShip->getColliderDimensions().x / 2.0f,
                         gShip->getColliderDimensions().y / 2.0f };

    return position.x + halfSize.x < 0.0f || position.x - halfSize.x > SCREEN_WIDTH ||
           position.y + halfSize.y < 0.0f;
}

void checkMissionOutcome()
{
    // Pads first: a ship on a pad may also be brushing the ice beside it.
    for (int i = 0; i < PAD_COUNT; i++)
    {
        if (!gShip->isColliding(&gPads[i])) continue;

        MissionOutcome outcome = judgeLanding(&gPads[i], i == BARGE_INDEX);
        if (isSuccessfulLanding(outcome)) settleOnPad(&gPads[i]);
        endMission(outcome);
        return;
    }

    if      (isTouchingAny(gTerrain, TERRAIN_COUNT)) endMission(CRASHED_INTO_ICE);
    else if (isTouchingAny(gMeteors, METEOR_COUNT))  endMission(HIT_METEOR);
    else if (isOutOfBounds())                        endMission(DRIFTED_AWAY);
}

void stepMission(float deltaTime)
{
    // Once the mission is over the world freezes; only the ship animates.
    if (gGameState != PLAYING)
    {
        gShip->update(deltaTime);
        return;
    }

    gMissionTime += deltaTime;
    if (gFuel <= 0.0f) cutThrusters(); // fuel can run out between key reads

    gShip->setAcceleration(thrustAcceleration());
    gShip->setAnimation(shipAnimation());
    burnFuel(deltaTime);
    gShip->update(deltaTime);

    moveObstacles(deltaTime);
    checkMissionOutcome();
}

void update()
{
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    if (deltaTime > MAX_FRAME_TIME) deltaTime = MAX_FRAME_TIME;

    deltaTime += gTimeAccumulator; // fold in any leftover from last frame

    while (deltaTime >= FIXED_TIMESTEP) // consume as many full steps as possible
    {
        stepMission(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }

    gTimeAccumulator = deltaTime; // save the remainder for next frame
}

/* ------------------------------ RENDER ----------------------------- */

void drawBackground()
{
    float     tileSize    = static_cast<float>(gBackground.width);
    Rectangle textureArea = { 0.0f, 0.0f, tileSize,
                              static_cast<float>(gBackground.height) };

    for (float x = 0.0f; x < SCREEN_WIDTH; x += tileSize)
        for (float y = 0.0f; y < SCREEN_HEIGHT; y += tileSize)
            DrawTexturePro(gBackground, textureArea, { x, y, tileSize, tileSize },
                           { 0.0f, 0.0f }, 0.0f, WHITE);
}

// Terrain and pads are plain rectangles the size of their colliders.
Rectangle areaOf(Entity *block)
{
    Vector2 position = block->getPosition();
    Vector2 size     = block->getScale();

    return { position.x - size.x / 2.0f, position.y - size.y / 2.0f, size.x, size.y };
}

void drawRectangleArea(Rectangle area, Color colour)
{
    DrawRectangle(static_cast<int>(area.x), static_cast<int>(area.y),
                  static_cast<int>(area.width), static_cast<int>(area.height), colour);
}

void drawGround()
{
    for (int i = 0; i < TERRAIN_COUNT; i++)
    {
        Rectangle area = areaOf(&gTerrain[i]);
        drawRectangleArea(area, ColorFromHex(ICE_COLOUR));
        drawRectangleArea({ area.x, area.y, area.width, FROST_THICKNESS },
                          ColorFromHex(FROST_COLOUR));
    }

    for (int i = 0; i < PAD_COUNT; i++)
    {
        Rectangle area = areaOf(&gPads[i]);

        if (i == BARGE_INDEX)
        {
            // Grey hull with a green landing deck on top
            drawRectangleArea(area, ColorFromHex(HULL_COLOUR));
            drawRectangleArea({ area.x, area.y, area.width, BARGE_DECK },
                              ColorFromHex(PAD_COLOUR));
        }
        else drawRectangleArea(area, ColorFromHex(PAD_COLOUR));
    }
}

HudReadout makeReadout()
{
    Vector2 velocity = gShip->getVelocity();

    return {
        gFuel, MAX_FUEL,
        velocity.y, static_cast<float>(fabs(velocity.x)),
        SAFE_DOWN_SPEED, SAFE_SIDE_SPEED
    };
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    drawBackground();
    drawGround();
    for (int i = 0; i < METEOR_COUNT; i++) gMeteors[i].render();
    gShip->render();

    drawHud(makeReadout());
    drawControls();

    if (gGameState == PLAYING && gMissionTime < BRIEF_DURATION)
    {
        float opacity = (BRIEF_DURATION - gMissionTime) / BRIEF_FADE;
        drawBrief(opacity > 1.0f ? 1.0f : opacity);
    }

    if (gGameState != PLAYING)
        drawResult(gGameState == MISSION_ACCOMPLISHED, gOutcome);

    EndDrawing();
}

void shutdown()
{
    // Unload textures while the window (GL context) still exists.
    delete gShip;
    delete[] gTerrain;
    delete[] gPads;
    delete[] gMeteors;

    if (gBackground.id != 0) UnloadTexture(gBackground);
    unloadHud();

    UnloadSound(gWinSound);
    UnloadSound(gLoseSound);
    CloseAudioDevice();

    CloseWindow();
}

int main(void)
{
    if (!initialise())
    {
        shutdown();
        return 1;
    }

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
