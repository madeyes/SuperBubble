#include <stdint.h>
#include "grid.h"
#include "resource_manager.h"
#include "sprite_renderer.h"

void initGrid(Bubble (&grid) [GRID_COLUMNS][GRID_ROWS])
{
	for (uint8_t col = 0; col < GRID_COLUMNS; col++)
	{
		for (uint8_t row = 0; row < GRID_ROWS; row++)
		{
			gridSpaceToPlaySpace(glm::ivec2(col, row), grid[col][row].playSpacePosition);
			grid[col][row].color = RED;
			grid[col][row].state = DEAD;
			grid[col][row].animationFrame = 0;
		}
	}
}


void renderGrid(Bubble const (&grid) [GRID_COLUMNS][GRID_ROWS])
{
	glm::uvec2 renderPos;

	for (uint8_t col = 0; col < GRID_COLUMNS; col++)
	{
		for (uint8_t row = 0; row < GRID_ROWS; row++)
		{
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