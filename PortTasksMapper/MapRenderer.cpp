#include "MapRenderer.h"
#include <vector>
#include <algorithm>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

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
float lastX = 400, lastY = 300;
bool firstMouse = true;
bool rotating = false;
bool panning = false;

glm::vec3 target = glm::vec3(31.0f, 0.0f, 31.0f);
glm::vec3 cameraPos;
glm::ivec2 highlightedTile = { -1, -1 };

void setYaw(float y) { yaw = y; }
void setPitch(float p) { pitch = std::clamp(p, -90.0f, 90.0f); }
void setDistance(float d) { distance = std::clamp(d, 100.0f, 300.0f); }

void resetCamera() 
{
	setYaw(-90.0f);
	setPitch(-70.0f);
	setDistance(180.0f);
}

void createShader() {
	const char* vertSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;

        uniform mat4 uVP;
        uniform vec4 uColor;

        out vec3 vColor;

        void main() {
            gl_Position = uVP * vec4(aPos, 1.0);
            vColor = (aColor == vec3(0.0)) ? uColor.rgb : aColor;
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


void initMap() 
{
	std::vector<Vertex> verts;
	float tileSize = 4.0f;

	for (int y = 0; y < MAP_HEIGHT; ++y) 
	{
		for (int x = 0; x < MAP_WIDTH; ++x) 
		{
			int type = (x + y) % 3;
			float r = 0.3f, g = 0.3f, b = 0.3f;
			switch (type) {
			case 0: r = 0.1f; g = 0.5f; b = 0.2f; break;
			case 1: r = 0.1f; g = 0.5f; b = 0.2f; break;
			case 2: r = 0.1f; g = 0.5f; b = 0.2f; break;
			}

			float left = -1.0f + x * tileSize;
			float right = left + tileSize;
			float top = 1.0f - y * tileSize;
			float bottom = top - tileSize;

			float height = 0.0f;
			verts.push_back({ left, height, -top, r, g, b });
			verts.push_back({ right, height, -top, r, g, b });
			verts.push_back({ right, height, -bottom, r, g, b });

			verts.push_back({ left, height, -top, r, g, b });
			verts.push_back({ right, height, -bottom, r, g, b });
			verts.push_back({ left, height, -bottom, r, g, b });
		}
	}

	createShader();

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

	void renderMap() 
	{
		glUseProgram(shaderProgram);

		int width, height;
		glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
		float aspect = width / (float)height;

		glm::mat4 projection = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 500.0f);

		// reasonable view
		setYaw(-90.f);
		setPitch(34.0f);

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
		glDrawArrays(GL_TRIANGLES, 0, MAP_WIDTH * MAP_HEIGHT * 6);



		if (highlightedTile.x != -1)
		{
			float tileSize = 4.0f;

			float x = highlightedTile.x * tileSize;
			float z = highlightedTile.y * tileSize;
			float y = 0.05f;

			float left = -1.0f + highlightedTile.x * tileSize;
			float right = left + tileSize;
			float top = 1.0f - highlightedTile.y * tileSize;
			float bottom = top - tileSize;

			float highlightVerts[] = {
				left, y, -top,
				right, y, -top,
				right, y, -bottom,
				left, y, -top,
				right, y, -bottom,
				left, y, -bottom
			};

			GLuint highlightVBO, highlightVAO;
			glGenVertexArrays(1, &highlightVAO);
			glGenBuffers(1, &highlightVBO);

			glBindVertexArray(highlightVAO);
			glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(highlightVerts), highlightVerts, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glDisableVertexAttribArray(1); // no color
			glUniform4f(glGetUniformLocation(shaderProgram, "uColor"), 1.0f, 1.0f, 0.0f, 1.0f);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDeleteVertexArrays(1, &highlightVAO);
			glDeleteBuffers(1, &highlightVBO);
		}

	}

	void cleanupMap() 
	{
		glDeleteProgram(shaderProgram);
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) 
	{
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			rotating = (action == GLFW_PRESS);

		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			panning = (action == GLFW_PRESS);

		if (action == GLFW_RELEASE)
			firstMouse = true;
	}

	void mouse_callback(GLFWwindow* window, double xpos, double ypos) 
	{
		glfwGetCursorPos(window, &xpos, &ypos);

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		float ndcX = (2.0f * xpos) / width - 1.0f;
		float ndcY = 1.0f - (2.0f * ypos) / height;

		if (!rotating && !panning)
			return;

		if (firstMouse) 
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = static_cast<float>(xpos - lastX);
		float yoffset = static_cast<float>(ypos - lastY);
		lastX = static_cast<float>(xpos);
		lastY = static_cast<float>(ypos);

		if (rotating) 
		{
			float sensitivity = 0.2f;
			pitch -= yoffset * sensitivity;
			setPitch(pitch);
		}
		else if (panning) 
		{
			float panSpeed = 0.005f * distance;
			target.x -= xoffset * panSpeed;
			target.z -= yoffset * panSpeed;
		}
	}

	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) 
	{
		distance -= static_cast<float>(yoffset) * 2.0f;
		setDistance(distance);
	}

	void updateHighlight(GLFWwindow* window)
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		float ndcX = (2.0f * xpos) / width - 1.0f;
		float ndcY = 1.0f - (2.0f * ypos) / height;

		glm::mat4 projection = glm::perspective(glm::radians(50.0f), (float)width / height, 0.1f, 500.0f);
		glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0, 1, 0));
		glm::mat4 invVP = glm::inverse(projection * view);

		glm::vec4 rayStartNDC = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
		glm::vec4 rayEndNDC = glm::vec4(ndcX, ndcY, 1.0f, 1.0f); // not 0.0f

		glm::vec4 rayStartWorld = invVP * rayStartNDC;
		rayStartWorld /= rayStartWorld.w;

		glm::vec4 rayEndWorld = invVP * rayEndNDC;
		rayEndWorld /= rayEndWorld.w;

		glm::vec3 origin = glm::vec3(rayStartWorld);
		glm::vec3 rayDir = glm::normalize(glm::vec3(rayEndWorld - rayStartWorld));

		if (rayDir.y >= 0.0f)
		{
			highlightedTile = { -1, -1 };
			return;
		}

		float t = -origin.y / rayDir.y;
		glm::vec3 intersect = origin + rayDir * t;

		float tileSize = 4.0f;
		int tileX = static_cast<int>(intersect.x / tileSize);
		int tileZ = static_cast<int>(intersect.z / tileSize); // remove the minus!

		if (tileX >= 0 && tileX < MAP_WIDTH && tileZ >= 0 && tileZ < MAP_HEIGHT)
			highlightedTile = { tileX, tileZ };
		else
			highlightedTile = { -1, -1 };
	}

