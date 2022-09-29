#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace VulkanEngine {

	class VECamera
	{
	public:
		void SetOrthographicProjection(float left, float rigt, float top,
			float bottom, float near, float far);
		
		void SetPerspectiveProjection(float fovY, float aspect, float near, float far);

		void SetViewDirection(glm::vec3 position,
			glm::vec3 direction,
			glm::vec3 up = glm::vec3{ 0.0f, -1.0f, 0.0f });

		void SetViewTarget(glm::vec3 position,
			glm::vec3 target,
			glm::vec3 up = glm::vec3{ 0.0f, -1.0f, 0.0f });

		void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
		const glm::mat4& GetView() const { return m_ViewMatrix; }

	private:
		glm::mat4 m_ProjectionMatrix{ 1.0f };
		glm::mat4 m_ViewMatrix{1.0f};

	};
}