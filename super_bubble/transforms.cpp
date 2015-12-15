#include "transforms.h"

bool gridSpaceToPlaySpace(const glm::uvec2 gridPosition, glm::uvec2 &playPosition)
{
	playPosition.x = gridPosition.x * GRID_SIZE;
	playPosition.y = gridPosition.y * GRID_SIZE;
	return true;
}

bool playSpaceToGridSpace(const glm::uvec2 playPosition, glm::uvec2 &gridPosition)
{
	if (playPosition.x < 0 ||
		playPosition.y < 0 ||
		playPosition.x > (GRID_COLUMNS * GRID_SIZE) ||
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

bool windowSpaceToPlaySpace(const glm::uvec2 windowPosition, glm::uvec2 &playPosition)
{
	if (windowPosition.x < PLAY_SPACE_POS.x ||
		playPosition.y < PLAY_SPACE_POS.y ||
		windowPosition.x > (PLAY_SPACE_POS.x + (GRID_COLUMNS * GRID_SIZE)) ||
		windowPosition.y > (PLAY_SPACE_POS.y + (GRID_ROWS * GRID_SIZE)))
	{
		// Outside play space bounds.
		return false;
	}
	playPosition.x = windowPosition.x - PLAY_SPACE_POS.x;
	playPosition.y = windowPosition.y - PLAY_SPACE_POS.y;
	return true;
}

bool playSpaceToWindowSpace(const glm::uvec2 playPosition, glm::uvec2 &windowPosition)
{
	if (playPosition.x < 0 ||
		playPosition.y < 0 ||
		playPosition.x > (GRID_COLUMNS * GRID_SIZE) ||
		playPosition.y > (GRID_ROWS * GRID_SIZE))
	{
		return false;
	}
	windowPosition.x = playPosition.x + PLAY_SPACE_POS.x;
	windowPosition.y = playPosition.y + PLAY_SPACE_POS.y;
	return true;
}
