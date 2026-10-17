/**
* Author: Joseph Jiminian
* Assignment: Pong Clone
* Date due: 10/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#include "../CS3113/cs3113.h"

enum GameStatus { IN_PROGRESS, PLAYER_1_WINS, PLAYER_2_WINS };

// Global Constants
constexpr int SCREEN_WIDTH  = 800,
              SCREEN_HEIGHT = 450,
              FPS           = 60,
              MAX_BALLS     = 3,
              PADDLE_SPEED  = 300,
              BALL_SPEED    = 280;

constexpr float MUSIC_VOLUME = 0.33f;

constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  SCREEN_SIZE = { static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) },
                  PADDLE_SIZE = { 45.0f, 100.0f }, // keeps the racket image's 135:300 ratio
                  BALL_SIZE   = { 20.0f, 20.0f },
                  P1_INIT_POS = { 50.0f, ORIGIN.y },
                  P2_INIT_POS = { SCREEN_WIDTH - 50.0f, ORIGIN.y };

// Every ball exists from the start; only the first gBallCount are used
constexpr Vector2 BALL_INIT_POSITIONS[MAX_BALLS]  = {
                      { ORIGIN.x, ORIGIN.y },
                      { ORIGIN.x, ORIGIN.y - 120.0f },
                      { ORIGIN.x, ORIGIN.y + 120.0f }
                  },
                  BALL_INIT_MOVEMENTS[MAX_BALLS] = {
                      {  1.0f,  0.6f },
                      { -1.0f, -0.4f },
                      {  1.0f, -0.8f }
                  };

constexpr char COURT_FP[]       = "assets/tennis/court.png";
constexpr char RACKET_RED_FP[]  = "assets/tennis/racket_red.png";
constexpr char RACKET_BLUE_FP[] = "assets/tennis/racket_blue.png";
constexpr char BALL_FP[]        = "assets/tennis/ball.png";
constexpr char P1_WINS_FP[]     = "assets/tennis/p1_wins.png";
constexpr char P2_WINS_FP[]     = "assets/tennis/p2_wins.png";
constexpr char HIT_SFX_FP[]     = "assets/tennis/hit.ogg";
constexpr char CONFIRM_SFX_FP[] = "assets/tennis/score.ogg";
constexpr char MUSIC_FP[]       = "assets/tennis/music.mp3";

// Global Variables
AppStatus  gAppStatus      = RUNNING;
GameStatus gGameStatus     = IN_PROGRESS;
bool       gIsSinglePlayer = false;
int        gBallCount      = 1;
float      gPreviousTicks  = 0.0f;

Vector2 gP1Position = P1_INIT_POS,
        gP1Movement = { 0.0f, 0.0f },

        gP2Position = P2_INIT_POS,
        gP2Movement = { 0.0f, 0.0f },

        gBallPositions[MAX_BALLS],
        gBallMovements[MAX_BALLS],

        gP1WinsScale,
        gP2WinsScale;

Texture2D gCourtTexture;
Texture2D gP1Texture;
Texture2D gP2Texture;
Texture2D gBallTexture;
Texture2D gP1WinsTexture;
Texture2D gP2WinsTexture;

Music gBGM;
Sound gHitSound;
Sound gConfirmSound;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
bool isColliding(const Vector2 *positionA, const Vector2 *scaleA, const Vector2 *positionB, const Vector2 *scaleB);

// Function Definitions

bool isColliding(const Vector2 *positionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB)
{
    float xDistance = fabs(positionA->x - positionB->x) -
                      ((scaleA->x + scaleB->x) / 2.0f);
    float yDistance = fabs(positionA->y - positionB->y) -
                      ((scaleA->y + scaleB->y) / 2.0f);

    return xDistance < 0.0f && yDistance < 0.0f;
}

void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale)
{
    // Whole texture (UV coordinates)
    Rectangle textureArea = {
        0.0f, 0.0f,
        static_cast<float>(texture->width),
        static_cast<float>(texture->height)
    };

    // Destination rectangle – centred on position
    Rectangle destinationArea = {
        position->x,
        position->y,
        static_cast<float>(scale->x),
        static_cast<float>(scale->y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(scale->x) / 2.0f,
        static_cast<float>(scale->y) / 2.0f
    };

    DrawTexturePro(
        *texture,
        textureArea, destinationArea, originOffset,
        0.0f, WHITE
    );
}

void resetBall(int index)
{
    gBallPositions[index] = BALL_INIT_POSITIONS[index];
    gBallMovements[index] = BALL_INIT_MOVEMENTS[index];
    Normalise(&gBallMovements[index]);
}

void setBallCount(int count)
{
    for (int i = gBallCount; i < count; i++) resetBall(i);

    gBallCount = count;
    PlaySound(gConfirmSound);
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tennis Pong");
    InitAudioDevice();

    gCourtTexture  = LoadTexture(COURT_FP);
    gP1Texture     = LoadTexture(RACKET_RED_FP);
    gP2Texture     = LoadTexture(RACKET_BLUE_FP);
    gBallTexture   = LoadTexture(BALL_FP);
    gP1WinsTexture = LoadTexture(P1_WINS_FP);
    gP2WinsTexture = LoadTexture(P2_WINS_FP);

    // Winner banners are drawn at their original size
    gP1WinsScale = { static_cast<float>(gP1WinsTexture.width), static_cast<float>(gP1WinsTexture.height) };
    gP2WinsScale = { static_cast<float>(gP2WinsTexture.width), static_cast<float>(gP2WinsTexture.height) };

    gHitSound      = LoadSound(HIT_SFX_FP);
    gConfirmSound  = LoadSound(CONFIRM_SFX_FP);

    gBGM = LoadMusicStream(MUSIC_FP);
    SetMusicVolume(gBGM, MUSIC_VOLUME);
    PlayMusicStream(gBGM);

    for (int i = 0; i < MAX_BALLS; i++) resetBall(i);

    SetTargetFPS(FPS);

    // so the first frame's deltaTime doesn't include loading time
    gPreviousTicks = static_cast<float>(GetTime());
}

void processInput()
{
    // to close the game
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

    // once someone wins, the court is frozen
    if (gGameStatus != IN_PROGRESS) return;

    // player 1: W and S
    gP1Movement = { 0.0f, 0.0f };

    if      (IsKeyDown(KEY_W)) gP1Movement.y = -1; // up
    else if (IsKeyDown(KEY_S)) gP1Movement.y =  1; // down

    // T switches player 2 between a person and the computer
    if (IsKeyPressed(KEY_T))
    {
        gIsSinglePlayer = !gIsSinglePlayer;
        gP2Movement     = { 0.0f, gIsSinglePlayer ? 1.0f : 0.0f };
        PlaySound(gConfirmSound);
    }

    // player 2: arrow keys, ignored while the computer has control
    if (!gIsSinglePlayer)
    {
        gP2Movement = { 0.0f, 0.0f };

        if      (IsKeyDown(KEY_UP))   gP2Movement.y = -1; // up
        else if (IsKeyDown(KEY_DOWN)) gP2Movement.y =  1; // down
    }

    // 1, 2 and 3 choose how many balls are in play
    if      (IsKeyPressed(KEY_ONE))   setBallCount(1);
    else if (IsKeyPressed(KEY_TWO))   setBallCount(2);
    else if (IsKeyPressed(KEY_THREE)) setBallCount(3);
}

void updateAutoPaddle()
{
    float halfHeight = PADDLE_SIZE.y / 2.0f;

    if      (gP2Position.y <= halfHeight)                 gP2Movement.y =  1;
    else if (gP2Position.y >= SCREEN_HEIGHT - halfHeight) gP2Movement.y = -1;
}

void movePaddle(Vector2 *position, const Vector2 *movement, float deltaTime)
{
    float halfHeight = PADDLE_SIZE.y / 2.0f;

    position->y += PADDLE_SPEED * movement->y * deltaTime;

    if      (position->y < halfHeight)                 position->y = halfHeight;
    else if (position->y > SCREEN_HEIGHT - halfHeight) position->y = SCREEN_HEIGHT - halfHeight;
}

void bounceOffWalls(int index)
{
    float halfHeight = BALL_SIZE.y / 2.0f;

    if (gBallPositions[index].y < halfHeight)
    {
        gBallPositions[index].y = halfHeight;
        gBallMovements[index].y = fabs(gBallMovements[index].y);
    }
    else if (gBallPositions[index].y > SCREEN_HEIGHT - halfHeight)
    {
        gBallPositions[index].y = SCREEN_HEIGHT - halfHeight;
        gBallMovements[index].y = -fabs(gBallMovements[index].y);
    }
}

void bounceOffPaddle(int index, const Vector2 *paddlePosition, float direction)
{
    if (!isColliding(&gBallPositions[index], &BALL_SIZE, paddlePosition, &PADDLE_SIZE)) return;

    // already heading away from this racket
    if (gBallMovements[index].x * direction > 0.0f) return;

    gBallMovements[index].x = direction;
    gBallMovements[index].y = (gBallPositions[index].y - paddlePosition->y) /
                              (PADDLE_SIZE.y / 2.0f);
    Normalise(&gBallMovements[index]);

    PlaySound(gHitSound);
}

void endGame(GameStatus winner)
{
    gGameStatus = winner;

    StopMusicStream(gBGM);
}

void updateBall(int index, float deltaTime)
{
    gBallPositions[index].x += BALL_SPEED * gBallMovements[index].x * deltaTime;
    gBallPositions[index].y += BALL_SPEED * gBallMovements[index].y * deltaTime;

    bounceOffWalls(index);
    bounceOffPaddle(index, &gP1Position,  1.0f);
    bounceOffPaddle(index, &gP2Position, -1.0f);

    // a ball that gets past a racket loses the game for that player
    if (gGameStatus != IN_PROGRESS) return;

    if      (gBallPositions[index].x < 0.0f)         endGame(PLAYER_2_WINS);
    else if (gBallPositions[index].x > SCREEN_WIDTH) endGame(PLAYER_1_WINS);
}

void update()
{
    // delta time
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    UpdateMusicStream(gBGM);

    if (gGameStatus != IN_PROGRESS) return;

    if (gIsSinglePlayer) updateAutoPaddle();

    movePaddle(&gP1Position, &gP1Movement, deltaTime);
    movePaddle(&gP2Position, &gP2Movement, deltaTime);

    for (int i = 0; i < gBallCount; i++) updateBall(i, deltaTime);
}

void render()
{
    BeginDrawing();
    ClearBackground(BLACK);

    renderObject(&gCourtTexture, &ORIGIN, &SCREEN_SIZE);

    renderObject(&gP1Texture, &gP1Position, &PADDLE_SIZE);
    renderObject(&gP2Texture, &gP2Position, &PADDLE_SIZE);

    // balls not in play are neither updated nor drawn
    for (int i = 0; i < gBallCount; i++)
        renderObject(&gBallTexture, &gBallPositions[i], &BALL_SIZE);

    if      (gGameStatus == PLAYER_1_WINS) renderObject(&gP1WinsTexture, &ORIGIN, &gP1WinsScale);
    else if (gGameStatus == PLAYER_2_WINS) renderObject(&gP2WinsTexture, &ORIGIN, &gP2WinsScale);

    EndDrawing();
}

void shutdown()
{
    UnloadTexture(gCourtTexture);
    UnloadTexture(gP1Texture);
    UnloadTexture(gP2Texture);
    UnloadTexture(gBallTexture);
    UnloadTexture(gP1WinsTexture);
    UnloadTexture(gP2WinsTexture);

    UnloadSound(gHitSound);
    UnloadSound(gConfirmSound);
    UnloadMusicStream(gBGM);

    CloseAudioDevice();
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
