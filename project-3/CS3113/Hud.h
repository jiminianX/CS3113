/**
* Author: Joseph Jiminian
* Assignment: Rise of the AI
* Date due: 11/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#ifndef HUD_H
#define HUD_H

#include "cs3113.h"

constexpr int SCREEN_WIDTH  = 1280,
              SCREEN_HEIGHT = 720;

// Why the mission ended. Each one has its own message image.
enum MissionOutcome
{
    LANDED_ON_PAD,
    LANDED_ON_BARGE,
    CRASHED_INTO_ICE,
    HIT_METEOR,
    TOO_FAST_DOWN,
    TOO_FAST_SIDEWAYS,
    MISSED_PAD_MIDDLE,
    CLIPPED_PAD_SIDE,
    DRIFTED_AWAY,
    OUTCOME_COUNT
};

struct HudReadout
{
    float fuel;
    float maxFuel;
    float downSpeed;     // px/s, positive while falling
    float sideSpeed;     // px/s, always positive
    float safeDownSpeed; // px/s
    float safeSideSpeed; // px/s
};

// Every word on screen is an image in assets/text/ (no DrawText).
// loadHud() returns false if any image is missing.
bool loadHud();
void unloadHud();

void drawHud(HudReadout readout);
void drawControls();
void drawBrief(float opacity);
void drawResult(bool isSuccess, MissionOutcome outcome);

#endif // HUD_H
