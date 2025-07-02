#pragma once
#include <glm/glm.hpp>
#include <cstdint>

struct UnderlayColor
{
	int id;
	uint32_t rgb;
};

extern const UnderlayColor underlayColors[];
extern const int underlayColorsCount;

glm::vec3 getUnderlayRGB(int id);