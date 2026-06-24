#include <vector>
#include <cstdlib>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "input.h"
#include "shaders.h"
#include "stb_easy_font.h"

// Window size and how many particles to simulate.
// Lower PARTICLE_COUNT if the simulation runs slowly on your GPU.
const GLuint WINDOW_WIDTH = 1920, WINDOW_HEIGHT = 1080;
const int PARTICLE_COUNT = 1'000'000;

// Random float in [-1, 1).
static float randSigned() {
    return static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
}

// Create a window and an OpenGL context, exiting on failure.
static GLFWwindow* createWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Particle Simulator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    return window;
}

// Fill the buffers with random positions/velocities; color is derived from position.
static void initParticles(std::vector<glm::vec4>& positions,
                          std::vector<glm::vec4>& velocities,
                          std::vector<glm::vec4>& colors) {
    positions.reserve(PARTICLE_COUNT);
    velocities.reserve(PARTICLE_COUNT);
    colors.reserve(PARTICLE_COUNT);
    for (int i = 0; i < PARTICLE_COUNT; ++i) {
        glm::vec4 p(randSigned(), randSigned(), randSigned(), 1.0f);
        positions.push_back(p);
        velocities.push_back(glm::vec4(randSigned() * 0.05f, randSigned() * 0.05f, randSigned() * 0.05f, 0.0f));
        colors.push_back(glm::vec4((p.x + 1.0f) / 2.0f, (p.y + 1.0f) / 2.0f, (p.z + 1.0f) / 2.0f, 1.0f));
    }
}

// Upload a vector of vec4 into a shader storage buffer bound at the given index.
static GLuint createStorageBuffer(GLuint bindingIndex, const std::vector<glm::vec4>& data) {
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(glm::vec4), data.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, ssbo);
    return ssbo;
}

// Draw the on-screen control hints using stb_easy_font (no extra libraries needed).
static void renderText(GLFWwindow* window) {
    const char* lines[] = {
        "Hold left click: pull particles toward the cursor",
        "T: pause/resume    WASD: move    Space/Ctrl: up/down",
        "Esc: toggle mouse look",
    };

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glScalef(2.0f, 2.0f, 1.0f); // make the tiny bitmap font more readable

    static char buffer[60000];
    unsigned char white[4] = { 255, 255, 255, 255 };
    glEnableClientState(GL_VERTEX_ARRAY);
    float y = 5.0f;
    for (const char* line : lines) {
        int quads = stb_easy_font_print(5.0f, y, const_cast<char*>(line), white, buffer, sizeof(buffer));
        glVertexPointer(2, GL_FLOAT, 16, buffer);
        glDrawArrays(GL_QUADS, 0, quads * 4);
        y += 12.0f;
    }
    glDisableClientState(GL_VERTEX_ARRAY);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}

int main() {
    GLFWwindow* window = createWindow();

    GLuint renderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    GLuint computeProgram = createComputeProgram(computeShaderSource);

    std::vector<glm::vec4> positions, velocities, colors;
    initParticles(positions, velocities, colors);

    // Position/velocity/color live in shader storage buffers that the compute shader
    // updates in place and the vertex shader reads back to draw the points.
    GLuint posSSBO = createStorageBuffer(0, positions);
    GLuint velSSBO = createStorageBuffer(1, velocities);
    GLuint colSSBO = createStorageBuffer(2, colors);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, posSSBO);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, colSSBO);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glm::vec3 pointOfMass(0.0f);
    bool isActive = false;
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov),
            (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f);

        processInput(window, pointOfMass, isActive, cameraPos, view, projection);

        // Step the simulation on the GPU.
        glUseProgram(computeProgram);
        glUniform1f(glGetUniformLocation(computeProgram, "dt"), deltaTime);
        glUniform3fv(glGetUniformLocation(computeProgram, "pointOfMass"), 1, glm::value_ptr(pointOfMass));
        glUniform1f(glGetUniformLocation(computeProgram, "isActive"), isActive ? 1.0f : 0.0f);
        glUniform1f(glGetUniformLocation(computeProgram, "isRunning"), isRunning ? 1.0f : 0.0f);
        glDispatchCompute(PARTICLE_COUNT / 256 + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        // Draw the particles.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(renderProgram);
        glUniformMatrix4fv(glGetUniformLocation(renderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(renderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
        glBindVertexArray(0);

        // Draw the control hints on top using the fixed-function pipeline.
        glUseProgram(0);
        renderText(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &posSSBO);
    glDeleteBuffers(1, &velSSBO);
    glDeleteBuffers(1, &colSSBO);
    glfwTerminate();
    return 0;
}
