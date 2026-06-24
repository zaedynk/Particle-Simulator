#include "input.h"

// Camera state shared with main.cpp.
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
float fov = 70.0f;
bool isRunning = true;

// Internal state for input handling.
namespace {
    float cameraSpeed = 0.5f;
    float cameraSensitivity = 0.1f;
    float yaw = -90.0f;
    float pitch = 0.0f;
    bool firstMouse = true;
    bool mouseLook = false;        // false = normal cursor, true = camera look
    double lastMouseX = 0.0, lastMouseY = 0.0;

    const double KEY_REPEAT_DELAY = 0.2; // seconds between toggle presses
    double lastEscapePress = 0.0;
    double lastTPress = 0.0;
}

void processInput(GLFWwindow* window, glm::vec3& pointOfMass, bool& isActive,
                  const glm::vec3& camPos, const glm::mat4& view, const glm::mat4& projection) {
    double now = glfwGetTime();

    // Esc toggles mouse look (camera rotation vs. a normal cursor).
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && now - lastEscapePress > KEY_REPEAT_DELAY) {
        mouseLook = !mouseLook;
        glfwSetInputMode(window, GLFW_CURSOR, mouseLook ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        lastEscapePress = now;
    }

    // Move faster while Shift is held.
    float speed = cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        speed *= 2.0f;

    glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= speed * right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += speed * right;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) cameraPos += speed * cameraUp;
    if ((glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
        && glfwGetKey(window, GLFW_KEY_W) != GLFW_PRESS)
        cameraPos -= speed * cameraUp;

    // T pauses/resumes the simulation.
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && now - lastTPress > KEY_REPEAT_DELAY) {
        isRunning = !isRunning;
        lastTPress = now;
    }

    // Hold left click: cast a ray through the cursor and set the point of mass 25 units along it.
    isActive = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (isActive) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // Window pixel -> normalized device coords -> view space -> world space.
        glm::vec4 rayClip(2.0f * (float)xpos / width - 1.0f, 1.0f - 2.0f * (float)ypos / height, -1.0f, 1.0f);
        glm::vec4 rayEye = glm::inverse(projection) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
        glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));

        pointOfMass = camPos + rayWorld * 25.0f;
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouseLook) return;

    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    double xoffset = (xpos - lastMouseX) * cameraSensitivity;
    double yoffset = (lastMouseY - ypos) * cameraSensitivity;
    lastMouseX = xpos;
    lastMouseY = ypos;

    yaw += (float)xoffset;
    pitch = glm::clamp(pitch + (float)yoffset, -89.0f, 89.0f);

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}
