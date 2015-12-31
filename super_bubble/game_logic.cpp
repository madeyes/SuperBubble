#include <list>
#include <stdlib.h>
#include <iostream>
#include "defs.h"
#include "transforms.h"
#include "collision.h"

static int8_t fallAmount = (int8_t)(3.0f * SCALE);
static int8_t levelFallAmount = (int8_t)(3.0f * SCALE);

static const int8_t SPAWN_POS_Y = -2;

static uint8_t *deathFrame = nullptr;

// For game over animation.
static int8_t gameOverRow = GRID_ROWS - 1;

enum Direction
{
    NORTH,
    EAST,
    SOUTH,
    WEST
};

static Direction buddyBubbleDirection = SOUTH;
static std::list<Bubble*> currentChain;
static std::list<Bubble*> bounceList;

static void printBubble(const Bubble &bubble);
static GameState applyGravity(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, double secondsSinceLastUpdate);
static uint8_t checkForLink(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], const uint8_t &x, const uint8_t &y, const BubbleColor color);
static void bounce();

void resetGameLogic()
{
	gameOverRow = GRID_ROWS - 1;
}

GameState menu()
{
    return MENU;
}

GameState spawnBubble(std::list<Bubble> &fallingBubbles, std::pair<BubbleColor, BubbleColor> &nextColors)
{
    fallingBubbles.clear();
    glm::ivec2 gridPos(rand() % GRID_COLUMNS, SPAWN_POS_Y);
    Bubble mainBubble, buddyBubble;
    gridSpaceToPlaySpace(gridPos, mainBubble.playSpacePosition);
    gridPos.y++;
    gridSpaceToPlaySpace(gridPos, buddyBubble.playSpacePosition);
	mainBubble.color = nextColors.first;
	buddyBubble.color = nextColors.second;
    nextColors.first = static_cast<BubbleColor>(rand() % (MAX_COLOR + 1));
    nextColors.second = static_cast<BubbleColor>(rand() % (MAX_COLOR + 1));
    mainBubble.state = buddyBubble.state = FALLING;
    mainBubble.animationFrame = buddyBubble.animationFrame = 0;
    mainBubble.visited = buddyBubble.visited = false;
    mainBubble.bounceAmount = buddyBubble.bounceAmount = 0;
    mainBubble.bounceDir = buddyBubble.bounceDir = 0;
    
    buddyBubbleDirection = SOUTH;
    fallAmount = levelFallAmount;    

    // Must be pushed in bottom up order.
    fallingBubbles.push_back(buddyBubble);
    fallingBubbles.push_back(mainBubble);

    return PLAYER_CONTROL;
}

