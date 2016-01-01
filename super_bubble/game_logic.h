#ifndef STATE_HANDLERS_H
#define STATE_HANDLERS_H

void resetGameLogic();
GameState spawnBubble(std::list<Bubble> &fallingBubbles, std::pair<BubbleColor, BubbleColor> &nextColors);
GameState controlPlayerBubbles(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, Controls &controls, const double secondsSinceLastUpdate);
GameState dropEnemyBubbles(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, uint8_t &numEnemyBubbles, const double secondsSinceLastUpdate);
GameState scanForVictims(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], uint32_t &score);
GameState animateDeaths(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS]);
GameState scanForFloaters(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles);
GameState gravity(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, const double secondsSinceLastUpdate);
GameState gameOver(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS]);

#endif
