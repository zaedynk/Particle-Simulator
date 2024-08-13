#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


GLuint compileShader(GLenum type, const GLchar* source);
GLuint createShaderProgram(const GLchar* vertexSource, const GLchar* fragmentSource);
GLuint createComputeShaderProgram(const GLchar* computeSource);

// Shader sources
extern const GLchar* vertexShaderSource;
extern const GLchar* fragmentShaderSource;
extern const GLchar* computeShaderSource;