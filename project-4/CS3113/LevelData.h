/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#ifndef LEVEL_DATA_H
#define LEVEL_DATA_H

#include "cs3113.h"
#include "LevelLayout.h"

// Everything that makes one level different from another.
struct LevelConfig
{
    const char              *name;
    std::vector<std::string> layout;
    TileTheme                theme;
    const char              *bgColour;
    const char              *musicPath;
    SceneID                  nextScene;
    float                    enemySpeedScale;
};

LevelConfig level1Config();
LevelConfig level2Config();
LevelConfig level3Config();
LevelConfig bossLevelConfig();

#endif // LEVEL_DATA_H
