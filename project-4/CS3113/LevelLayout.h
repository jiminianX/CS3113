/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#ifndef LEVEL_LAYOUT_H
#define LEVEL_LAYOUT_H

#include <string>
#include <vector>

/*
    Levels are drawn as ASCII art, one string per tile row:

        '.'  empty            'P'  player start      'G'  goal flag
        '#'  solid ground     'W'  wanderer          'F'  follower
                              'V'  flyer (bat)       'B'  boss

    The parser turns that into the tile-index array the Map class expects
    (auto-picking left/middle/right edge tiles) plus a list of spawns.
*/
namespace Layout
{
    const char EMPTY    = '.';
    const char SOLID    = '#';
    const char PLAYER   = 'P';
    const char GOAL     = 'G';
    const char WANDERER = 'W';
    const char FOLLOWER = 'F';
    const char FLYER    = 'V';
    const char BOSS     = 'B';
}

// 0-based indices into the tile atlas for a "surface" row of tiles.
struct TileTheme
{
    int single;
    int left;
    int middle;
    int right;
};

struct Spawn
{
    char kind;
    int  row;
    int  col;
    int  minCol; // patrol range (inclusive)
    int  maxCol;
};

struct ParsedLayout
{
    int                       columns = 0;
    int                       rows    = 0;
    std::vector<unsigned int> tiles;   // 1-based for Map; 0 = no tile
    int                       playerRow = 0;
    int                       playerCol = 0;
    bool                      hasGoal   = false;
    int                       goalRow   = 0;
    int                       goalCol   = 0;
    std::vector<Spawn>        spawns;
};

ParsedLayout parseLayout(const std::vector<std::string> &rows,
                         const TileTheme                &surfaceTheme);

#endif // LEVEL_LAYOUT_H
