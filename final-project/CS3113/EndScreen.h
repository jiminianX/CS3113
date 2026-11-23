/**
* Author: Joseph Jiminian
* Assignment: seven star rising
* Date due: n/a, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#ifndef END_SCREEN_H
#define END_SCREEN_H

#include "Scene.h"

// The "Valley Cleared" and "Game Over" screens. Both wait for ENTER and go
// back to the menu.
class EndScreen : public Scene
{
private:
    const char *mTitle;
    const char *mMessage;
    Color       mTitleColour;
    float       mTime = 0.0f;

public:
    static constexpr float BLINK_RATE = 1.5f;

    EndScreen();
    EndScreen(Vector2 origin, const char *bgHexCode, const char *title,
              const char *message, Color titleColour);
    ~EndScreen();

    void initialise() override;
    void processInput() override;
    void update(float deltaTime) override;
    void render() override;
    void renderUI() override;
    void shutdown() override;
};

#endif // END_SCREEN_H
