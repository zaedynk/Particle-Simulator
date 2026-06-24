#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// Handles keyboard/mouse each frame: camera movement, pause toggle, and the
// mouse-driven "point of mass" that particles are pulled toward.
void processInput(GLFWwindow* window, glm::vec3& pointOfMass, bool& isActive,
                  const glm::vec3& camPos, const glm::mat4& view, const glm::mat4& projection);

// GLFW cursor callback: turns the camera when mouse look is enabled.
void mouseCallback(GLFWwindow* window, double xpos, double ypos);

// Camera state (defined in input.cpp).
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;
extern float fov;

// Simulation state.
extern bool isRunning;
