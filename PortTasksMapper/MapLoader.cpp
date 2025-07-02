#include <string.h>
#include "MapLoader.h"
#include <iostream>

void loadTerrain(Tile tiles[TILE_Z][TILE_X][TILE_Y], const unsigned char* buf, size_t buf_len)
{
	size_t offset = 0;

	for (int z = 0; z < TILE_Z; z++)
	{
		for (int x = 0; x < TILE_X; x++)
		{
			for (int y = 0; y < TILE_Y; y++)
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
						if (offset >= buf_len) return;
						tile->height = buf[offset++];
						break;
					}
					else if (attribute <= 49)
					{
						if (offset + 1 >= buf_len) return;
						tile->attrOpcode = attribute;
						tile->overlayId = buf[offset] << 8 | buf[offset + 1];
						offset += 2;
						tile->overlayPath = (attribute - 2) / 4;
						tile->overlayRotation = (attribute - 2) & 3;
					}
					else if (attribute <= 81)
					{
						tile->settings = attribute - 49;
					}
					else
					{
						tile->underlayId = attribute - 81;
					}
				}
			}
		}
	}
}
