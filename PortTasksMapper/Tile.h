#ifndef TILE_H
#define TILE_H

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