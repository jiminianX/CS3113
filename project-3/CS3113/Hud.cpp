/**
* Author: Joseph Jiminian
* Assignment: Rise of the AI
* Date due: 11/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Hud.h"

// Panel layout (pixels)
constexpr int PANEL_X      = 16,
              PANEL_Y      = 16,
              PANEL_WIDTH  = 360,
              PANEL_HEIGHT = 112,
              LABEL_X      = PANEL_X + 14,
              BAR_X        = PANEL_X + 150,
              BAR_WIDTH    = 150,
              BAR_HEIGHT   = 14,
              ROW_HEIGHT   = 32,
              FIRST_ROW_Y  = PANEL_Y + 14,
              NUMERAL_SIZE = 19,
              DIGIT_GAP    = 2,
              TEXT_COUNT   = 12,
              DIGIT_COUNT  = 10;

constexpr float LOW_FUEL_FRACTION  = 0.25f,
                HALF_FUEL_FRACTION = 0.5f,
                BLINKS_PER_SECOND  = 2.0f,
                SPEED_GAUGE_RANGE  = 2.0f; // a full gauge is 2x the safe speed

constexpr char PANEL_COLOUR[]  = "#0B1020",
               TRACK_COLOUR[]  = "#1F2A3D",
               DIM_COLOUR[]    = "#8BA0B8",
               SAFE_COLOUR[]   = "#4DFF88",
               WARN_COLOUR[]   = "#F5C518",
               DANGER_COLOUR[] = "#FF5A5A";

// Images rendered from the Kenney font (see README)
enum TextImage
{
    TITLE, GOAL, WARNING, CONTROLS, ACCOMPLISHED, FAILED, RETRY,
    FUEL_LABEL, DOWN_LABEL, SIDE_LABEL, LOW_FUEL, OUT_OF_FUEL
};

const char *TEXT_FILES[TEXT_COUNT] = {
    "assets/text/title.png",        "assets/text/brief_goal.png",
    "assets/text/brief_warning.png", "assets/text/controls.png",
    "assets/text/accomplished.png", "assets/text/failed.png",
    "assets/text/retry.png",        "assets/text/label_fuel.png",
    "assets/text/label_down.png",   "assets/text/label_side.png",
    "assets/text/low_fuel.png",     "assets/text/out_of_fuel.png"
};

// Same order as the MissionOutcome enum
const char *REASON_FILES[OUTCOME_COUNT] = {
    "assets/text/reason_pad.png",      "assets/text/reason_barge.png",
    "assets/text/reason_ice.png",      "assets/text/reason_meteor.png",
    "assets/text/reason_too_hard.png", "assets/text/reason_sideways.png",
    "assets/text/reason_off_pad.png",  "assets/text/reason_pad_side.png",
    "assets/text/reason_drifted.png"
};

const char *DIGIT_FILES[DIGIT_COUNT] = {
    "assets/kenney_space-shooter-remastered/PNG/UI/numeral0.png",
    "assets/kenney_space-shooter-remastered/PNG/UI/numeral1.png",
    "assets/kenney_space-shooter-remastered/PNG/UI/numeral2.png",
    "assets/kenney_space-shooter-remastered/PNG/UI/numeral3.png",
    "assets/kenney_space-shooter-remastered/PNG/UI/numeral4.png",
    "assets/kenney_space-shooter-remastered/PNG/UI/numeral5.png",
    "assets/kenney_space-shooter-remastered/PNG/UI/numeral6.png",
    "assets/kenney_space-shooter-remastered/PNG/UI/numeral7.png",
    "assets/kenney_space-shooter-remastered/PNG/UI/numeral8.png",
    "assets/kenney_space-shooter-remastered/PNG/UI/numeral9.png"
};

Texture2D gTextImages[TEXT_COUNT];
Texture2D gReasonImages[OUTCOME_COUNT];
Texture2D gDigitImages[DIGIT_COUNT];

/* ------------------------------ LOADING ---------------------------- */

