#ifndef MAPLOADER_H
#define MAPLOADER_H

#include <stddef.h>
#include "Tile.h"

void loadTerrain(Tile tiles[TILE_Z][TILE_X][TILE_Y], const unsigned char* buf, size_t buf_len);

#endif