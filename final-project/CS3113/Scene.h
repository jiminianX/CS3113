/**
* Author: Joseph Jiminian
* Assignment: seven star rising
* Date due: n/a, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#ifndef SCENE_H
#define SCENE_H

#include "Entity.h"

struct GameState
{
    Entity *hero; // nullptr on menu screens
    Map    *map;  // the solid tiles the hero collides with

    SceneID nextSceneID;
};

class Scene
{
protected:
    GameState   mGameState;
    Vector2     mOrigin;
    const char *mBGColourHexCode = "#000000";

public:
    Scene();
    Scene(Vector2 origin, const char *bgHexCode);
    virtual ~Scene() {}

    virtual void initialise() = 0;
    virtual void processInput() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;   // inside the camera (world)
    virtual void renderUI() = 0; // on top of the screen (HUD, text)
    virtual void shutdown() = 0;

    GameState   getState()           const { return mGameState;       }
    Vector2     getOrigin()          const { return mOrigin;          }
    const char *getBGColourHexCode() const { return mBGColourHexCode; }
};

#endif // SCENE_H
