#include "Transform.hpp"

#include "glm/gtx/euler_angles.hpp"
#include <glm/gtx/quaternion.hpp>

namespace EVA
{
	Transform::Transform()
	{
		m_Parent = nullptr;
        SetPosition(m_LocalPosition);
        SetOrientation(m_LocalOrientation);
        SetScale(m_LocalScale);
	}

	void Transform::Translate(const glm::vec3 offset)
	{
		m_LocalPosition += offset;
		UpdateModelMatrix();
	}

	void Transform::TranslateXY(const glm::vec2 offset)
	{
		m_LocalPosition += glm::vec3(offset.x, offset.y, 0.0f);
		UpdateModelMatrix();
	}

	void Transform::TranslateXZ(const glm::vec2 offset)
	{
		m_LocalPosition += glm::vec3(offset.x, 0.0f, offset.y);
		UpdateModelMatrix();
	}

	void Transform::SetPosition(const glm::vec3 newPosition)
	{
		m_LocalPosition = newPosition;
		UpdateModelMatrix();
	}

	void Transform::SetPositionXY(const glm::vec2 newPosition)
	{
		m_LocalPosition = glm::vec3(newPosition.x, newPosition.y, m_LocalPosition.z);
		UpdateModelMatrix();
	}

	void Transform::SetPositionXZ(const glm::vec2 newPosition)
	{
		m_LocalPosition = glm::vec3(newPosition.x, m_LocalPosition.y, newPosition.y);
		UpdateModelMatrix();
	}

	void Transform::Rotate(const glm::quat offset)
	{
		m_LocalOrientation = offset * m_LocalOrientation;
		UpdateModelMatrix();

		m_LocalRotation = glm::eulerAngles(m_LocalOrientation) * glm::degrees(1.0f);
	}

	void Transform::Rotate(const glm::vec3 axis, const float angle)
	{
		m_LocalOrientation = glm::angleAxis(glm::radians(angle), axis) * m_LocalOrientation;
		UpdateModelMatrix();

		m_LocalRotation = glm::eulerAngles(m_LocalOrientation) * glm::degrees(1.0f);
	}

	void Transform::Rotate(const float angle)
	{
		m_LocalOrientation = glm::angleAxis(glm::radians(angle), YAXIS) * m_LocalOrientation;
		UpdateModelMatrix();

		m_LocalRotation = glm::eulerAngles(m_LocalOrientation) * glm::degrees(1.0f);
	}

	void Transform::Rotate(const glm::vec3 euler)
	{
		m_LocalOrientation = glm::angleAxis(glm::radians(euler.z), ZAXIS) * m_LocalOrientation;
		m_LocalOrientation = glm::angleAxis(glm::radians(euler.x), XAXIS) * m_LocalOrientation;
		m_LocalOrientation = glm::angleAxis(glm::radians(euler.y), YAXIS) * m_LocalOrientation;

		m_LocalRotation = glm::eulerAngles(m_LocalOrientation) * glm::degrees(1.0f);
	}

	void Transform::SetOrientation(const glm::quat newOrientation)
	{
		m_LocalOrientation = newOrientation;
		UpdateModelMatrix();

		m_LocalRotation = glm::eulerAngles(m_LocalOrientation) * glm::degrees(1.0f);
	}

	void Transform::SetOrientation(const glm::vec3 axis, const float angle)
	{
		m_LocalOrientation = glm::angleAxis(glm::radians(angle), axis);
		UpdateModelMatrix();

		m_LocalRotation = glm::eulerAngles(m_LocalOrientation) * glm::degrees(1.0f);
	}

	void Transform::SetOrientation(const float angle)
	{
		m_LocalOrientation = glm::angleAxis(glm::radians(angle), YAXIS);
		UpdateModelMatrix();

		m_LocalRotation = glm::eulerAngles(m_LocalOrientation) * glm::degrees(1.0f);
	}

	void Transform::SetOrientation(const glm::vec3 euler)
	{
		m_LocalRotation = euler;

		m_LocalOrientation = glm::toQuat(glm::eulerAngleXYZ(glm::radians(m_LocalRotation.x), glm::radians(m_LocalRotation.y), glm::radians(m_LocalRotation.z)));

		UpdateModelMatrix();
	}

