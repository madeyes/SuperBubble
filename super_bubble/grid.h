#ifndef GRID_H
#define GRID_H

#include "defs.h"
#include "transforms.h"

void initGrid(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS]);
void renderGrid(Bubble (&grid)[GRID_COLUMNS][GRID_ROWS], const double secondsSinceLastUpdate);

#endif