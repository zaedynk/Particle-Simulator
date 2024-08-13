#include "Shader.h"

GLuint compileShader(GLenum type, const GLchar* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

GLuint createShaderProgram(const GLchar* vertexSource, const GLchar* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint createComputeShaderProgram(const GLchar* computeSource) {
    GLuint computeShader = compileShader(GL_COMPUTE_SHADER, computeSource);

    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(computeProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(computeShader);

    return computeProgram;
}


// Shader sources
const GLchar* vertexShaderSource = R"(
#version 430 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;
out vec4 fragColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * position;
    fragColor = color;
    gl_PointSize = 1.1;
}
)";

const GLchar* fragmentShaderSource = R"(
#version 430 core
in vec4 fragColor;
out vec4 color;
void main() {
    color = fragColor;
}
)";

const GLchar* computeShaderSource = R"(
#version 430 core
layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer Pos {
    vec4 positions[];
};

layout(std430, binding = 1) buffer Vel {
    vec4 velocities[];
};

layout(std430, binding = 2) buffer Col {
    vec4 colors[];
};

uniform float dt;
uniform vec3 pointOfMass;
uniform float isActive;
uniform float isRunning;

const float EPSILON = 0.001;
const float DRAG_COEF = log(0.998) * 176.0;

void main() {
    uint id = gl_GlobalInvocationID.x;

    vec3 position = positions[id].xyz;
    vec3 velocity = velocities[id].xyz;

    vec3 toMass = pointOfMass - position;
    
    // Implementation of Newton's law of gravity
    float m1 = 1.0;   // constant particle mass
    float m2 = 176.0; // (user controlled) pointOfMass mass
    float G  = 1.0;   // gravitational constant 
    float m1_m2 = m1 * m2; // mass of both objects multiplied
    float rSquared = max(dot(toMass, toMass), EPSILON * EPSILON); // distance between objects squared
    // Technically toMass would have to be normalized but feels better without
    vec3 force = toMass * (G * ((m1_m2) / rSquared));
    
    // acceleration = force / m. 
    vec3 acceleration = (force * isRunning * isActive) / m1;

    velocity *= mix(1.0, exp(DRAG_COEF * dt), isRunning); // Apply drag
    position += (dt * velocity + 0.5 * acceleration * dt * dt) * isRunning; // Euler integration
    velocity += acceleration * dt;

    positions[id] = vec4(position, 1.0);
    velocities[id] = vec4(velocity, 0.0);

    // Update color based on velocity
    float speed = length(velocity);
    colors[id] = vec4(clamp(speed * 0.045, 0.0, 1.0), // Red channel
                      clamp(speed * 0.08, 0.2, 1.0), // Green channel
                      0.7 - clamp(speed * 0.045, 0.0, 1.0), // Blue channel
                      1.0); // Alpha channel
}
)";