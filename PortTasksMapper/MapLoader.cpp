#include <iostream>
#include <string.h>
#include "MapLoader.h"

Tile*** MapLoader::loadTerrain(const unsigned char* buf, size_t buf_len)
{
	Tile*** tiles = new Tile**[4];
	for (int z = 0; z < 4; ++z)
	{
		tiles[z] = new Tile*[64];
		for (int x = 0; x < 64; ++x)
		{
			tiles[z][x] = new Tile[64];
		}
	}

	size_t offset = 0;

	for (int z = 0; z < 4; z++)
	{
		for (int x = 0; x < 64; x++)
		{
			for (int y = 0; y < 64; y++)
			{
				Tile* tile = &tiles[z][x][y];
				memset(tile, 0, sizeof(Tile));

				while (offset + 1 < buf_len)
				{
					unsigned short attribute = buf[offset] << 8 | buf[offset + 1];
					offset += 2;

					if (attribute == 0)
						break;
					else if (attribute == 1)
					{
						if (offset >= buf_len) return tiles;
						tile -> height = buf[offset++];
						break;
					}
					else if (attribute <= 49)
					{
						if (offset + 1 >= buf_len) return tiles;
						tile -> attrOpcode = attribute;
						tile -> overlayId = buf[offset] << 8 | buf[offset + 1];
						offset += 2;
						tile -> overlayPath = (attribute - 2) / 4;
						tile -> overlayRotation = (attribute - 2) & 3;
					}
					else if (attribute <= 81)
					{
						tile -> settings = attribute - 49;
					}
					else
					{
						tile -> underlayId = attribute - 81;
					}
				}
			}
		}
	}
	return tiles;
}
