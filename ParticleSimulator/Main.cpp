#include <vector>
#include "InputManager.h"
#include "Shader.h"

#include <GL/glut.h>

// Window dimensions
const GLuint Window_Width = 1920, Window_Height = 1080;

void initGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }
}

GLFWwindow* createWindow(int width, int height, const std::string& title) {
    GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    return window;
}

void initGLEW() {
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void setupGLFWCallbacks(GLFWwindow* window) {
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glViewport(0, 0, Window_Width, Window_Height);
}

void renderBitmapString(float x, float y, void* font, const char* string) {
    glWindowPos2f(x, y);
    for (const char* c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void renderText() {
    glDisable(GL_DEPTH_TEST);  // Disable depth test to render text on top of everything
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, Window_Width, Window_Height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);  // Set the text color to bright white

    // Render instructions
    renderBitmapString(10, 30, GLUT_BITMAP_HELVETICA_18, "Hold left click to move particles towards the mouse");
    renderBitmapString(10, 50, GLUT_BITMAP_HELVETICA_18, "Hit T to pause the simulation");
    renderBitmapString(10, 70, GLUT_BITMAP_HELVETICA_18, "Move around with WASD");
    renderBitmapString(10, 90, GLUT_BITMAP_HELVETICA_18, "Hit Escape to lock/hide the mouse");

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);  // Re-enable depth test
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Particle data structure
struct Particle {
    glm::vec4 position; // x, y, z, w
    glm::vec4 velocity; // x, y, z, w
    glm::vec4 color;    // r, g, b, a
};

void initParticles(std::vector<glm::vec4>& positions, std::vector<glm::vec4>& velocities, std::vector<glm::vec4>& colors, int numParticles) {
    positions.reserve(numParticles);
    velocities.reserve(numParticles);
    colors.reserve(numParticles);
    for (int i = 0; i < numParticles; ++i) {
        glm::vec4 position = glm::vec4(static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f,
            static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f,
            static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f, 1.0f);
        glm::vec4 velocity = glm::vec4(static_cast<float>(rand()) / RAND_MAX * 0.1f - 0.05f,
            static_cast<float>(rand()) / RAND_MAX * 0.1f - 0.05f,
            static_cast<float>(rand()) / RAND_MAX * 0.1f - 0.05f, 0.0f);

        float x = position.x;
        float y = position.y;
        float z = position.z;
        glm::vec4 color = glm::vec4((x + 1.0f) / 2.0f, (y + 1.0f) / 2.0f, (z + 1.0f) / 2.0f, 1.0f);

        positions.push_back(position);
        velocities.push_back(velocity);
        colors.push_back(color);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv); // Initialize GLUT
    initGLFW();
    GLFWwindow* window = createWindow(Window_Width, Window_Height, "Particle Simulator");
    initGLEW();
    setupGLFWCallbacks(window);

    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    GLuint computeProgram = createComputeShaderProgram(computeShaderSource);

    std::vector<glm::vec4> positions, velocities, colors;
    int numParticles = 1000000;
    initParticles(positions, velocities, colors, numParticles);

    GLuint posSSBO, velSSBO, colSSBO;
    glGenBuffers(1, &posSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, positions.size() * sizeof(glm::vec4), positions.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);

    glGenBuffers(1, &velSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, velocities.size() * sizeof(glm::vec4), velocities.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);

    glGenBuffers(1, &colSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, colSSBO);

    GLuint VAO, posVBO, colVBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &posVBO);
    glGenBuffers(1, &colVBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, colVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    glm::vec3 pointOfMass = glm::vec3(0.0f, 0.0f, 0.0f);
    bool isActive = false;

    // Use persistent mapping for positions, velocities, and colors
    glm::vec4* posPtr = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, positions.size() * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glm::vec4* velPtr = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, velocities.size() * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glm::vec4* colPtr = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, colors.size() * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)Window_Width / (float)Window_Height, 0.1f, 1000.0f);

        processInput(window, pointOfMass, isActive, cameraPos, view, projection);

        // Compute shader
        glUseProgram(computeProgram);
        glUniform1f(glGetUniformLocation(computeProgram, "dt"), deltaTime);
        glUniform3fv(glGetUniformLocation(computeProgram, "pointOfMass"), 1, glm::value_ptr(pointOfMass));
        glUniform1f(glGetUniformLocation(computeProgram, "isActive"), isActive ? 1.0f : 0.0f);
        glUniform1f(glGetUniformLocation(computeProgram, "isRunning"), isRunning ? 1.0f : 0.0f);
        glDispatchCompute((GLuint)(positions.size() / 256 + 1), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Clear color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render particles
        glUseProgram(shaderProgram);
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, posSSBO);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, colSSBO);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_POINTS, 0, numParticles);
        glBindVertexArray(0);

        // Render text
        glUseProgram(0);  // Use fixed-function pipeline for rendering text
        renderText();     // Render instructions

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &posVBO);
    glDeleteBuffers(1, &colVBO);
    glDeleteBuffers(1, &posSSBO);
    glDeleteBuffers(1, &velSSBO);
    glDeleteBuffers(1, &colSSBO);

    glfwTerminate();
    return 0;
}
