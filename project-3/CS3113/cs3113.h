/**
* Author: Joseph Jiminian
* Assignment: Rise of the AI
* Date due: 11/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#ifndef CS3113_H
#define CS3113_H

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <iostream>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <string>

#define LOG(argument) std::cout << argument << '\n'

enum AppStatus   { TERMINATED, RUNNING };
enum TextureType { SINGLE, ATLAS       };

Color     ColorFromHex(const char *hex);
void      Normalise(Vector2 *vector);
float     GetLength(const Vector2 vector);
Rectangle getUVRectangle(const Texture2D *texture, int index, int rows, int cols);

#endif // CS3113_H
