#include "transforms.h"

bool gridSpaceToPlaySpace(const glm::ivec2 gridPosition, glm::ivec2 &playPosition)
{
    playPosition.x = gridPosition.x * GRID_SIZE;
    playPosition.y = gridPosition.y * GRID_SIZE;
    return true;
}

bool playSpaceToGridSpace(const glm::ivec2 playPosition, glm::ivec2 &gridPosition)
{
    // TODO: Should it be (GRID_COLUMNS - 1) and (GRID_ROWS - 1)?
    if (playPosition.x > (GRID_COLUMNS * GRID_SIZE) ||
        playPosition.y > (GRID_ROWS * GRID_SIZE) ||
        playPosition.x % GRID_SIZE != 0 ||
        playPosition.y % GRID_SIZE != 0)
    {
        return false;
    }
    else
    {
        gridPosition.x = playPosition.x / GRID_SIZE;
        gridPosition.y = playPosition.y / GRID_SIZE;
        return true;
    }
}

bool windowSpaceToPlaySpace(const glm::uvec2 windowPosition, glm::ivec2 &playPosition)
{
    if (windowPosition.x < PLAY_SPACE_POS.x ||
        windowPosition.y < PLAY_SPACE_POS.y ||
        windowPosition.x >(PLAY_SPACE_POS.x + (GRID_COLUMNS * GRID_SIZE)) ||
        windowPosition.y >(PLAY_SPACE_POS.y + (GRID_ROWS * GRID_SIZE)))
    {
        // Outside play space bounds.
        return false;
    }
    playPosition.x = windowPosition.x - PLAY_SPACE_POS.x;
    playPosition.y = windowPosition.y - PLAY_SPACE_POS.y;
    return true;
}

bool playSpaceToWindowSpace(const glm::ivec2 playPosition, glm::uvec2 &windowPosition)
{
    // Y can be less than zero to allow bubbles to enter from off-screen, but X cannot.
    if (playPosition.x < 0 ||
        playPosition.x >(GRID_COLUMNS * GRID_SIZE) ||
        playPosition.y > (GRID_ROWS * GRID_SIZE))
    {
        return false;
    }
    windowPosition.x = playPosition.x + PLAY_SPACE_POS.x;
    windowPosition.y = playPosition.y + PLAY_SPACE_POS.y;
    return true;
}

// Return: number of matches
uint8_t playSpaceToNearestVerticalGrid(const glm::ivec2 playPosition, glm::ivec2 &gridPosition0, glm::ivec2 &gridPosition1)
{
    int8_t x = playPosition.x / GRID_SIZE;
    int8_t y = playPosition.y / GRID_SIZE;

    if (playPosition.x % GRID_SIZE == 0)
    {
        gridPosition0.x = x;
    }
    else
    {
        return 0;
    }

    if (playPosition.y % GRID_SIZE == 0)
    {
        gridPosition0.y = y;
        return 1;
    }
    else
    {
        gridPosition1.x = x;
        gridPosition0.y = y;
        if (playPosition.y < 0)
        {
            gridPosition1.y = y - 1;
        }
        else
        {
            gridPosition1.y = y + 1;
        }
        return 2;
    }
}