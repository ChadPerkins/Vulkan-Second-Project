#include "InputController.h"

#include <limits>

namespace VulkanEngine {

    void InputController::MoveInPlaneXZ(GLFWwindow* window, float deltaTime, VEGameObject& gameObject)
    {
        glm::vec3 rotate{ 0.0f };

        if (glfwGetKey(window, m_Keys.lookRight) == GLFW_PRESS)
            rotate.y += 1;

        if (glfwGetKey(window, m_Keys.lookLeft) == GLFW_PRESS)
            rotate.y -= 1;

        if (glfwGetKey(window, m_Keys.lookUp) == GLFW_PRESS)
            rotate.x += 1;

        if (glfwGetKey(window, m_Keys.lookDown) == GLFW_PRESS)
            rotate.x -= 1;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
            gameObject.m_Transform.Rotation += m_CameraSpeed * deltaTime * glm::normalize(rotate);

        // Limit pitch values between about +/- 85ish degrees
        gameObject.m_Transform.Rotation.x = glm::clamp(gameObject.m_Transform.Rotation.x, -1.5f, 1.5f);

        gameObject.m_Transform.Rotation.y = glm::mod(gameObject.m_Transform.Rotation.y, glm::two_pi<float>());

        float yaw = gameObject.m_Transform.Rotation.y;
        const glm::vec3 forward{ sin(yaw), 0.0f, cos(yaw) };
        const glm::vec3 right{ forward.z, 0.0f, -forward.x };
        const glm::vec3 up{ 0.0f, -1.0f, 0.0f };

        glm::vec3 movementDirection{ 0.0f };

        if (glfwGetKey(window, m_Keys.moveForward) == GLFW_PRESS)
            movementDirection += forward;

        if (glfwGetKey(window, m_Keys.moveBackward) == GLFW_PRESS)
            movementDirection -= forward;

        if (glfwGetKey(window, m_Keys.moveRight) == GLFW_PRESS)
            movementDirection += right;

        if (glfwGetKey(window, m_Keys.moveLeft) == GLFW_PRESS)
            movementDirection -= right;

        if (glfwGetKey(window, m_Keys.moveUp) == GLFW_PRESS)
            movementDirection += up;

        if (glfwGetKey(window, m_Keys.moveDown) == GLFW_PRESS)
            movementDirection -= up;

        if (glm::dot(movementDirection, movementDirection) > std::numeric_limits<float>::epsilon())
            gameObject.m_Transform.Translation += m_MovementSpeed * deltaTime * glm::normalize(movementDirection);

    }

}