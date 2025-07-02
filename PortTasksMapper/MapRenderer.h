#pragma once

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

#include <GLFW/glfw3.h>
void initMap();
void renderMap();
void cleanupMap();
void updateHighlight(GLFWwindow* window);