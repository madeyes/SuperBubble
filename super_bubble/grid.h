#ifndef GRID_H
#define GRID_H

#include "defs.h"
#include "transforms.h"

void initGrid(Bubble(&grid)[GRID_COLUMNS][GRID_ROWS]);
void renderGrid(Bubble const (&grid)[GRID_COLUMNS][GRID_ROWS]);

#endif