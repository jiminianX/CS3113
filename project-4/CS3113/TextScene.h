/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#ifndef TEXT_SCENE_H
#define TEXT_SCENE_H

#include "Scene.h"

struct TextSceneConfig
{
    const char  *title;
    const char  *subtitle;
    const char  *footer;      // may be nullptr
    const char  *bgColour;
    const char  *titleColour;
    SceneID      nextScene;   // where ENTER takes you (lives are reset)
    const char  *musicPath;   // may be nullptr
    const Sound *introSound;  // may be nullptr; played on entry
};

// One class, three scene objects: the main menu, "You Win" and "You Lose".
class TextScene : public Scene
{
private:
    TextSceneConfig mConfig;

    void drawCentredText(const char *text, int y, int fontSize, Color colour) const;

public:
    static constexpr int TITLE_FONT_SIZE    = 72;
    static constexpr int SUBTITLE_FONT_SIZE = 30;
    static constexpr int FOOTER_FONT_SIZE   = 20;

    TextScene(Vector2 origin, const TextSceneConfig &config, const SoundBank *sounds);
    ~TextScene();

    void initialise()            override;
    void processInput()          override;
    void update(float deltaTime) override;
    void render()                override;
    void renderUI()              override;
    void shutdown()              override;
};

#endif // TEXT_SCENE_H