bool loadImages(Texture2D *textures, const char **files, int count)
{
    bool isLoaded = true;

    for (int i = 0; i < count; i++)
    {
        textures[i] = LoadTexture(files[i]);

        if (textures[i].id == 0)
        {
            LOG("Missing image: " << files[i]);
            isLoaded = false;
        }
    }

    return isLoaded;
}

void unloadImages(Texture2D *textures, int count)
{
    for (int i = 0; i < count; i++)
        if (textures[i].id != 0) UnloadTexture(textures[i]);
}

bool loadHud()
{
    bool hasText    = loadImages(gTextImages,   TEXT_FILES,   TEXT_COUNT);
    bool hasReasons = loadImages(gReasonImages, REASON_FILES, OUTCOME_COUNT);
    bool hasDigits  = loadImages(gDigitImages,  DIGIT_FILES,  DIGIT_COUNT);

    return hasText && hasReasons && hasDigits;
}

void unloadHud()
{
    unloadImages(gTextImages,   TEXT_COUNT);
    unloadImages(gReasonImages, OUTCOME_COUNT);
    unloadImages(gDigitImages,  DIGIT_COUNT);
}

/* ------------------------------ HELPERS ---------------------------- */

// Draws a whole image at its real size with its top-left corner at (x, y).
void drawImage(Texture2D image, float x, float y, Color tint)
{
    Rectangle textureArea     = { 0.0f, 0.0f, static_cast<float>(image.width),
                                  static_cast<float>(image.height) };
    Rectangle destinationArea = { x, y, static_cast<float>(image.width),
                                  static_cast<float>(image.height) };

    DrawTexturePro(image, textureArea, destinationArea, { 0.0f, 0.0f }, 0.0f, tint);
}

void drawCentredImage(Texture2D image, float y, Color tint)
{
    drawImage(image, (SCREEN_WIDTH - image.width) / 2.0f, y, tint);
}

bool isBlinkOn()
{
    return fmod(GetTime() * BLINKS_PER_SECOND, 1.0) < 0.5;
}

// Draws a whole number (0-999) with the Kenney numeral images.
void drawNumber(int value, float x, float y)
{
    int digitCount = value >= 100 ? 3 : (value >= 10 ? 2 : 1);
    int divisor    = digitCount == 3 ? 100 : (digitCount == 2 ? 10 : 1);

    for (int i = 0; i < digitCount; i++)
    {
        int digit = (value / divisor) % 10;
        drawImage(gDigitImages[digit], x + i * (NUMERAL_SIZE + DIGIT_GAP), y, WHITE);
        divisor /= 10;
    }
}

// A track, a coloured fill, and an outline.
void drawBar(int x, int y, float fraction, Color fillColour)
{
    if (fraction < 0.0f) fraction = 0.0f;
    if (fraction > 1.0f) fraction = 1.0f;

    DrawRectangle(x, y, BAR_WIDTH, BAR_HEIGHT, ColorFromHex(TRACK_COLOUR));
    DrawRectangle(x, y, static_cast<int>(BAR_WIDTH * fraction), BAR_HEIGHT, fillColour);
    DrawRectangleLines(x, y, BAR_WIDTH, BAR_HEIGHT, ColorFromHex(DIM_COLOUR));
}

Color fuelColour(float fraction)
{
    if (fraction > HALF_FUEL_FRACTION) return ColorFromHex(SAFE_COLOUR);
    if (fraction > LOW_FUEL_FRACTION)  return ColorFromHex(WARN_COLOUR);
    return ColorFromHex(DANGER_COLOUR);
}

/* ------------------------------ HUD -------------------------------- */

void drawFuelRow(int y, float fuel, float maxFuel)
{
    float fraction = fuel / maxFuel;

    drawImage(gTextImages[FUEL_LABEL], LABEL_X, y, WHITE);
    drawBar(BAR_X, y + 4, fraction, fuelColour(fraction));
    drawNumber(static_cast<int>(ceil(fuel)), BAR_X + BAR_WIDTH + 12, y + 1);
}