GameState controlPlayerBubbles(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, Controls &controls, double secondsSinceLastUpdate)
{
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
    else if (controls.rotateCW)
    {        
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
            uint8_t nextY = 1 + ((buddyBubble->playSpacePosition.y + GRID_SIZE) / GRID_SIZE);                        
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
        controls.rotateCW = false;
    }
    else if (controls.rotateACW)
    {
        switch (buddyBubbleDirection)
        {
            case NORTH:
            {
                if (canGoLeft(grid, *mainBubble, *buddyBubble))
                {
                    buddyBubbleDirection = WEST;
                    buddyBubble->playSpacePosition.x -= GRID_SIZE;
                    buddyBubble->playSpacePosition.y = mainBubble->playSpacePosition.y;
                }
                break;
            }
            case EAST:
            {
                buddyBubbleDirection = NORTH;
                buddyBubble->playSpacePosition.x = mainBubble->playSpacePosition.x;
                buddyBubble->playSpacePosition.y -= GRID_SIZE;            
                break;
            }
            case SOUTH:
            {
                if (canGoRight(grid, *mainBubble, *buddyBubble))
                {
                    buddyBubbleDirection = EAST;
                    buddyBubble->playSpacePosition.x += GRID_SIZE;
                    buddyBubble->playSpacePosition.y = mainBubble->playSpacePosition.y;
                }
                break;
            }
            case WEST:
            {            
                uint8_t nextY = 1 + ((buddyBubble->playSpacePosition.y + GRID_SIZE) / GRID_SIZE);            
                if (nextY < GRID_ROWS && grid[mainBubble->playSpacePosition.x / GRID_SIZE][nextY].state != IDLE)
                {
                    buddyBubbleDirection = SOUTH;
                    buddyBubble->playSpacePosition.x = mainBubble->playSpacePosition.x;
                    buddyBubble->playSpacePosition.y += GRID_SIZE;
                }
                break;
            }
        }
        controls.rotateACW = false;
    }
    else if (controls.drop)
    {
        // Increase speed and take away player control.
        controls.drop = false;
        return GRAVITY;
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

GameState scanForVictims(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], uint32_t &score)
{
    bool foundVictims = false;
    for (uint8_t y = 0; y < GRID_ROWS; y++)
    {
        for (uint8_t x = 0; x < GRID_COLUMNS; x++)
        {
            // Bubbles that were already found to be part of another chain can be skipped.
            if (!grid[x][y].visited)
            {
				uint8_t chainLength = checkForLink(grid, x, y, grid[x][y].color);
                if (chainLength >= CHAIN_DEATH_LENGTH)
                {
                    foundVictims = true;
					score += ((chainLength - (CHAIN_DEATH_LENGTH - 1)) * 100);
                    for (std::list<Bubble*>::iterator it = currentChain.begin(); it != currentChain.end(); it++)
                    {
                        (*it)->animationFrame = 0;
                    }
                    // Save pointer to animation frame so that we can track it in the animate death state.
                    deathFrame = &(currentChain.front()->animationFrame);
                }
                else
                {
                    // Chain wasn't long enough, so reset state of all bubbles in the chain to idle.
                    for (std::list<Bubble*>::iterator it = currentChain.begin(); it != currentChain.end(); it++)
                    {
                        (*it)->state = IDLE;
                    }
                }
                currentChain.clear();
            }
        }
    }
    if (foundVictims)
    {
        return ANIMATE_DEATHS;
    }
    return BUBBLE_SPAWN;
}

GameState animateDeaths(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS])
{    
    if (*deathFrame == BUBBLE_FRAMES - 1)
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

GameState scanForFloaters(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles)
{
    bool foundFloaters = false;
    for (int x = 0; x < GRID_COLUMNS; x++)
    {
        bool emptySpace = false;
        // Check rows from the bottom up for empty space.
        for (int y = GRID_ROWS - 1; y >= 0; y--)
        {
            // Clear visited flag used by chain algorithm.
            grid[x][y].visited = false;
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

GameState gravity(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, double secondsSinceLastUpdate)
{
    fallAmount = FAST_FALL_AMOUNT;
    return applyGravity(grid, fallingBubbles, secondsSinceLastUpdate);
}


GameState gameOver(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS])
{
	if (gameOverRow >= 0)
	{		
		for (uint8_t col = 0; col < GRID_COLUMNS; col++)
		{	
			grid[col][gameOverRow].state = GHOST;
		}
		gameOverRow--;
	}
	return GAME_OVER;
}

static void bounce()
{    
    bool allDone = true;
    for (std::list<Bubble*>::iterator it = bounceList.begin(); it != bounceList.end(); it++)
    {
        if ((*it)->bounceAmount != 0)
        {
            allDone = false;
            (*it)->playSpacePosition.y += ((*it)->bounceAmount * (*it)->bounceDir);
            (*it)->bounceDir *= -1;
            if ((*it)->bounceDir < 0) (*it)->bounceAmount--;
        }
    }
    
    if (allDone)
    {
        bounceList.clear();
    }
}

static GameState applyGravity(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, double secondsSinceLastUpdate)
{
    uint8_t pixels = round(static_cast<double>(fallAmount) * (secondsSinceLastUpdate / TARGET_FRAME_SECONDS));

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
            grid[hitPos->x][hitPos->y - 1].bounceAmount = BOUNCE_HEIGHT;
            grid[hitPos->x][hitPos->y - 1].bounceDir = -1;

            bounceList.push_back(&grid[hitPos->x][hitPos->y - 1]);

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

    // Apply bounce to anything on the bounce list.
    if (bounceList.size() > 0)
    {
        bounce();
    }
    else if (fallingBubbles.size() == 0)
    {
        return SCAN_FOR_VICTIMS;
    }
    return GRAVITY;
}

// Finds size of a group of touching same coloured squares.
// Takes x, y input specifying grid location (in grid co-ordinates!) to start checking
// from.
static int findGroupSize(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], const uint8_t &x, const uint8_t &y, const BubbleColor &color)
{
    if (grid[x][y].color == color)
    {
        grid[x][y].state = DYING;
        // Set visited flag so we don't start looking for a chain from this bubble again.
        // The visited state of all bubbles will be cleared when scanning for floaters.
        grid[x][y].visited = true;
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

static uint8_t checkForLink(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], const uint8_t &x, const uint8_t &y, const BubbleColor color)
{
    // Make sure we are not outside the grid. We are checking grid coordinates, not pixels, so boundary is at zero.
    if (x < 0) return 0;
    if (y < 0) return 0;
    if (x > GRID_COLUMNS - 1) return 0;
    if (y > GRID_ROWS - 1) return 0;

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