#pragma once
#include "VE_Model.h"

#include "glm/gtc/matrix_transform.hpp"

#include <memory>

namespace VulkanEngine {

	// Transform a component from object space into the shared world space (model transformation matrix)
	struct TransformComponent
	{
		glm::vec3 Translation{}; // Position offset
		glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };
		glm::vec3 Rotation;

		// Matrix corresponds to Translate * R.y * R.x * R.z * Scale transformation
		// Rotation cenvention uses Tait-Bryan angles with axis order Y(1), X(2), Z(3)
		glm::mat4 Mat4()
		{
			const float c3 = glm::cos(Rotation.z);
			const float s3 = glm::sin(Rotation.z);
			const float c2 = glm::cos(Rotation.x);
			const float s2 = glm::sin(Rotation.x);
			const float c1 = glm::cos(Rotation.y);
			const float s1 = glm::sin(Rotation.y);
			return glm::mat4{
				{
					Scale.x * (c1 * c3 + s1 * s2 * s3),
					Scale.x * (c2 * s3),
					Scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					Scale.y * (c3 * s1 * s2 - c1 * s3),
					Scale.y * (c2 * c3),
					Scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					Scale.z * (c2 * s1),
					Scale.z * (-s2),
					Scale.z * (c1 * c2),
					0.0f,
				},
				{Translation.x, Translation.y, Translation.z, 1.0f} };
		}
	};

	class VEGameObject
	{
	public:
		using id_t = unsigned int;

		static VEGameObject CreateGameObject()
		{
			static id_t currentId = 0;
			return VEGameObject(currentId++);
		}

		VEGameObject(const VEGameObject&) = delete;
		VEGameObject& operator=(const VEGameObject&) = delete;
		VEGameObject(VEGameObject&&) = default;
		VEGameObject& operator=(VEGameObject&&) = default;

		id_t GetId() { return m_Id; }

		std::shared_ptr<VEModel> m_Model{};
		glm::vec3 m_Color{};
		TransformComponent m_Transform{};

	private:
		VEGameObject(id_t objId) : m_Id{ objId } {}

	private:
		id_t m_Id;
	};
}