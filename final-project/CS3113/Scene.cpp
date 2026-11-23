/**
* Author: Joseph Jiminian
* Assignment: seven star rising
* Date due: n/a, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Scene.h"

Scene::Scene() : mGameState{ nullptr, nullptr, NO_SCENE }, mOrigin{ 0.0f, 0.0f } {}

Scene::Scene(Vector2 origin, const char *bgHexCode)
    : mGameState{ nullptr, nullptr, NO_SCENE }, mOrigin{ origin },
      mBGColourHexCode{ bgHexCode }
{}
