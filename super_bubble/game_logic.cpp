#include <list>
#include <time.h>
#include <stdlib.h>
#include "defs.h"
#include "transforms.h"

static bool applyGravity(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles);

GameState menu()
{
	return MENU;
}

GameState spawnBubble(Bubble &mainBubble, Bubble &buddyBubble, std::list<Bubble> &fallingBubbles)
{
	srand(time(NULL));
	fallingBubbles.clear();
	glm::uvec2 gridPos(rand() % GRID_COLUMNS, 0);
	gridSpaceToPlaySpace(gridPos, mainBubble.playSpacePosition);
	gridPos.y++;
	gridSpaceToPlaySpace(gridPos, buddyBubble.playSpacePosition);
	mainBubble.color = static_cast<BubbleColor>(rand() % (MAX_COLOR + 1));
	buddyBubble.color = static_cast<BubbleColor>(rand() % (MAX_COLOR + 1));
	mainBubble.state = buddyBubble.state = FALLING;
	mainBubble.animationFrame = buddyBubble.animationFrame = 0;	
	// Must be pushed in bottom up order for collision detection to work.
	fallingBubbles.push_back(buddyBubble);
	fallingBubbles.push_back(mainBubble);
	
	//return PLAYER_CONTROL;
	return GRAVITY;
}

GameState controlPlayerBubbles(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS], Bubble &mainBubble, Bubble &buddyBubble /*, keys */)
{
	return PLAYER_CONTROL;
}

GameState scanForVictims(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS])
{
	return SCAN_FOR_VICTIMS;
}

GameState animateDeaths(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS])
{
	return ANIMATE_DEATHS;
}

GameState scanForFloaters(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles)
{
	return SCAN_FOR_FLOATERS;
}

GameState gravity(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles)
{
	if (applyGravity(grid, fallingBubbles))
	{
		//return SCAN_FOR_VICTIMS;
		return BUBBLE_SPAWN;
	}
	return GRAVITY;
}

GameState gameOver()
{
	return GAME_OVER;
}

static bool applyGravity(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles)
{
	glm::uvec2 gridPos;

	std::list<Bubble>::iterator it = fallingBubbles.begin();
	while (it != fallingBubbles.end())
	{
		// Check if we are exactly in a grid square. Only then do we need to check for something below us.
		if (playSpaceToGridSpace(it->playSpacePosition, gridPos))
		{
			// Check for ground or a bubble in the square below.
			if (gridPos.y == GRID_ROWS - 1 || grid[gridPos.x][gridPos.y + 1].state == IDLE)
			{
				// We have hit something so fill in this grid square...
				grid[gridPos.x][gridPos.y].state = IDLE;
				grid[gridPos.x][gridPos.y].color = it->color;

				// ...and remove the bubble from the falling list.
				fallingBubbles.erase(it++);
				continue;
			}
		}
		if (it->state == FALLING)
		{
			// TODO: use falling speed.
			it->playSpacePosition.y++;
		}
		it++;
	}

	if (fallingBubbles.size() == 0)
	{
		return true;
	}
	return false;
}