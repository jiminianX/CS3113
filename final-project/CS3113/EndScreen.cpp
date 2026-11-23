/**
* Author: Joseph Jiminian
* Assignment: seven star rising
* Date due: n/a, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "EndScreen.h"

EndScreen::EndScreen()
    : Scene{ {}, nullptr }, mTitle{ "" }, mMessage{ "" }, mTitleColour{ WHITE } {}

EndScreen::EndScreen(Vector2 origin, const char *bgHexCode, const char *title,
                     const char *message, Color titleColour)
    : Scene{ origin, bgHexCode }, mTitle{ title }, mMessage{ message },
      mTitleColour{ titleColour }
{}

EndScreen::~EndScreen() { shutdown(); }

void EndScreen::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
    mGameState.hero        = nullptr;
    mGameState.map         = nullptr;
    mTime                  = 0.0f;
}

void EndScreen::processInput()
{
    if (IsKeyPressed(KEY_ENTER)) mGameState.nextSceneID = MENU;
}

void EndScreen::update(float deltaTime)
{
    mTime += deltaTime;
}

void EndScreen::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));
}

void EndScreen::renderUI()
{
    DrawText(mTitle,   200, 220, 72, mTitleColour);
    DrawText(mMessage, 200, 320, 24, RAYWHITE);

    if (fmodf(mTime * BLINK_RATE, 1.0f) < 0.65f)
        DrawText("Press ENTER to return to the menu", 200, 420, 24, LIGHTGRAY);
}

void EndScreen::shutdown() {}
