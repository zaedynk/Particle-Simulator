#include "shaders.h"
#include <initializer_list>

// Compile a single shader stage and report any errors.
static GLuint compileShader(GLenum type, const GLchar* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
    }
    return shader;
}

// Link a list of compiled shaders into a program and report any errors.
static GLuint linkProgram(std::initializer_list<GLuint> shaders) {
    GLuint program = glCreateProgram();
    for (GLuint shader : shaders)
        glAttachShader(program, shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed:\n" << infoLog << std::endl;
    }

    for (GLuint shader : shaders)
        glDeleteShader(shader);
    return program;
}

GLuint createShaderProgram(const GLchar* vertexSource, const GLchar* fragmentSource) {
    return linkProgram({
        compileShader(GL_VERTEX_SHADER, vertexSource),
        compileShader(GL_FRAGMENT_SHADER, fragmentSource),
    });
}

GLuint createComputeProgram(const GLchar* computeSource) {
    return linkProgram({ compileShader(GL_COMPUTE_SHADER, computeSource) });
}

// Draws each particle as a small point, colored by the value passed from the compute shader.
const GLchar* vertexShaderSource = R"(
#version 430 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;
out vec4 fragColor;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * position;
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

// Moves every particle each frame. While the left mouse button is held, particles
// are pulled toward a point in space using Newton's law of gravity, then colored by speed.
const GLchar* computeShaderSource = R"(
#version 430 core
layout (local_size_x = 256) in;

layout(std430, binding = 0) buffer Pos { vec4 positions[];  };
layout(std430, binding = 1) buffer Vel { vec4 velocities[]; };
layout(std430, binding = 2) buffer Col { vec4 colors[];     };

uniform float dt;
uniform vec3 pointOfMass;
uniform float isActive;   // 1 while the mouse pulls particles, else 0
uniform float isRunning;  // 1 while the simulation runs, 0 when paused

const float EPSILON = 0.001;
const float DRAG_COEF = log(0.998) * 176.0;

void main() {
    uint id = gl_GlobalInvocationID.x;
    if (id >= positions.length()) return;

    vec3 position = positions[id].xyz;
    vec3 velocity = velocities[id].xyz;

    // Newton's law of gravity toward the mouse-controlled point of mass.
    vec3 toMass = pointOfMass - position;
    float m2 = 176.0;  // mass of the point of mass
    float rSquared = max(dot(toMass, toMass), EPSILON * EPSILON);
    vec3 force = toMass * (m2 / rSquared); // toMass left un-normalized on purpose; it feels better
    vec3 acceleration = force * isRunning * isActive;

    velocity *= mix(1.0, exp(DRAG_COEF * dt), isRunning);             // drag
    position += (dt * velocity + 0.5 * acceleration * dt * dt) * isRunning; // Euler integration
    velocity += acceleration * dt;

    positions[id]  = vec4(position, 1.0);
    velocities[id] = vec4(velocity, 0.0);

    // Color by speed: slow particles are blue, fast particles shift to green/red.
    float speed = length(velocity);
    colors[id] = vec4(clamp(speed * 0.045, 0.0, 1.0),
                      clamp(speed * 0.08,  0.2, 1.0),
                      0.7 - clamp(speed * 0.045, 0.0, 1.0),
                      1.0);
}
)";
