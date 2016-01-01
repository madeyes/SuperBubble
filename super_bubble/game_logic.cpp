#include <list>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include "defs.h"
#include "transforms.h"
#include "collision.h"
#include "bubble_net.h"

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
static GameState applyGravity(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, const double secondsSinceLastUpdate);
static uint8_t checkForLink(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], const uint8_t &x, const uint8_t &y, const BubbleColor color);
static void bounce();

void resetGameLogic()
{
	gameOverRow = GRID_ROWS - 1;
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
    nextColors.first = static_cast<BubbleColor>(rand() % (MAX_SPAWN_COLOR + 1));
    nextColors.second = static_cast<BubbleColor>(rand() % (MAX_SPAWN_COLOR + 1));
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

    return GameState::PLAYER_CONTROL;
}

GameState controlPlayerBubbles(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, Controls &controls, const double secondsSinceLastUpdate)
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
        case Direction::NORTH:
        {
            if (canGoRight(grid, *mainBubble, *buddyBubble))
            {
                buddyBubbleDirection = Direction::EAST;
                buddyBubble->playSpacePosition.x += GRID_SIZE;
                buddyBubble->playSpacePosition.y = mainBubble->playSpacePosition.y;
            }
            break;
        }
        case Direction::EAST:
        {            
            uint8_t nextY = 1 + ((buddyBubble->playSpacePosition.y + GRID_SIZE) / GRID_SIZE);                        
            if (nextY < GRID_ROWS && 
                grid[mainBubble->playSpacePosition.x / GRID_SIZE][nextY].state != BubbleState::IDLE)
            {
                buddyBubbleDirection = Direction::SOUTH;
                buddyBubble->playSpacePosition.x = mainBubble->playSpacePosition.x;
                buddyBubble->playSpacePosition.y += GRID_SIZE;
            }
            break;
        }
        case Direction::SOUTH:
        {
            if (canGoLeft(grid, *mainBubble, *buddyBubble))
            {
                buddyBubbleDirection = Direction::WEST;
                buddyBubble->playSpacePosition.x -= GRID_SIZE;
                buddyBubble->playSpacePosition.y = mainBubble->playSpacePosition.y;
            }
            break;
        }
        case Direction::WEST:
        {
            buddyBubbleDirection = Direction::NORTH;
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
            case Direction::NORTH:
            {
                if (canGoLeft(grid, *mainBubble, *buddyBubble))
                {
                    buddyBubbleDirection = Direction::WEST;
                    buddyBubble->playSpacePosition.x -= GRID_SIZE;
                    buddyBubble->playSpacePosition.y = mainBubble->playSpacePosition.y;
                }
                break;
            }
            case Direction::EAST:
            {
                buddyBubbleDirection = Direction::NORTH;
                buddyBubble->playSpacePosition.x = mainBubble->playSpacePosition.x;
                buddyBubble->playSpacePosition.y -= GRID_SIZE;            
                break;
            }
            case Direction::SOUTH:
            {
                if (canGoRight(grid, *mainBubble, *buddyBubble))
                {
                    buddyBubbleDirection = Direction::EAST;
                    buddyBubble->playSpacePosition.x += GRID_SIZE;
                    buddyBubble->playSpacePosition.y = mainBubble->playSpacePosition.y;
                }
                break;
            }
            case Direction::WEST:
            {            
                uint8_t nextY = 1 + ((buddyBubble->playSpacePosition.y + GRID_SIZE) / GRID_SIZE);            
                if (nextY < GRID_ROWS && 
                    grid[mainBubble->playSpacePosition.x / GRID_SIZE][nextY].state != BubbleState::IDLE)
                {
                    buddyBubbleDirection = Direction::SOUTH;
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
        return GameState::GRAVITY;
    }

    GameState result = applyGravity(grid, fallingBubbles, secondsSinceLastUpdate);
    if (result == GRAVITY && fallingBubbles.size() == 2)
    {
        return GameState::PLAYER_CONTROL;
    }
    else
    {
        return result;
    }
}

/*
 * numEnemyBubbles will be updated with the number of enemy bubbles consumed (dropped onto the play field).
**/
GameState dropEnemyBubbles(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, uint8_t &numEnemyBubbles, const double secondsSinceLastUpdate)
{
    if (numEnemyBubbles == 0)
    {
        return GameState::SCAN_FOR_VICTIMS;
    }
    else
    {
        std::cout << "Got enemy bubbles: " << (uint16_t)numEnemyBubbles << std::endl;
        glm::ivec2 gridPos(0, -1);
        int8_t numBubblesToDrop = std::min(numEnemyBubbles, GRID_COLUMNS);
        for (int8_t x = 0; x < numBubblesToDrop; x++)
        {
            Bubble faller;
            faller.state = BubbleState::FALLING;
            faller.color = GHOST;
            gridPos.x = x;
            gridSpaceToPlaySpace(gridPos, faller.playSpacePosition);
            fallingBubbles.push_back(faller);
        }
        numEnemyBubbles -= numBubblesToDrop;
        return GameState::GRAVITY;
    }
}

GameState scanForVictims(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], uint32_t &score)
{
    bool foundVictims = false;    
    uint8_t totalDeaths = 0;

    for (uint8_t y = 0; y < GRID_ROWS; y++)
    {
        for (uint8_t x = 0; x < GRID_COLUMNS; x++)
        {
            // Bubbles that were already found to be part of another chain can be skipped.
            if (!grid[x][y].visited && grid[x][y].color != GHOST)
            {
				uint8_t chainLength = checkForLink(grid, x, y, grid[x][y].color);  
                
                if (chainLength >= CHAIN_DEATH_LENGTH)
                {                    
                    foundVictims = true;                    
                    totalDeaths += chainLength;

                    score += ((chainLength - (CHAIN_DEATH_LENGTH - 1)) * 100);
                    for (std::list<Bubble*>::iterator it = currentChain.begin(); it != currentChain.end(); it++)
                    {
                        (*it)->animationFrame = 0;
                    }
                    // Save pointer to animation frame so that we can track it in the animate death state.
                    // The frames will be the same for all bubbles.
                    deathFrame = &(currentChain.front()->animationFrame);
                }
                else
                {
                    // Chain wasn't long enough, so reset state of all bubbles in the chain to idle.
                    for (std::list<Bubble*>::iterator it = currentChain.begin(); it != currentChain.end(); it++)
                    {
                        (*it)->state = BubbleState::IDLE;
                    }
                }
                currentChain.clear();
            }
        }
    }    
    if (foundVictims)
    {
        // Now find chains of ghost bubbles.
        // They can only be killed if they are touching a chain of another colour.
        for (uint8_t y = 0; y < GRID_ROWS; y++)
        {
            for (uint8_t x = 0; x < GRID_COLUMNS; x++)
            {
                if (!grid[x][y].visited && grid[x][y].color == GHOST)
                {
                    checkForLink(grid, x, y, GHOST);
                    
                    bool killGhostChain = false;
                    for (std::list<Bubble*>::iterator it = currentChain.begin(); it != currentChain.end(); it++)
                    {
                        // Check each direction to see if it is touching a dying bubble.
                        glm::ivec2 gridPos;
                        playSpaceToGridSpace((*it)->playSpacePosition, gridPos);
                        // Above
                        if (gridPos.y > 0 && 
                            grid[gridPos.x][gridPos.y - 1].color != GHOST &&
                            grid[gridPos.x][gridPos.y - 1].state == BubbleState::DYING)
                        {
                            killGhostChain = true;
                            break;
                        }
                        // Below
                        if (gridPos.y < GRID_ROWS - 1 &&
                            grid[gridPos.x][gridPos.y + 1].color != GHOST &&
                            grid[gridPos.x][gridPos.y + 1].state == BubbleState::DYING)
                        {
                            killGhostChain = true;
                            break;
                        }
                        // Left
                        if (gridPos.x > 0 &&
                            grid[gridPos.x - 1][gridPos.y].color != GHOST &&
                            grid[gridPos.x - 1][gridPos.y].state == BubbleState::DYING)
                        {
                            killGhostChain = true;
                            break;
                        }
                        // Right
                        if (gridPos.x < GRID_COLUMNS - 1 &&
                            grid[gridPos.x + 1][gridPos.y].color != GHOST &&
                            grid[gridPos.x + 1][gridPos.y].state == BubbleState::DYING)
                        {
                            killGhostChain = true;
                            break;
                        }
                    } // end iterate over currentChain.                    
                    for (std::list<Bubble*>::iterator it = currentChain.begin(); it != currentChain.end(); it++)
                    {
                        if (!killGhostChain)
                        {
                            (*it)->state = BubbleState::IDLE;
                        }
                        else
                        {
                            (*it)->animationFrame = 0;
                        }
                    }                    
                    currentChain.clear();
                } // end if (visited) and (color == GHOST)
            } // end iterate over x.
        } // end iterate over y.
        if (networkIsConnected() && totalDeaths >= CHAIN_MIN_SEND_LENGTH)
        {
            sendBubbles((totalDeaths - (CHAIN_MIN_SEND_LENGTH - 1)) * 2);
        }
        return GameState::ANIMATE_DEATHS;
    }
    else
    {        
        return GameState::BUBBLE_SPAWN;
    }
}

