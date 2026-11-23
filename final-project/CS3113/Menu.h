/**
* Author: Joseph Jiminian
* Assignment: seven star rising
* Date due: n/a, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#ifndef MENU_H
#define MENU_H

#include "Scene.h"

class Menu : public Scene
{
private:
    Texture2D mOrbTexture;
    float     mTime = 0.0f;

public:
    static constexpr int   ORB_COUNT     = 7;
    static constexpr float RING_CENTRE_Y = 186.0f,
                           RING_RADIUS_X = 450.0f,
                           RING_RADIUS_Y = 70.0f,
                           RING_SPEED    = 0.25f, // radians per second
                           ORB_SIZE      = 42.0f,
                           BLINK_RATE    = 1.5f;

    Menu();
    Menu(Vector2 origin, const char *bgHexCode);
    ~Menu();

    void initialise() override;
    void processInput() override;
    void update(float deltaTime) override;
    void render() override;
    void renderUI() override;
    void shutdown() override;
};

#endif // MENU_H
