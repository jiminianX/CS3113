/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#include "LevelLayout.h"
#include <algorithm>
#include <iostream>

namespace
{
    // Dirt "body" tiles that sit underneath a surface tile.
    const TileTheme BODY_THEME = { 120, 121, 122, 123 };

    const int WANDER_RADIUS = 4;
    const int FOLLOW_RADIUS = 6;
    const int FLY_RADIUS    = 4;
    const int BOSS_RADIUS   = 1000;

    class Grid
    {
    public:
        explicit Grid(const std::vector<std::string> &rows) : mRows(rows)
        {
            for (const std::string &row : rows)
                mColumns = std::max(mColumns, static_cast<int>(row.size()));
        }

        int  columns() const { return mColumns; }
        int  rows()    const { return static_cast<int>(mRows.size()); }
        char at(int row, int col) const
        {
            if (row < 0 || row >= rows() || col < 0) return Layout::EMPTY;
            const std::string &line = mRows[row];
            return col < static_cast<int>(line.size()) ? line[col] : Layout::EMPTY;
        }
        bool isSolid(int row, int col) const { return at(row, col) == Layout::SOLID; }

        // Off the left/right edge counts as solid so border tiles join up.
        bool isSolidForTiling(int row, int col) const
        {
            if (col < 0 || col >= mColumns) return row >= 0 && row < rows();
            return isSolid(row, col);
        }

    private:
        const std::vector<std::string> &mRows;
        int                             mColumns = 0;
    };

    int pickEdgeTile(const TileTheme &theme, bool hasLeft, bool hasRight)
    {
        if (hasLeft && hasRight) return theme.middle;
        if (hasRight)            return theme.left;
        if (hasLeft)             return theme.right;
        return theme.single;
    }

    unsigned int tileFor(const Grid &grid, int row, int col, const TileTheme &surface)
    {
        if (!grid.isSolid(row, col)) return 0;

        bool isSurface = !grid.isSolid(row - 1, col);
        const TileTheme &theme = isSurface ? surface : BODY_THEME;

        int atlasIndex = pickEdgeTile(
            theme,
            grid.isSolidForTiling(row, col - 1),
            grid.isSolidForTiling(row, col + 1)
        );

        return static_cast<unsigned int>(atlasIndex + 1); // Map is 1-based
    }

    // A walker may move along open cells that have ground underneath.
    bool isWalkable(const Grid &grid, int row, int col)
    {
        return col >= 0 && col < grid.columns() &&
               !grid.isSolid(row, col) && grid.isSolid(row + 1, col);
    }

    bool isFlyable(const Grid &grid, int row, int col)
    {
        return col >= 0 && col < grid.columns() && !grid.isSolid(row, col);
    }

    Spawn makeSpawn(const Grid &grid, char kind, int row, int col)
    {
        bool isFlyer = kind == Layout::FLYER;
        int  radius  = kind == Layout::WANDERER ? WANDER_RADIUS :
                       kind == Layout::FOLLOWER ? FOLLOW_RADIUS :
                       isFlyer                  ? FLY_RADIUS    : BOSS_RADIUS;

        // Bosses are tall, so they check for ground below their footprint.
        int probeRow = kind == Layout::BOSS ? row + 1 : row;

        int minCol = col;
        int maxCol = col;

        while (col - minCol < radius &&
               (isFlyer ? isFlyable(grid, row, minCol - 1)
                        : isWalkable(grid, probeRow, minCol - 1)))
            minCol--;

        while (maxCol - col < radius &&
               (isFlyer ? isFlyable(grid, row, maxCol + 1)
                        : isWalkable(grid, probeRow, maxCol + 1)))
            maxCol++;

        Spawn spawn = { kind, row, col, minCol, maxCol };
        return spawn;
    }

    bool isEnemyMarker(char cell)
    {
        return cell == Layout::WANDERER || cell == Layout::FOLLOWER ||
               cell == Layout::FLYER    || cell == Layout::BOSS;
    }
}

ParsedLayout parseLayout(const std::vector<std::string> &rows,
                         const TileTheme                &surfaceTheme)
{
    Grid         grid(rows);
    ParsedLayout layout;
    int          playerCount = 0;

    layout.columns = grid.columns();
    layout.rows    = grid.rows();
    layout.tiles.assign(layout.columns * layout.rows, 0);

    for (int row = 0; row < layout.rows; row++)
    {
        for (int col = 0; col < layout.columns; col++)
        {
            char cell = grid.at(row, col);
            layout.tiles[row * layout.columns + col] =
                tileFor(grid, row, col, surfaceTheme);

            if (cell == Layout::PLAYER)
            {
                layout.playerRow = row;
                layout.playerCol = col;
                playerCount++;
            }
            else if (cell == Layout::GOAL)
            {
                layout.hasGoal = true;
                layout.goalRow = row;
                layout.goalCol = col;
            }
            else if (isEnemyMarker(cell))
            {
                layout.spawns.push_back(makeSpawn(grid, cell, row, col));
            }
        }
    }

    if (playerCount != 1)
        std::cout << "WARNING: level layout has " << playerCount
                  << " player starts (expected exactly 1)\n";

    return layout;
}
