#include "collision.h"
#include "transforms.h"

static struct CollisionInfo
{
    static const uint8_t EMPTY_Y_VALUE = -1;
    static const uint8_t MAX_Y_CHECKS = 3;

    // X and Y are in grid space.
    int8_t checkXLeft;
    int8_t checkXRight;
    uint8_t checkY[MAX_Y_CHECKS];
};

static CollisionInfo calcCollisionInfo(const Bubble &playerBubble0, const Bubble &playerBubble1);


bool canGoLeft(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS], const Bubble &playerBubble0, const Bubble &playerBubble1)
{
    CollisionInfo info = calcCollisionInfo(playerBubble0, playerBubble1);

    if (info.checkXLeft < 0 || info.checkXLeft >= GRID_COLUMNS)
    {
        return false;
    }

    for (uint8_t i = 0; i < CollisionInfo::MAX_Y_CHECKS; i++)
    {
        if (info.checkY[i] != CollisionInfo::EMPTY_Y_VALUE && grid[info.checkXLeft][info.checkY[i]].state == IDLE)
        {
            return false;
        }
    }

    return true;
}

bool canGoRight(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS], const Bubble &playerBubble0, const Bubble &playerBubble1)
{
    CollisionInfo info = calcCollisionInfo(playerBubble0, playerBubble1);

    if (info.checkXRight < 0 || info.checkXRight >= GRID_COLUMNS)
    {
        return false;
    }

    for (uint8_t i = 0; i < CollisionInfo::MAX_Y_CHECKS; i++)
    {
        // IDLE means there is something in the grid location.
        if (info.checkY[i] != CollisionInfo::EMPTY_Y_VALUE && grid[info.checkXRight][info.checkY[i]].state == IDLE)
        {
            return false;
        }
    }

    return true;
}

static CollisionInfo calcCollisionInfo(const Bubble &playerBubble0, const Bubble &playerBubble1)
{
    CollisionInfo result;

    glm::ivec2 gridPos0a;
    glm::ivec2 gridPos0b;
    glm::ivec2 gridPos1a;
    glm::ivec2 gridPos1b;
    bool exactlyInGrid;

    // Convert input to grid coordinates, making sure gridPos0x always refers to the bubble with smallest Y.
    if (playerBubble0.playSpacePosition.y < playerBubble1.playSpacePosition.y)
    {
        exactlyInGrid = (playSpaceToNearestVerticalGrid(playerBubble0.playSpacePosition, gridPos0a, gridPos0b) == 1);
        playSpaceToNearestVerticalGrid(playerBubble1.playSpacePosition, gridPos1a, gridPos1b);
    }
    else
    {
        exactlyInGrid = (playSpaceToNearestVerticalGrid(playerBubble1.playSpacePosition, gridPos0a, gridPos0b) == 1);
        playSpaceToNearestVerticalGrid(playerBubble0.playSpacePosition, gridPos1a, gridPos1b);
    }

    if (gridPos0a.x <= gridPos1a.x)
    {
        result.checkXLeft = gridPos0a.x - 1;
        result.checkXRight = gridPos1a.x + 1;
    }
    else
    {
        result.checkXLeft = gridPos1a.x - 1;
        result.checkXRight = gridPos0a.x + 1;
    }

    // Fill in the CollisionInfo struct with the Y values in the grid that need to be checked for collisions.
    if (playerBubble0.playSpacePosition.x == playerBubble1.playSpacePosition.x)
    {
        // Bubbles are stacked.
        if (exactlyInGrid)
        {
            result.checkY[0] = gridPos0a.y;
            result.checkY[1] = gridPos1a.y;
            result.checkY[2] = CollisionInfo::EMPTY_Y_VALUE;
        }
        else
        {
            result.checkY[0] = gridPos0a.y;
            result.checkY[1] = gridPos0b.y;
            result.checkY[2] = gridPos1b.y;
        }
    }
    else if (playerBubble0.playSpacePosition.y == playerBubble1.playSpacePosition.y)
    {
        // Bubbles are side by side.
        if (exactlyInGrid)
        {
            result.checkY[0] = gridPos0a.y;
            result.checkY[1] = CollisionInfo::EMPTY_Y_VALUE;
            result.checkY[2] = CollisionInfo::EMPTY_Y_VALUE;
        }
        else
        {
            result.checkY[0] = gridPos0a.y;
            result.checkY[1] = gridPos0b.y;
            result.checkY[2] = CollisionInfo::EMPTY_Y_VALUE;
        }
    }

    return result;
}