	void Transform::SetOrientation(const float x, const float y, const float z)
	{
		m_LocalRotation = { x,y,z };

		m_LocalOrientation = glm::toQuat(glm::eulerAngleXYZ(glm::radians(m_LocalRotation.x), glm::radians(m_LocalRotation.y), glm::radians(m_LocalRotation.z)));

		UpdateModelMatrix();
	}

	void Transform::Scale(const glm::vec3 amount)
	{
		m_LocalScale += amount;
		UpdateModelMatrix();
	}

	void Transform::ScaleXY(const glm::vec2 amount)
	{
		m_LocalScale += glm::vec3(amount.x, amount.y, 0.0f);
		UpdateModelMatrix();
	}

	void Transform::ScaleXZ(const glm::vec2 amount)
	{
		m_LocalScale += glm::vec3(amount.x, 0.0f, amount.y);
		UpdateModelMatrix();
	}

	void Transform::Scale(const float amount)
	{
		m_LocalScale += glm::vec3(amount);
		UpdateModelMatrix();
	}

	void Transform::SetScale(const glm::vec3 newScale)
	{
		m_LocalScale = newScale;
		UpdateModelMatrix();
	}

	void Transform::SetScaleXY(const glm::vec2 newScale)
	{
		m_LocalScale = glm::vec3(newScale.x, newScale.y, 1.0f);
		UpdateModelMatrix();
	}

	void Transform::SetScaleXZ(const glm::vec2 newScale)
	{
		m_LocalScale = glm::vec3(newScale.x, 1.0f, newScale.y);
		UpdateModelMatrix();
	}

	void Transform::SetScale(const float newScale)
	{
		m_LocalScale = glm::vec3(newScale);
		UpdateModelMatrix();
	}

	void Transform::UpdateModelMatrix()
	{
		// Updates the world position, orientation and scale
        m_Position    = m_Parent == nullptr ? m_LocalPosition : glm::vec3(m_Parent->m_ModelMatrix * glm::vec4(m_LocalPosition, 1.0f));
		m_Orientation = m_Parent == nullptr ? m_LocalOrientation : m_Parent->m_Orientation * m_LocalOrientation;
		m_Scale = m_Parent == nullptr ? m_LocalScale : m_Parent->m_Scale * m_LocalScale;
		
		// Parent
        m_ModelMatrix = m_Parent == nullptr ? glm::identity<glm::mat4>() : m_Parent->m_ModelMatrix;

		// Position
		m_ModelMatrix = glm::translate(m_ModelMatrix, m_LocalPosition);

		// Orientation
		/*auto o = m_LocalOrientation;
		o.y *= -1;
		o.z *= -1;*/
		m_ModelMatrix = m_ModelMatrix * glm::toMat4(m_LocalOrientation);

		// Scale
		m_ModelMatrix = glm::scale(m_ModelMatrix, m_LocalScale);

		// Directions
		m_Forward = glm::normalize(m_Orientation * ZAXIS);
		m_Right = glm::normalize(m_Orientation * -XAXIS);
		m_Up = glm::normalize(m_Orientation * YAXIS);

		// Update the children
		for (auto child : m_Children)
		{
			child->UpdateModelMatrix();
		}
	}

	void Transform::SetParent(Transform* newParent)
	{
		// If the transform has a parent
		if (m_Parent != nullptr)
		{
			// Remove the transform from the parents list of children
			const auto index = m_Parent->GetChildIndex(this);
			if (index != -1)
				m_Parent->m_Children.erase(m_Parent->m_Children.begin() + index);
		}

		// Set new parent
		m_Parent = newParent;

		// Add the transform to the new parents list of children
		if (m_Parent != nullptr)
			m_Parent->m_Children.push_back(this);

		// Update the model matrix
		UpdateModelMatrix();
	}

	int Transform::GetChildIndex(Transform *child) const
	{
		for (unsigned int i = 0; i < m_Children.size(); i++)
		{
			if (child == m_Children[i])
				return i;
		}
		return -1;
	}

	glm::vec3 Transform::LocalToWorld(const glm::vec3 localPosition) const
	{
		return m_Orientation * localPosition;
	}

	std::vector<Transform*>& Transform::GetChildren()
	{
		return m_Children;
	}
}