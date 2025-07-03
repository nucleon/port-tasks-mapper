#define GLFW_INCLUDE_NONE

#include <iostream>
#include <windows.h>
#include "pch.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Tile.h"
#include "MapLoader.h"
#include "MapRenderer.h"
#include "Utils.h"

void hideConsole();

void drawUI();

extern float yaw;
extern float pitch;
extern float distance;
extern glm::vec3 target;
extern glm::vec3 cameraPos;

extern int guiHoverTileX;
extern int guiHoverTileY;
extern Tile*** guitiles;

static bool showWireframe = true;

int main()
{

	if (!glfwInit())
	{
		std::cerr << "Failed to init GLFW\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Port Tasks Mapping Tool - OpenGL 3.3", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create window\n";
		glfwTerminate();
		return -1;
	}

	MapRenderer renderer("data/m50_50.dat", window);

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to init GLAD\n";
		return -1;
	}

	glfwSetCursorPosCallback(window, MapRenderer::mouse_callback);
	glfwSetMouseButtonCallback(window, MapRenderer::mouse_button_callback);
	glfwSetScrollCallback(window, MapRenderer::scroll_callback);

	size_t bufSize;
	unsigned char* buf = loadFileBytes("m50_50.dat", &bufSize);
	if (buf)
	{
		Tile*** tiles = MapLoader::loadTerrain(buf, bufSize);
		renderer.uploadTileMesh(tiles);
		renderer.setTiles(tiles);
		free(buf);
	}
	else
	{
		std::cerr << "Failed to load map data\n";
	}

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	ImGui_ImplOpenGL3_Init("#version 330");

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		renderer.renderMap();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		drawUI();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	renderer.cleanupMap();
	return 0;
}

void hideConsole()
{
#ifndef _DEBUG
	HWND console = GetConsoleWindow();
	ShowWindow(console, SW_HIDE);
#endif

}

void drawUI()
{
	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowSize(ImVec2(280, 160));
	ImGui::Begin("Port Tasks", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	ImGui::Text("Tool Options");
	if (ImGui::Button("Reload Map"))
	{
		std::cout << "Button was clicked!\n";
	}
	if (ImGui::Checkbox("Wireframe", &showWireframe))
		glPolygonMode(GL_FRONT_AND_BACK, showWireframe ? GL_LINE : GL_FILL);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::SliderFloat("Yaw", &yaw, -180.0f, 180.0f, "%.0f"))
		yaw = roundf(yaw);

	if (ImGui::SliderFloat("Pitch", &pitch, -89.0f, 89.0f, "%.0f"))
		pitch = roundf(pitch);

	if (ImGui::SliderFloat("Distance", &distance, 60.0f, 300.0f, "%.0f"))
		distance = roundf(distance);

	ImGui::End();

	ImVec2 window_size(220, 110);
	ImVec2 window_pos(800 - window_size.x - 10, 10);

	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

	ImGui::SetNextWindowBgAlpha(0.5f);

	ImGui::Begin("Tile Info", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

	if (guiHoverTileX >= 0 && guiHoverTileX < 64 && guiHoverTileY >= 0 && guiHoverTileY < 64 && guitiles)
	{
		const Tile& tile = guitiles[0][guiHoverTileX][guiHoverTileY];
		const int regionX = 50;
		const int regionY = 50;
		const int worldX = regionX * 64 + guiHoverTileX;
		const int worldY = regionY * 64 + guiHoverTileY;

		ImGui::Text("Tile Info:");
		ImGui::Text("Map X: %d   Y: %d", guiHoverTileX, guiHoverTileY);
		ImGui::Text("World X: %d   Y: %d", worldX, worldY);
		ImGui::Text("Overlay ID: %d", tile.overlayId);
		ImGui::Text("Underlay ID: %d", tile.underlayId);
	}
	else
	{
		ImGui::Text("No tile hovered.");
	}

	ImGui::End();

}
