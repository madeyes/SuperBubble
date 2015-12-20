#include <list>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include "defs.h"
#include "transforms.h"
#include "collision.h"

static const uint8_t FALL_AMOUNT = 3;
static const int8_t SPAWN_POS_Y = -2;

static enum Direction
{
	NORTH,
	EAST,
	SOUTH,
	WEST
};

static Direction buddyBubbleDirection = SOUTH;
static std::list<Bubble*> currentChain;

// TODO: remove me when we have animation.
static uint8_t frameCount;

static void printBubble(const Bubble &bubble);
static GameState applyGravity(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, double secondsSinceLastUpdate);
static uint8_t checkForLink(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], const uint8_t &x, const uint8_t &y, const BubbleColor color);

GameState menu()
{
	return MENU;
}

GameState spawnBubble(std::list<Bubble> &fallingBubbles)
{
	srand(time(NULL));
	fallingBubbles.clear();	
	glm::ivec2 gridPos(rand() % GRID_COLUMNS, SPAWN_POS_Y);	
	Bubble mainBubble, buddyBubble;
	gridSpaceToPlaySpace(gridPos, mainBubble.playSpacePosition);
	gridPos.y++;
	gridSpaceToPlaySpace(gridPos, buddyBubble.playSpacePosition);
	mainBubble.color = static_cast<BubbleColor>(rand() % (MAX_COLOR + 1));
	buddyBubble.color = static_cast<BubbleColor>(rand() % (MAX_COLOR + 1));
	mainBubble.state = buddyBubble.state = FALLING;
	mainBubble.animationFrame = buddyBubble.animationFrame = 0;
	buddyBubbleDirection = SOUTH;

	// Must be pushed in bottom up order.
	fallingBubbles.push_back(buddyBubble);
	fallingBubbles.push_back(mainBubble);

	return PLAYER_CONTROL;
}

GameState controlPlayerBubbles(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, Controls &controls, double secondsSinceLastUpdate)
{
	//std::cout << secondsSinceLastUpdate << ", ";

	Bubble *buddyBubble = &fallingBubbles.front();
	Bubble *mainBubble = &*std::next(fallingBubbles.begin());

	const glm::ivec2 horizontalMove(GRID_SIZE, 0);
	if (controls.left && canGoLeft(grid, *mainBubble, *buddyBubble))
	{
		mainBubble->playSpacePosition -= horizontalMove;
		buddyBubble->playSpacePosition -= horizontalMove;
		controls.left = false;
	}
	else if (controls.right && canGoRight(grid, *mainBubble, *buddyBubble))
	{
		mainBubble->playSpacePosition += horizontalMove;
		buddyBubble->playSpacePosition += horizontalMove;
		controls.right = false;
	}
	else if (controls.rotate)
	{
		// Rotation is clockwise.
		switch (buddyBubbleDirection)
		{
			case NORTH:
			{
				if (canGoRight(grid, *mainBubble, *buddyBubble))
				{
					buddyBubbleDirection = EAST;
					buddyBubble->playSpacePosition.x += GRID_SIZE;
					buddyBubble->playSpacePosition.y = mainBubble->playSpacePosition.y;
				}
				break;
			}
			case EAST:
			{
				uint8_t remainder = buddyBubble->playSpacePosition.y % GRID_SIZE;
				uint8_t nextY = 0;
				if (remainder == 0)
				{
					nextY = 1 + ((buddyBubble->playSpacePosition.y + GRID_SIZE) / GRID_SIZE);
				}
				else
				{
					nextY = 2 + ((buddyBubble->playSpacePosition.y + GRID_SIZE) / GRID_SIZE);
				}
				if (nextY < GRID_ROWS && grid[mainBubble->playSpacePosition.x / GRID_SIZE][nextY].state != IDLE)
				{
					buddyBubbleDirection = SOUTH;
					buddyBubble->playSpacePosition.x = mainBubble->playSpacePosition.x;
					buddyBubble->playSpacePosition.y += GRID_SIZE;				
				}
				break;
			}
			case SOUTH:
			{
				if (canGoLeft(grid, *mainBubble, *buddyBubble))
				{
					buddyBubbleDirection = WEST;
					buddyBubble->playSpacePosition.x -= GRID_SIZE;
					buddyBubble->playSpacePosition.y = mainBubble->playSpacePosition.y;
				}
				break;
			}
			case WEST:
			{
				buddyBubbleDirection = NORTH;
				buddyBubble->playSpacePosition.x = mainBubble->playSpacePosition.x;
				buddyBubble->playSpacePosition.y -= GRID_SIZE;
				break;
			}
		}
		controls.rotate = false;
	}

	GameState result = applyGravity(grid, fallingBubbles, secondsSinceLastUpdate);
	if (result == GRAVITY && fallingBubbles.size() == 2)
	{
		return PLAYER_CONTROL;
	}
	else
	{
		return result;
	}
}

GameState scanForVictims(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS])
{
	bool foundVictims = false;
	for (uint8_t y = 0; y < GRID_ROWS; y++)
    {
        for (uint8_t x = 0; x < GRID_COLUMNS; x++)
        {
            if (checkForLink(grid, x, y, grid[x][y].color) >= CHAIN_DEATH_LENGTH)
            {
                foundVictims = true;
            }
            else
            {
				for (std::list<Bubble*>::iterator it = currentChain.begin(); it != currentChain.end(); it++)
				{
                    (*it)->state = IDLE;
                }
            }
            currentChain.clear();
        }
    }
    if (foundVictims)
    {
		frameCount = 60;
        return ANIMATE_DEATHS;
    }
    return BUBBLE_SPAWN;    
}

