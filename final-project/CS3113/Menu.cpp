/**
* Author: Joseph Jiminian
* Assignment: seven star rising
* Date due: n/a, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Menu.h"

Menu::Menu()                                      : Scene{ {},     nullptr   } {}
Menu::Menu(Vector2 origin, const char *bgHexCode) : Scene{ origin, bgHexCode } {}

Menu::~Menu() { shutdown(); }

void Menu::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
    mGameState.hero        = nullptr;
    mGameState.map         = nullptr;
    mTime                  = 0.0f;

    mOrbTexture = LoadTexture("assets/sprites/orb.png");
}

void Menu::processInput()
{
    if (IsKeyPressed(KEY_ENTER)) mGameState.nextSceneID = VALLEY;
}

void Menu::update(float deltaTime)
{
    mTime += deltaTime;
}

void Menu::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));
}

void Menu::renderUI()
{
    // The seven star orbs circling the title.
    for (int i = 0; i < ORB_COUNT; i++)
    {
        float angle = mTime * RING_SPEED + i * 2.0f * PI / ORB_COUNT;

        Rectangle textureArea = {
            0.0f, 0.0f,
            static_cast<float>(mOrbTexture.width),
            static_cast<float>(mOrbTexture.height)
        };
        Rectangle destinationArea = {
            mOrigin.x + cosf(angle) * RING_RADIUS_X,
            RING_CENTRE_Y + sinf(angle) * RING_RADIUS_Y,
            ORB_SIZE, ORB_SIZE
        };

        DrawTexturePro(mOrbTexture, textureArea, destinationArea,
                       { ORB_SIZE / 2.0f, ORB_SIZE / 2.0f }, 0.0f, WHITE);
    }

    DrawText("SEVEN STAR RISING", 254, 154, 72, BLACK); // shadow
    DrawText("SEVEN STAR RISING", 250, 150, 72, GOLD);
    DrawText("Find the seven star orbs before the warlord makes his wish.",
             277, 285, 24, RAYWHITE);

    DrawText("WASD / Arrow keys  -  move",                                    252, 360, 20, LIGHTGRAY);
    DrawText("J or Space  -  fire a ki blast (costs ki)",                     252, 394, 20, LIGHTGRAY);
    DrawText("Hold K  -  charge ki (you can't move, and hits hurt twice as much)", 252, 428, 20, LIGHTGRAY);
    DrawText("Goal: find the valley's 3 star orbs to open the stone gate",    252, 462, 20, LIGHTGRAY);
    DrawText("You have 3 lives of 5 hearts each.    Q  -  quit",              252, 496, 20, LIGHTGRAY);

    // Blinking prompt
    if (fmodf(mTime * BLINK_RATE, 1.0f) < 0.65f)
        DrawText("Press ENTER to start", 485, 630, 28, GOLD);
}

void Menu::shutdown()
{
    UnloadTexture(mOrbTexture);
    mOrbTexture = {};
}
