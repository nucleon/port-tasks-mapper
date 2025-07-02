#pragma once

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

#include <GLFW/glfw3.h>
#include "Tile.h"

void initMap();
void renderMap();
void cleanupMap();
void InitRS2Map(Tile tiles[TILE_Z][TILE_X][TILE_Y]);