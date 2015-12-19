#ifndef STATE_HANDLERS_H
#define STATE_HANDLERS_H

GameState menu();
GameState spawnBubble(std::list<Bubble> &fallingBubbles);
GameState controlPlayerBubbles(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles, Controls &controls);
GameState scanForVictims(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS]);
GameState animateDeaths(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS]);
GameState scanForFloaters(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles);
GameState gravity(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles);
GameState gameOver();
	
#endif
