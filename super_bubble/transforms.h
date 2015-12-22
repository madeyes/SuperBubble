#ifndef TRANSFORMS_H
#define TRANSFORMS_H

#include "defs.h"

/* The coordinate spaces in the game are:
 *  Window space - the entire game window - this is the space everything is drawn in.
 *  Play space - the area where the game grid is displayed - this is the space where the bubbles are translated to allow them to move smoothly.
 *  Grid space - the grid where a bubble can be dispayed in each grid square, where the grid squares are GRID_SIZE width (which is specified in play space).
 */

 // These methods return true if the transformation is valid.
bool gridSpaceToPlaySpace(const glm::ivec2 gridPosition, glm::ivec2 &playPosition);
bool playSpaceToGridSpace(const glm::ivec2 playPosition, glm::ivec2 &gridPosition);
bool windowSpaceToPlaySpace(const glm::uvec2 windowPosition, glm::ivec2 &playPosition);
bool playSpaceToWindowSpace(const glm::ivec2 playPosition, glm::uvec2 &windowPosition);
uint8_t playSpaceToNearestVerticalGrid(const glm::ivec2 playPosition, glm::ivec2 &gridPosition0, glm::ivec2 &gridPosition1);

#endif