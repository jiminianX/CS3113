/**
* Author: Joseph Jiminian
* Assignment: seven star rising
* Date due: n/a, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "cs3113.h"

Color ColorFromHex(const char *hex)
{
    // Skip leading '#', if present
    if (hex[0] == '#') hex++;

    unsigned int r = 0,
                 g = 0,
                 b = 0,
                 a = 255;

    // 8-digit form: RRGGBBAA
    if (sscanf(hex, "%02x%02x%02x%02x", &r, &g, &b, &a) == 4)
    {
        return (Color) { (unsigned char) r,
                         (unsigned char) g,
                         (unsigned char) b,
                         (unsigned char) a };
    }

    // 6-digit form: RRGGBB
    if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) == 3)
    {
        return (Color) { (unsigned char) r,
                         (unsigned char) g,
                         (unsigned char) b,
                         (unsigned char) 255 };
    }

    // Fallback – return white so you notice something went wrong
    return RAYWHITE;
}

/**
 * @brief Calculates and returns the magnitude of a 2D vector.
 */
float GetLength(const Vector2 vector)
{
    return sqrtf(
        pow(vector.x, 2) + pow(vector.y, 2)
    );
}

/**
 * @brief Turns a vector into its unit vector (same direction, length 1).
 */
void Normalise(Vector2 *vector)
{
    float magnitude = GetLength(*vector);

    vector->x /= magnitude;
    vector->y /= magnitude;
}

/**
 * @brief Returns the area of frame `index` inside a sprite sheet that is
 * split into `rows` x `cols` equally sized frames.
 */
Rectangle getUVRectangle(const Texture2D *texture, int index, int rows, int cols)
{
    float uCoord = (float) (index % cols) / (float) cols;
    uCoord *= texture->width;

    float vCoord = (float) (index / cols) / (float) rows;
    vCoord *= texture->height;

    float sliceWidth  = texture->width  / (float) cols;
    float sliceHeight = texture->height / (float) rows;

    return {
        uCoord,     // top-left x-coord
        vCoord,     // top-left y-coord
        sliceWidth, // width of slice
        sliceHeight // height of slice
    };
}

/**
 * @brief Smoothly moves the camera towards a target position.
 */
void panCamera(Camera2D *camera, const Vector2 *targetPosition)
{
    Vector2 positionDifference = Vector2Subtract(
        *targetPosition, camera->target
    );

    camera->target = Vector2Add(
        camera->target, Vector2Scale(positionDifference, 0.1f)
    ); // 0.1 = smoothing factor
}
