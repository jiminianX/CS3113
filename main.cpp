/**
* Author: Joseph Jiminian
* Assignment: Simple 2d Scene
* Date due: 10/05/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/cs3113.h"

enum ScenePhase { DRIBBLING, SHOOTING, CELEBRATING };

// Global Constants
constexpr int SCREEN_WIDTH  = 1200,
              SCREEN_HEIGHT = 675,
              FPS           = 60;

constexpr Vector2 SCREEN_CENTRE = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
constexpr Vector2 SCREEN_SIZE   = { static_cast<float>(SCREEN_WIDTH),
                                    static_cast<float>(SCREEN_HEIGHT) };

// Phase lengths (seconds)
constexpr float DRIBBLE_DURATION     = 6.0f,
                SHOT_DURATION        = 1.6f,
                CELEBRATION_DURATION = 5.0f;

// Object sizes (pixels)
constexpr Vector2 LEBRON_DRIBBLE_SIZE = { 160.0f, 255.0f },
                  LEBRON_SHOOT_SIZE   = { 130.0f, 276.0f },
                  DEFENDER_SIZE       = { 250.0f, 205.0f },
                  BALL_SIZE           = {  60.0f,  60.0f };

// LeBron oscillates diagonally toward the basket and back
constexpr Vector2 DRIVE_CENTRE    = { 430.0f, 470.0f },
                  DRIVE_AMPLITUDE = { 220.0f, -70.0f },
                  HAND_OFFSET     = {  40.0f,   5.0f };
constexpr float   DRIVE_SPEED         = 2.0f,
                  DRIBBLE_HEIGHT      = 110.0f,
                  DRIBBLE_SPEED       = 3.0f,
                  DRIBBLE_SPIN_SPEED  = 360.0f,
                  DRIBBLE_SHRINK      = 0.25f;  // ball is 25% smaller at the floor

// Defender stays between LeBron and the rim
constexpr float GUARD_DISTANCE = 150.0f,
                SHUFFLE_RADIUS = 18.0f,
                SHUFFLE_SPEED  = 6.0f;

// the ball arcs to the rim, spinning and shrinking
constexpr Vector2 RIM_POSITION   = { 802.0f, 222.0f },  // rim in court.png
                  RELEASE_OFFSET = {  41.0f, -128.0f }; // ball in shooting.png
constexpr float   SHOT_ARC_HEIGHT = 260.0f,
                  SHOT_SPIN_SPEED = 100.0f,
                  BALL_END_SCALE  = 0.55f,
                  JUMP_HEIGHT     = 60.0f,
                  JUMP_DRIFT      = 20.0f,
                  CONTEST_HEIGHT  = 40.0f;

// Celebration: the photo pulses like a heartbeat
constexpr float PULSE_AMPLITUDE = 0.05f,
                PULSE_SPEED     = 8.0f;

constexpr char COURT_FP[]       = "assets/game/court.png";
constexpr char CELEBRATING_FP[] = "assets/game/celebrating.png";
constexpr char DRIBBLING_FP[] = "assets/game/dribbling.png";
constexpr char SHOOTING_FP[]  = "assets/game/shooting.png";
constexpr char DEFENDER_FP[]  = "assets/game/defender.png";
constexpr char BALL_FP[]      = "assets/game/ball.png";

// Global Variables
AppStatus  gAppStatus     = RUNNING;
ScenePhase gPhase         = DRIBBLING;
float      gPreviousTicks = 0.0f,
           gPhaseTime     = 0.0f,
           gBallAngle     = 0.0f,
           gBallScale     = 1.0f,
           gCelebrateScale = 1.0f;

Vector2 gLebronPosition   = DRIVE_CENTRE,
        gBallPosition     = DRIVE_CENTRE,
        gDefenderPosition = DRIVE_CENTRE,
        gLebronAnchor     = DRIVE_CENTRE,  // where the jump starts
        gDefenderAnchor   = DRIVE_CENTRE,
        gShotStart        = DRIVE_CENTRE;

Texture2D gCourtTexture,
          gCelebratingTexture,
          gDribblingTexture,
          gShootingTexture,
          gDefenderTexture,
          gBallTexture;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

// Helpers
void switchPhase(ScenePhase nextPhase)
{
    gPhase     = nextPhase;
    gPhaseTime = 0.0f;
}

void drawTexture(Texture2D texture, Vector2 position, Vector2 size,
                 float angle, Color tint)
{
    Rectangle textureArea = {
        0.0f, 0.0f,
        static_cast<float>(texture.width),
        static_cast<float>(texture.height)
    };

    Rectangle destinationArea = { position.x, position.y, size.x, size.y };

    Vector2 objectOrigin = { size.x / 2.0f, size.y / 2.0f };

    DrawTexturePro(texture, textureArea, destinationArea, objectOrigin, angle, tint);
}

// Phase updates
void updateDribbling(float deltaTime)
{
    // LeBron oscillates diagonally, driving toward the basket and back out
    float drive = sin(DRIVE_SPEED * gPhaseTime);
    gLebronPosition = {
        DRIVE_CENTRE.x + DRIVE_AMPLITUDE.x * drive,
        DRIVE_CENTRE.y + DRIVE_AMPLITUDE.y * drive
    };

    // Ball bouncing to the floor and back
    float bounce = fabs(sin(DRIBBLE_SPEED * gPhaseTime));
    gBallPosition = {
        gLebronPosition.x + HAND_OFFSET.x,
        gLebronPosition.y + HAND_OFFSET.y + DRIBBLE_HEIGHT * bounce
    };
    gBallAngle += DRIBBLE_SPIN_SPEED * deltaTime;
    gBallScale  = 1.0f - DRIBBLE_SHRINK * bounce;  // full size at his hand

    // Defender is relative to LeBron and always on the rim side of him
    Vector2 towardRim = {
        RIM_POSITION.x - gLebronPosition.x,
        RIM_POSITION.y - gLebronPosition.y
    };
    Normalise(&towardRim);

    float shuffle = SHUFFLE_SPEED * gPhaseTime;
    gDefenderPosition = {
        gLebronPosition.x + towardRim.x * GUARD_DISTANCE + SHUFFLE_RADIUS * cos(shuffle),
        gLebronPosition.y + towardRim.y * GUARD_DISTANCE + SHUFFLE_RADIUS * sin(shuffle)
    };

    if (gPhaseTime < DRIBBLE_DURATION) return;

    gLebronAnchor   = gLebronPosition;
    gDefenderAnchor = gDefenderPosition;
    gShotStart      = { gLebronPosition.x + RELEASE_OFFSET.x,
                        gLebronPosition.y + RELEASE_OFFSET.y };
    switchPhase(SHOOTING);
}

void updateShooting(float deltaTime)
{
    float progress = gPhaseTime / SHOT_DURATION;
    if (progress > 1.0f) progress = 1.0f;

    // LeBron rises and drifts toward the basket and the defender jumps
    float jump = sin(PI * progress);
    gLebronPosition = {
        gLebronAnchor.x + JUMP_DRIFT * progress,
        gLebronAnchor.y - JUMP_HEIGHT * jump
    };
    gDefenderPosition = {
        gDefenderAnchor.x,
        gDefenderAnchor.y - CONTEST_HEIGHT * jump
    };

    // Ball spinning and shrinking with distance towards rim
    float arc = 4.0f * progress * (1.0f - progress);
    gBallPosition = {
        gShotStart.x + (RIM_POSITION.x - gShotStart.x) * progress,
        gShotStart.y + (RIM_POSITION.y - gShotStart.y) * progress - SHOT_ARC_HEIGHT * arc
    };
    gBallAngle += SHOT_SPIN_SPEED * deltaTime;
    gBallScale  = 1.0f - (1.0f - BALL_END_SCALE) * progress;

    if (progress < 1.0f) return;

    gDefenderAnchor = gDefenderPosition;
    switchPhase(CELEBRATING);
}

void updateCelebrating()
{
    // Photo pulses like a heartbeat, never shrinking smaller than the screen
    gCelebrateScale = 1.0f + PULSE_AMPLITUDE + PULSE_AMPLITUDE * sin(PULSE_SPEED * gPhaseTime);

    if (gPhaseTime < CELEBRATION_DURATION) return;

    switchPhase(DRIBBLING);
}

// Function Definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LeBron Takes Over");

    gCourtTexture     = LoadTexture(COURT_FP);
    gCelebratingTexture = LoadTexture(CELEBRATING_FP);
    gDribblingTexture = LoadTexture(DRIBBLING_FP);
    gShootingTexture  = LoadTexture(SHOOTING_FP);
    gDefenderTexture  = LoadTexture(DEFENDER_FP);
    gBallTexture      = LoadTexture(BALL_FP);

    SetTargetFPS(FPS);
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    // Delta time
    float ticks = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    gPhaseTime += deltaTime;

    if      (gPhase == DRIBBLING) updateDribbling(deltaTime);
    else if (gPhase == SHOOTING)  updateShooting(deltaTime);
    else                          updateCelebrating();
}

void render()
{
    BeginDrawing();

    ClearBackground(BLACK);

    // Celebration: only the pulsing photo
    if (gPhase == CELEBRATING)
    {
        Vector2 celebrateSize = { SCREEN_SIZE.x * gCelebrateScale,
                                  SCREEN_SIZE.y * gCelebrateScale };
        drawTexture(gCelebratingTexture, SCREEN_CENTRE, celebrateSize, 0.0f, WHITE);
        EndDrawing();
        return;
    }

    drawTexture(gCourtTexture, SCREEN_CENTRE, SCREEN_SIZE, 0.0f, WHITE);

    drawTexture(gDefenderTexture, gDefenderPosition, DEFENDER_SIZE, 0.0f, WHITE);

    bool isDribbling = gPhase == DRIBBLING;
    Texture2D lebronTexture = isDribbling ? gDribblingTexture : gShootingTexture;
    Vector2   lebronSize    = isDribbling ? LEBRON_DRIBBLE_SIZE : LEBRON_SHOOT_SIZE;
    drawTexture(lebronTexture, gLebronPosition, lebronSize, 0.0f, WHITE);

    Vector2 ballSize = { BALL_SIZE.x * gBallScale, BALL_SIZE.y * gBallScale };
    drawTexture(gBallTexture, gBallPosition, ballSize, gBallAngle, WHITE);

    EndDrawing();
}

void shutdown()
{
    UnloadTexture(gCourtTexture);
    UnloadTexture(gCelebratingTexture);
    UnloadTexture(gDribblingTexture);
    UnloadTexture(gShootingTexture);
    UnloadTexture(gDefenderTexture);
    UnloadTexture(gBallTexture);

    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
