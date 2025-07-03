#pragma once

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

#ifndef MAPRENDERER_H
#define MAPRENDERER_H

#include <GLFW/glfw3.h>
#include "Tile.h"
#include <string>

class MapRenderer {
public:
	MapRenderer(const std::string& filename, GLFWwindow* window);
	void initMap();
	void renderMap();
	void cleanupMap();
	static void uploadTileMesh(Tile*** tiles);
	void resetCamera();
	static void createShader();

	glm::vec3 intersectRayWithGround(glm::vec3 rayOrigin, glm::vec3 rayDir);
	glm::vec3 getRayFromMouse(GLFWwindow* window, const glm::mat4& view, const glm::mat4& projection);

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void setTiles(Tile*** newTiles);

private:
	std::string file;
	GLFWwindow* window;
	inline static int hoverTileX = -1;
	inline static int hoverTileY = -1;
	Tile*** tiles = nullptr;
};

#endif