GameState animateDeaths(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS])
{    
    if (*deathFrame == BUBBLE_FRAMES - 1)
    {
        for (uint8_t y = 0; y < GRID_ROWS; y++)
        {
            for (uint8_t x = 0; x < GRID_COLUMNS; x++)
            {
                if (grid[x][y].state == BubbleState::DYING)
                {
                    grid[x][y].state = BubbleState::DEAD;
                }
            }
        }        
        return GameState::SCAN_FOR_FLOATERS;
    }

    return GameState::ANIMATE_DEATHS;
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
            if (grid[x][y].state == BubbleState::DEAD)
            {
                emptySpace = true;
            }
            else if (grid[x][y].state == BubbleState::IDLE)
            {
                // If we have seen empty space before seeing this bubble then it must fall.
                if (emptySpace)
                {
                    // Copy this bubble and add it to the falling list.
                    Bubble faller = grid[x][y];
                    faller.state = FALLING;
                    fallingBubbles.push_back(faller);
                    // Mark the old grid position as dead.
                    grid[x][y].state = BubbleState::DEAD;
                    foundFloaters = true;
                }
            }
        }
    }
    if (foundFloaters)
    {
        return GameState::GRAVITY;
    }
    else
    {
        return GameState::BUBBLE_SPAWN;
    }
}

