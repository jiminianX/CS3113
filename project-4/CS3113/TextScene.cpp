/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#include "TextScene.h"

TextScene::TextScene(Vector2 origin, const TextSceneConfig &config,
                     const SoundBank *sounds)
    : Scene{ origin, config.bgColour, sounds }, mConfig{ config }
{}

TextScene::~TextScene() { shutdown(); }

void TextScene::initialise()
{
    shutdown();

    mGameState.nextSceneID = NO_SCENE;
    startMusic(mConfig.musicPath);
    if (mConfig.introSound != nullptr) playSound(*mConfig.introSound);

    mIsLoaded = true;
}

void TextScene::processInput()
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
    {
        resetLives();
        mGameState.nextSceneID = mConfig.nextScene;
    }
}

void TextScene::update(float deltaTime)
{
    (void) deltaTime;
    updateMusic();
}

void TextScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));
}

void TextScene::drawCentredText(const char *text, int y, int fontSize,
                                Color colour) const
{
    int x = (GetScreenWidth() - MeasureText(text, fontSize)) / 2;
    DrawText(text, x + 3, y + 3, fontSize, Fade(BLACK, 0.4f));
    DrawText(text, x, y, fontSize, colour);
}

void TextScene::renderUI()
{
    int centreY = GetScreenHeight() / 2;

    drawCentredText(mConfig.title, centreY - 110, TITLE_FONT_SIZE,
                    ColorFromHex(mConfig.titleColour));
    drawCentredText(mConfig.subtitle, centreY + 10, SUBTITLE_FONT_SIZE, RAYWHITE);

    if (mConfig.footer != nullptr)
        drawCentredText(mConfig.footer, GetScreenHeight() - 60,
                        FOOTER_FONT_SIZE, ColorFromHex("#DDDDDD"));
}

void TextScene::shutdown()
{
    if (!mIsLoaded) return;

    stopMusic();
    mIsLoaded = false;
}
