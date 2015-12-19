#ifndef COLLISION_H
#define COLLISION_H
#include "defs.h"

bool canGoLeft(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS], const Bubble &playerBubble0, const Bubble &playerBubble1);
bool canGoRight(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS], const Bubble &playerBubble0, const Bubble &playerBubble1);

#endif
