#pragma once
#include "VE_GameObject.h"
#include "VE_Window.h"

namespace VulkanEngine {

	class InputController
	{
    public:
        struct KeyMappings {
            int moveLeft        = GLFW_KEY_A;
            int moveRight       = GLFW_KEY_D;
            int moveForward     = GLFW_KEY_W;
            int moveBackward    = GLFW_KEY_S;
            int moveUp          = GLFW_KEY_E;
            int moveDown        = GLFW_KEY_Q;
            int lookLeft        = GLFW_KEY_LEFT;
            int lookRight       = GLFW_KEY_RIGHT;
            int lookUp          = GLFW_KEY_UP;
            int lookDown        = GLFW_KEY_DOWN;
        };

        void MoveInPlaneXZ(GLFWwindow* window, float deltaTime, VEGameObject& gameObject);

        KeyMappings m_Keys = {};

        float m_MovementSpeed   = { 3.0f };
        float m_CameraSpeed     = { 1.5f };
	};
}