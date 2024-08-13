#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

void processInput(GLFWwindow* window, glm::vec3& pointOfMass, bool& isActive, const glm::vec3& camPos, const glm::mat4& view, const glm::mat4& projection);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);

// Variables
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;
extern float cameraSpeed;
extern float cameraSensitivity;
extern float yaw;
extern float pitch;
extern bool firstMouse;
extern bool cursorLocked;
extern double lastMouseX;
extern double lastMouseY;
extern float lastX;
extern float lastY;
extern float fov;

extern bool isRunning;



