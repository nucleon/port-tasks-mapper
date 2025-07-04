#include <vector>
#include <algorithm>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include "Tile.h"
#include "Underlay.h"
#include "MapRenderer.h"

struct Vertex {
	float x, y, z;
	float r, g, b;
};

static GLuint vao = 0, vbo = 0;
static GLuint shaderProgram = 0;

const int MAP_WIDTH = 64;
const int MAP_HEIGHT = 64;

extern GLuint shaderProgram;
extern GLuint vao;

float yaw = -90.0f;
float pitch = -70.0f;
float distance = 180.0f;

int guiHoverTileX = -1;
int guiHoverTileY = -1;
Tile*** guitiles = nullptr;

float lastX = 400, lastY = 300;
bool firstMouse = true;
bool rotating = false;
bool panning = false;

glm::vec3 target = glm::vec3(31.0f, 0.0f, 31.0f);
glm::vec3 cameraPos;

void setYaw(float y) { yaw = y; }
void setPitch(float p) { pitch = std::clamp(p, -90.0f, 90.0f); }
void setDistance(float d) { distance = std::clamp(d, 100.0f, 300.0f); }

float cornerHeights[65][65] = {};

void MapRenderer::resetCamera() {
	setYaw(90.f);
	setPitch(45.0f);
	setDistance(180.0f);
}

void MapRenderer::createShader() {
	const char* vertSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;
        uniform mat4 uVP;
        out vec3 vColor;
        void main() {
            gl_Position = uVP * vec4(aPos, 1.0);
            vColor = aColor;
        }
    )";

	const char* fragSrc = R"(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vColor, 1.0);
        }
    )";

	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vertSrc, nullptr);
	glCompileShader(vert);

	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &fragSrc, nullptr);
	glCompileShader(frag);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vert);
	glAttachShader(shaderProgram, frag);
	glLinkProgram(shaderProgram);

	glDeleteShader(vert);
	glDeleteShader(frag);
}

void MapRenderer::uploadTileMesh(Tile*** tiles) {
	std::vector<Vertex> verts;
	float tileSize = 4.0f;
	float heightScale = 0.300f;

	for (int y = 0; y <= 64; ++y) {
		for (int x = 0; x <= 64; ++x) {
			float sum = 0.f;
			int count = 0;
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int tx = x + dx;
					int ty = y + dy;
					if (tx >= 0 && tx < 64 && ty >= 0 && ty < 64) {
						sum += tiles[0][tx][ty].height;
						count++;
					}
				}
			}
			cornerHeights[x][y] = (count > 0 ? sum / count : 0.f) * heightScale;
		}
	}

	for (int y = 0; y < 64; ++y) {
		for (int x = 0; x < 64; ++x) {
			Tile& tile = tiles[0][x][y];
			glm::vec3 color;
			if (tile.overlayId != 0)
				color = getOverlayRGB(tile.overlayId);
			else
				color = getUnderlayRGB(tile.underlayId);
			float r = color.r, g = color.g, b = color.b;

			float h00 = cornerHeights[x][y];
			float h10 = cornerHeights[x + 1][y];
			float h11 = cornerHeights[x + 1][y + 1];
			float h01 = cornerHeights[x][y + 1];

			float left = x * tileSize;
			float right = (x + 1) * tileSize;
			float top = (MAP_HEIGHT - 1 - y) * tileSize;
			float bottom = (MAP_HEIGHT - 1 - (y + 1)) * tileSize;

			verts.push_back({ left,  h00, top,    r, g, b });
			verts.push_back({ right, h10, top,    r, g, b });
			verts.push_back({ right, h11, bottom, r, g, b });

			verts.push_back({ left,  h00, top,    r, g, b });
			verts.push_back({ right, h11, bottom, r, g, b });
			verts.push_back({ left,  h01, bottom, r, g, b });
		}
	}

	vertexCount = verts.size();
	if (vao) glDeleteVertexArrays(1, &vao);
	if (vbo) glDeleteBuffers(1, &vbo);

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	createShader();

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void MapRenderer::renderMap() {
	glUseProgram(shaderProgram);

	int width, height;
	glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
	float aspect = width / (float)height;

	glm::mat4 projection = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 500.0f);

	//setYaw(90.f);
	//setPitch(45.0f);

	float yawRad = glm::radians(yaw);
	float pitchRad = glm::radians(pitch);

	cameraPos = target + glm::vec3(
		distance * cos(pitchRad) * cos(yawRad),
		distance * sin(pitchRad),
		distance * cos(pitchRad) * sin(yawRad)
	);

	glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0, 1, 0));
	glm::mat4 vp = projection * view;

	GLuint loc = glGetUniformLocation(shaderProgram, "uVP");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(vp));

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);

	glm::vec3 ray = getRayFromMouse(window, view, projection);
	glm::vec3 hit = intersectRayWithGround(cameraPos, ray);

	const float tileSize = 4.0f;
	int tileX = static_cast<int>(std::floor(hit.x / tileSize));
	int tileY = static_cast<int>(std::floor(hit.z / tileSize));
	tileX = std::clamp(tileX, 0, 63);
	tileY = std::clamp(tileY, 0, 63);
	tileY = (MAP_HEIGHT - 1) - tileY;


	int hoverTileX = tileX;
	int hoverTileY = tileY;

	guiHoverTileX = hoverTileX;
	guiHoverTileY = hoverTileY;
	guitiles = tiles;

	if (hoverTileX >= 0 && hoverTileX < 64 && hoverTileY >= 0 && hoverTileY < 64) {
		float tileSize = 4.0f;
		// Use same smoothing as mesh!
		float h00 = cornerHeights[hoverTileX][hoverTileY];
		float h10 = cornerHeights[hoverTileX + 1][hoverTileY];
		float h11 = cornerHeights[hoverTileX + 1][hoverTileY + 1];
		float h01 = cornerHeights[hoverTileX][hoverTileY + 1];

		float left = hoverTileX * tileSize;
		float right = (hoverTileX + 1) * tileSize;
		float top = (MAP_HEIGHT - 1 - hoverTileY) * tileSize;
		float bottom = (MAP_HEIGHT - 1 - (hoverTileY + 1)) * tileSize;

		glm::vec3 highlightColor = { 1.0f, 1.0f, 0.0f };

		Vertex highlightVerts[6] = {
			{ left,  h00, top,    highlightColor.r, highlightColor.g, highlightColor.b },
			{ right, h10, top,    highlightColor.r, highlightColor.g, highlightColor.b },
			{ right, h11, bottom, highlightColor.r, highlightColor.g, highlightColor.b },

			{ left,  h00, top,    highlightColor.r, highlightColor.g, highlightColor.b },
			{ right, h11, bottom, highlightColor.r, highlightColor.g, highlightColor.b },
			{ left,  h01, bottom, highlightColor.r, highlightColor.g, highlightColor.b }
		};

		GLuint tempVAO, tempVBO;
		glGenVertexArrays(1, &tempVAO);
		glGenBuffers(1, &tempVBO);
		glBindVertexArray(tempVAO);
		glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(highlightVerts), highlightVerts, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDeleteBuffers(1, &tempVBO);
		glDeleteVertexArrays(1, &tempVAO);
	}
	std::cout << "Tile: " << tileX << ", " << tileY << "\n";
}

