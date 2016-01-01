#include <stdint.h>
#include <iostream>
#include "grid.h"
#include "resource_manager.h"
#include "sprite_renderer.h"

void initGrid(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS])
{
    for (uint8_t col = 0; col < GRID_COLUMNS; col++)
    {
        for (uint8_t row = 0; row < GRID_ROWS; row++)
        {
            gridSpaceToPlaySpace(glm::ivec2(col, row), grid[col][row].playSpacePosition);
            grid[col][row].color = RED;
            grid[col][row].state = DEAD;
            grid[col][row].animationFrame = 0;
            grid[col][row].visited = false;
            grid[col][row].bounceAmount = BOUNCE_HEIGHT;
            grid[col][row].bounceDir = -1;
        }
    }
}


void renderGrid(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], const double secondsSinceLastUpdate)
{
    static double seconds = 0.0;
    seconds += secondsSinceLastUpdate;

    bool updateFrame = false;    
    if (seconds > BUBBLE_FRAME_SECONDS)
    {        
        updateFrame = true;
        seconds = 0.0;
    }
    
    glm::uvec2 renderPos;
    for (uint8_t col = 0; col < GRID_COLUMNS; col++)
    {
        for (uint8_t row = 0; row < GRID_ROWS; row++)
        {
            if (updateFrame)
            {
                if (grid[col][row].animationFrame + 1 < BUBBLE_FRAMES)
                {
                    grid[col][row].animationFrame++;    
                }
                else
                {
                    grid[col][row].animationFrame = 0;
                }
            }
            if (grid[col][row].state != DEAD)
            {
                // The bubbles are defined in play space, but this may be offset from window space, so transform it.
                playSpaceToWindowSpace(grid[col][row].playSpacePosition, renderPos);
                drawSprite(
                    // The texture atlas to use.
                    ResourceManager::GetTexture("bubbles"),
                    // Size of source image to extract from texture atlas.
                    UV_SIZE_BUBBLE,
                    // Column in texture sheet to use.
                    grid[col][row].animationFrame,
                    // Row in texture sheet to use. Based on current state.
                    grid[col][row].state - 1,
                    // Render position in window space coordinates.
                    renderPos,
                    // Size of target rendered image in window.
                    glm::uvec2(GRID_SIZE, GRID_SIZE),
                    // No rotation.
                    0.0f,
                    // RGB colour.
                    BUBBLE_COLORS[grid[col][row].color]);
            }
        }
    }
}