GameState gravity(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, const double secondsSinceLastUpdate)
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
			grid[col][gameOverRow].color = GHOST;
		}
		gameOverRow--;
	}
	return GameState::GAME_OVER;
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

static GameState applyGravity(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, const double secondsSinceLastUpdate)
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
        if (numMatches > 0 && (gridPos0.y == GRID_ROWS || (gridPos0.y >= 0 && grid[gridPos0.x][gridPos0.y].state == BubbleState::IDLE)))
        {
            hitPos = &gridPos0;
        }
        else if (numMatches == 2 && (gridPos1.y == GRID_ROWS || (gridPos1.y >= 0 && grid[gridPos1.x][gridPos1.y].state == BubbleState::IDLE)))
        {
            hitPos = &gridPos1;
        }

        if (hitPos != nullptr)
        {
            grid[hitPos->x][hitPos->y - 1].state = BubbleState::IDLE;
            grid[hitPos->x][hitPos->y - 1].color = it->color;
            grid[hitPos->x][hitPos->y - 1].bounceAmount = BOUNCE_HEIGHT;
            grid[hitPos->x][hitPos->y - 1].bounceDir = -1;            
            bounceList.push_back(&grid[hitPos->x][hitPos->y - 1]);

            fallingBubbles.erase(it++);

            // Check if the settle position of this bubble was the top row.
            if (hitPos->y - 1 == 0)
            {
                if (networkIsConnected())
                {
                    sendGameOver();
                }
                return GameState::GAME_OVER;
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
        return GameState::DROP_ENEMY_BUBBLES;
    }
    return GameState::GRAVITY;
}

// Finds size of a group of touching same coloured squares.
// Takes x, y input specifying grid location (in grid co-ordinates!) to start checking
// from.
static int findGroupSize(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], const uint8_t &x, const uint8_t &y, const BubbleColor &color)
{
    if (grid[x][y].color == color)
    {
        grid[x][y].state = BubbleState::DYING;
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

    if (grid[x][y].state == BubbleState::IDLE)
    {
        return findGroupSize(grid, x, y, color);
    }
    return 0;
}

static void printBubble(const Bubble &bubble)
{
    std::cout << "(" << bubble.playSpacePosition.x << "," << bubble.playSpacePosition.y << ") state: " << bubble.state << std::endl;
}