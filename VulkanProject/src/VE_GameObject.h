#pragma once
#include "VE_Model.h"

#include "glm/gtc/matrix_transform.hpp"

#include <memory>
#include <unordered_map>

namespace VulkanEngine {

	// Transform a component from object space into the shared world space (model transformation matrix)
	struct TransformComponent
	{
		glm::vec3 Translation{}; // Position offset
		glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };
		glm::vec3 Rotation;

		// Matrix corresponds to Translate * R.y * R.x * R.z * Scale transformation
		// Rotation cenvention uses Tait-Bryan angles with axis order Y(1), X(2), Z(3)
		glm::mat4 Mat4();
		glm::mat3 NormalMatrix();
	};

	class VEGameObject
	{
	public:
		using id_t	= unsigned int;
		using Map	= std::unordered_map<id_t, VEGameObject>;

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