glm::vec3 MapRenderer::intersectRayWithGround(glm::vec3 rayOrigin, glm::vec3 rayDir) {
	float t = -rayOrigin.y / rayDir.y;
	return rayOrigin + rayDir * t;
}

glm::vec3 MapRenderer::getRayFromMouse(GLFWwindow* window, const glm::mat4& view, const glm::mat4& projection) {
	int winWidth, winHeight;
	glfwGetFramebufferSize(window, &winWidth, &winHeight);
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	float x = (2.0f * (float)mouseX) / winWidth - 1.0f;
	float y = 1.0f - (2.0f * (float)mouseY) / winHeight;
	float z = 1.0f;
	glm::vec3 ray_nds = glm::vec3(x, y, z);
	glm::vec4 ray_clip = glm::vec4(ray_nds, 1.0);
	glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
	glm::vec3 ray_world = glm::vec3(glm::inverse(view) * ray_eye);
	return glm::normalize(ray_world);
}

void MapRenderer::cleanupMap() {
	glDeleteProgram(shaderProgram);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}
std::vector<std::string> savedPoints;

void MapRenderer::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
		rotating = (action == GLFW_PRESS);
	if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		panning = (action == GLFW_PRESS);
	if (action == GLFW_RELEASE)
		firstMouse = true;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		int absX = 50 * 64 + guiHoverTileX;
		int absY = 50 * 64 + guiHoverTileY;

		char buf[64];
		snprintf(buf, sizeof(buf), "WorldPoint(%d, %d, 0),", absX, absY);

		savedPoints.push_back(std::string(buf));
	}
}

void MapRenderer::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	glfwGetCursorPos(window, &xpos, &ypos);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float ndcX = (2.0f * xpos) / width - 1.0f;
	float ndcY = 1.0f - (2.0f * ypos) / height;
	if (!rotating && !panning)
		return;
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = static_cast<float>(xpos - lastX);
	float yoffset = static_cast<float>(ypos - lastY);
	lastX = static_cast<float>(xpos);
	lastY = static_cast<float>(ypos);
	if (rotating) {
		float sensitivity = 0.2f;
		pitch -= yoffset * sensitivity;
		setPitch(pitch);
	}
	else if (panning) {
		float panSpeed = 0.005f * distance;
		target.x -= xoffset * panSpeed;
		target.z -= yoffset * panSpeed;
	}
}

void MapRenderer::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	distance -= static_cast<float>(yoffset) * 2.0f;
	setDistance(distance);
}

MapRenderer::MapRenderer(const std::string& filename, GLFWwindow* window)
	: file(filename), window(window) {
	resetCamera();
}

void MapRenderer::setTiles(Tile*** newTiles) {
	tiles = newTiles;
}
