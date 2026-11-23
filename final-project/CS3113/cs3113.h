/**
* Author: Joseph Jiminian
* Assignment: seven star rising
* Date due: n/a, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#ifndef CS3113_H
#define CS3113_H
#define LOG(argument) std::cout << argument << '\n'

#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <string>
#include <iostream>

constexpr int SCREEN_WIDTH  = 1280,
              SCREEN_HEIGHT = 720;

enum AppStatus   { TERMINATED, RUNNING };
enum TextureType { SINGLE, ATLAS       };
enum SceneID     { NO_SCENE = -1, MENU, VALLEY, VALLEY_CLEARED, GAME_OVER };

Color     ColorFromHex(const char *hex);
void      Normalise(Vector2 *vector);
float     GetLength(const Vector2 vector);
Rectangle getUVRectangle(
    const Texture2D *texture, int index, int rows, int cols
);
void      panCamera(Camera2D *camera, const Vector2 *targetPosition);

#endif // CS3113_H
