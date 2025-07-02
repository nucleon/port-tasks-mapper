#ifndef TILE_H
#define TILE_H

#define TILE_Z 4
#define TILE_X 64
#define TILE_Y 64

typedef struct
{
	int height;
	int attrOpcode;
	unsigned char settings;
	short overlayId;
	unsigned char overlayPath;
	unsigned char overlayRotation;
	short underlayId;
} Tile;

#endif // TILE_H