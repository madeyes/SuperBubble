#ifndef STATE_HANDLERS_H
#define STATE_HANDLERS_H

GameState menu();
GameState spawnBubble(Bubble &mainBubble, Bubble &buddyBubble, std::list<Bubble> &fallingBubbles);
GameState controlPlayerBubbles(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS], Bubble &mainBubble, Bubble &buddyBubble /*, keys */);
GameState scanForVictims(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS]);
GameState animateDeaths(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS]);
GameState scanForFloaters(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles);
GameState gravity(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], std::list<Bubble> &fallingBubbles);
GameState gameOver();
	
#endif
