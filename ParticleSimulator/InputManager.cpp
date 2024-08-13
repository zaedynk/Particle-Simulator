#include "InputManager.h"

// Define the variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.5f;
float cameraSensitivity = 0.1f;
float yaw = -90.0f;
float pitch = 0.0f;
bool firstMouse = true;
bool cursorLocked = true;
double lastMouseX = 0.0;
double lastMouseY = 0.0;
float lastX = 1920.0f / 2.0;
float lastY = 1080.0f / 2.0;
float fov = 70.0f;
double lastEscapePressTime = 0;
double escapePressDelay = 0.2;  // 200 ms delay
bool isRunning = true;
double lastTPresstime = 0;
double TPressDelay = 0.2; // 200 ms delay

void processInput(GLFWwindow* window, glm::vec3& pointOfMass, bool& isActive, const glm::vec3& camPos, const glm::mat4& view, const glm::mat4& projection)
{
    double currentTime = glfwGetTime();

    // Toggle cursor lock with Escape key
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        if (currentTime - lastEscapePressTime > escapePressDelay)
        {
            cursorLocked = !cursorLocked; // Toggle the cursorLocked variable

            if (cursorLocked)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Lock the cursor
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Unlock the cursor
            }

            lastEscapePressTime = currentTime;  // Update the last escape press time
        }
    }

    // Camera movement speed based on whether Shift is pressed or not
    float currentCameraSpeed = cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        currentCameraSpeed *= 2.0f; // Move twice as fast when Shift is held

    // Camera movement based on key inputs
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += currentCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= currentCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * currentCameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * currentCameraSpeed;

    // Camera movement up and down with Space and Control
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += currentCameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_W) != GLFW_PRESS) // Move down only if not holding forward key
            cameraPos -= currentCameraSpeed * cameraUp;
    }

    // Toggle simulation running with 'T' key
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        if (currentTime - lastTPresstime > TPressDelay)
        {
            isRunning = !isRunning;
            lastTPresstime = currentTime;
        }
    }

    // Set point of mass with left mouse button
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // Normalize window coordinates to NDC
        float x = (2.0f * xpos) / width - 1.0f;
        float y = 1.0f - (2.0f * ypos) / height;
        float z = 1.0f;
        glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);

        // Convert to view space
        glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

        // Convert to world space
        glm::vec3 ray_wor = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));

        // Set the point of mass 25 units in the direction of the ray
        pointOfMass = camPos + ray_wor * 25.0f;
        isActive = true;
    }
    else
    {
        isActive = false;
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (!cursorLocked) // Check if the cursor is not locked
    {
        if (firstMouse)
        {
            lastMouseX = xpos;
            lastMouseY = ypos;
            firstMouse = false;
        }

        double xoffset = xpos - lastMouseX;
        double yoffset = lastMouseY - ypos;
        lastMouseX = xpos;
        lastMouseY = ypos;

        xoffset *= cameraSensitivity;
        yoffset *= cameraSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }
}
