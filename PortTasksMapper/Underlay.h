#pragma once
#include <glm/glm.hpp>
#include <cstdint>

struct UnderlayColor
{
	int id;
	uint32_t rgb;
};

struct OverlayColor
{
	int id;
	int rgb;
	int textureId;
	int secondaryRgb;
};

extern const UnderlayColor underlayColors[];
extern const OverlayColor overlayColors[];
extern const int underlayColorsCount;
extern const int overlayColorsCount;

glm::vec3 getUnderlayRGB(int id);
glm::vec3 getOverlayRGB(int id);