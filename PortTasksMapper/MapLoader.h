#ifndef MAPLOADER_H
#define MAPLOADER_H

#include <cstddef>
#include "Tile.h"

class MapLoader
{
public:
	static Tile*** loadTerrain(const unsigned char* buf, size_t buf_len);
};

#endif