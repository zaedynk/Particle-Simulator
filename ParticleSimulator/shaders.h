#pragma once
#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// Compile/link helpers.
GLuint createShaderProgram(const GLchar* vertexSource, const GLchar* fragmentSource);
GLuint createComputeProgram(const GLchar* computeSource);

// GLSL source strings (defined in shaders.cpp).
extern const GLchar* vertexShaderSource;
extern const GLchar* fragmentShaderSource;
extern const GLchar* computeShaderSource;