GameState animateDeaths(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], double secondsSinceLastUpdate)
{
	// Until we have some animation just stay in this state for a while.
	if (--frameCount == 0)
	{
		for (uint8_t y = 0; y < GRID_ROWS; y++)
		{
			for (uint8_t x = 0; x < GRID_COLUMNS; x++)
			{			
				if (grid[x][y].state == DYING)
				{
					grid[x][y].state = DEAD;
				}
			}
		}
		return SCAN_FOR_FLOATERS;
	}	
	return ANIMATE_DEATHS;
}

GameState scanForFloaters(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles)
{
	bool foundFloaters = false;    
    for (int x = 0; x < GRID_COLUMNS; x++)
    {
        bool emptySpace = false;
        // Check rows from the bottom up for empty space.
        for (int y = GRID_ROWS - 1; y >= 0; y--)
        {            
            if (grid[x][y].state == DEAD)
            {
                emptySpace = true;
            }
            else if (grid[x][y].state == IDLE)
            {
                // If we have seen empty space before seeing this bubble then it must fall.
                if (emptySpace)
                {
                    // Copy this bubble and add it to the falling list.
					Bubble faller = grid[x][y];
					faller.state = FALLING;
                    fallingBubbles.push_back(faller);
                    // Mark the old grid position as dead.
                    grid[x][y].state = DEAD;
                    foundFloaters = true;
                }
            }
        }
    }
    if (foundFloaters)
    {
        return GRAVITY;
    }
    else
    {
        return BUBBLE_SPAWN;
    }	
}

GameState gravity(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, double secondsSinceLastUpdate)
{
	return applyGravity(grid, fallingBubbles, secondsSinceLastUpdate);
}

GameState gameOver()
{	
	return GAME_OVER;
}

static GameState applyGravity(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, double secondsSinceLastUpdate)
{
	uint8_t pixels = round(static_cast<double>(FALL_AMOUNT) * (secondsSinceLastUpdate / TARGET_FRAME_SECONDS));

	glm::ivec2 gridPos0;
	glm::ivec2 gridPos1;
	std::list<Bubble>::iterator it = fallingBubbles.begin();
	while (it != fallingBubbles.end())
	{
		// Which grid squares would we be overlapping after adding the fall amount?
		const glm::ivec2 playSpaceNext(it->playSpacePosition.x, it->playSpacePosition.y + pixels);
		uint8_t numMatches = playSpaceToNearestVerticalGrid(playSpaceNext, gridPos0, gridPos1);
		
		// Check if one of the overlapped squares is ground or an idle bubble.
		glm::ivec2 *hitPos = nullptr;
		if (numMatches > 0 && (gridPos0.y == GRID_ROWS || (gridPos0.y >= 0 && grid[gridPos0.x][gridPos0.y].state == IDLE)))
		{
			hitPos = &gridPos0;						
		}
		else if (numMatches == 2 && (gridPos1.y == GRID_ROWS || (gridPos1.y >= 0 && grid[gridPos1.x][gridPos1.y].state == IDLE)))
		{			
			hitPos = &gridPos1;
		}

		if (hitPos != nullptr)
		{			
			grid[hitPos->x][hitPos->y - 1].state = IDLE;
			grid[hitPos->x][hitPos->y - 1].color = it->color;
			
			fallingBubbles.erase(it++);

			// Check if the settle position of this bubble was the top row.
			if (hitPos->y - 1 == 0)
			{
				return GAME_OVER;
			}			
		}
		else
		{
			it->playSpacePosition.y += pixels;		
			it++;
		}
	}

	if (fallingBubbles.size() == 0)
	{
		return SCAN_FOR_VICTIMS;
	}
	return GRAVITY;
}


// Finds size of a group of touching same coloured squares.
// Takes x, y input specifying grid location (in grid co-ordinates!) to start checking
// from.
static int findGroupSize(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], const uint8_t &x, const uint8_t &y, const BubbleColor &color)
{
    if (grid[x][y].color == color)
    {
        grid[x][y].state = DYING;
        currentChain.push_back(&grid[x][y]);
        return 1 + 
			checkForLink(grid, x - 1, y, color) + 
			checkForLink(grid, x, y - 1, color) + 
			checkForLink(grid, x + 1, y, color) + 
			checkForLink(grid, x, y + 1, color);
    }
    else
    {
        return 0;
    }
}

static uint8_t checkForLink(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], const uint8_t &x, const uint8_t &y, const BubbleColor color)
{
    // Make sure we are not outside the grid. We are checking grid coordinates, not pixels, so boundary is at zero.
    if (x < 0) return 0;
    if (y < 0) return 0;
    if (x > GRID_COLUMNS - 1 ) return 0;
    if (y > GRID_ROWS - 1 ) return 0;

    if (grid[x][y].state == IDLE)
    {
        return findGroupSize(grid, x, y, color);
    }
    return 0;
}

static void printBubble(const Bubble &bubble)
{
		std::cout << "(" << bubble.playSpacePosition.x << "," << bubble.playSpacePosition.y << ") state: " << bubble.state << std::endl;
}