// The bar is full at twice the safe speed; the white tick marks the limit.
// Green means slow enough to land, red means too fast.
void drawSpeedRow(int y, TextImage label, float speed, float safeSpeed)
{
    Color colour = speed <= safeSpeed ? ColorFromHex(SAFE_COLOUR)
                                      : ColorFromHex(DANGER_COLOUR);
    int   tickX  = BAR_X + static_cast<int>(BAR_WIDTH / SPEED_GAUGE_RANGE);

    drawImage(gTextImages[label], LABEL_X, y, WHITE);
    drawBar(BAR_X, y + 4, speed / (safeSpeed * SPEED_GAUGE_RANGE), colour);
    DrawRectangle(tickX - 1, y, 2, BAR_HEIGHT + 8, WHITE);
}

void drawFuelWarning(float fuel, float maxFuel)
{
    if (fuel / maxFuel >= LOW_FUEL_FRACTION || !isBlinkOn()) return;

    bool isEmpty = fuel <= 0.0f;
    drawImage(gTextImages[isEmpty ? OUT_OF_FUEL : LOW_FUEL],
              PANEL_X, PANEL_Y + PANEL_HEIGHT + 8,
              ColorFromHex(isEmpty ? DANGER_COLOUR : WARN_COLOUR));
}

void drawHud(HudReadout readout)
{
    DrawRectangle(PANEL_X, PANEL_Y, PANEL_WIDTH, PANEL_HEIGHT,
                  Fade(ColorFromHex(PANEL_COLOUR), 0.8f));

    drawFuelRow(FIRST_ROW_Y, readout.fuel, readout.maxFuel);
    drawSpeedRow(FIRST_ROW_Y + ROW_HEIGHT, DOWN_LABEL,
                 readout.downSpeed, readout.safeDownSpeed);
    drawSpeedRow(FIRST_ROW_Y + 2 * ROW_HEIGHT, SIDE_LABEL,
                 readout.sideSpeed, readout.safeSideSpeed);

    drawFuelWarning(readout.fuel, readout.maxFuel);
}

void drawControls()
{
    Texture2D controls = gTextImages[CONTROLS];
    float     y        = SCREEN_HEIGHT - controls.height - 6.0f;

    DrawRectangle(0, static_cast<int>(y) - 4, SCREEN_WIDTH,
                  controls.height + 10, Fade(BLACK, 0.5f));
    drawCentredImage(controls, y, WHITE);
}

void drawBrief(float opacity)
{
    drawCentredImage(gTextImages[TITLE],   210.0f, Fade(WHITE, opacity));
    drawCentredImage(gTextImages[GOAL],    275.0f, Fade(WHITE, opacity));
    drawCentredImage(gTextImages[WARNING], 310.0f,
                     Fade(ColorFromHex(DIM_COLOUR), opacity));
}

void drawResult(bool isSuccess, MissionOutcome outcome)
{
    constexpr int BOX_WIDTH  = 820,
                  BOX_HEIGHT = 210;

    int   left   = (SCREEN_WIDTH  - BOX_WIDTH)  / 2;
    int   top    = (SCREEN_HEIGHT - BOX_HEIGHT) / 2;
    Color accent = ColorFromHex(isSuccess ? SAFE_COLOUR : DANGER_COLOUR);

    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.45f));
    DrawRectangle(left, top, BOX_WIDTH, BOX_HEIGHT, ColorFromHex(PANEL_COLOUR));
    DrawRectangleLines(left,     top,     BOX_WIDTH,     BOX_HEIGHT,     accent);
    DrawRectangleLines(left + 1, top + 1, BOX_WIDTH - 2, BOX_HEIGHT - 2, accent);

    drawCentredImage(gTextImages[isSuccess ? ACCOMPLISHED : FAILED], top + 28.0f, accent);
    drawCentredImage(gReasonImages[outcome], top + 110.0f, WHITE);

    if (isBlinkOn())
        drawCentredImage(gTextImages[RETRY], top + 155.0f, ColorFromHex(DIM_COLOUR));
}
