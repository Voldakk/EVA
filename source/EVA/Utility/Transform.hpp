#pragma once

namespace EVA
{
	const glm::vec3 XAXIS = { 1.0f, 0.0f, 0.0f };
	const glm::vec3 YAXIS = { 0.0f, 1.0f, 0.0f };
	const glm::vec3 ZAXIS = { 0.0f, 0.0f, 1.0f };

	class GameObject;

	class Transform
	{
		glm::mat4 m_ModelMatrix;

		Transform* m_Parent = nullptr;
		std::vector<Transform *> m_Children;

		glm::vec3 m_LocalPosition = glm::vec3(0.0f);
		glm::quat m_LocalOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 m_LocalScale = glm::vec3(1.0f);

		glm::vec3 m_LocalRotation = glm::vec3(0.0f);

		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::quat m_Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 m_Scale = glm::vec3(1.0f);

		glm::vec3 m_Forward;
		glm::vec3 m_Right;
		glm::vec3 m_Up;

	public:

		inline const glm::mat4& GetModelMatrix() const { return m_ModelMatrix; }
		inline const glm::vec3& GetLocalPosition() const { return m_LocalPosition; }
		inline const glm::quat& GetLocalOrientation() const { return m_LocalOrientation; }
		inline const glm::vec3& GetLocalScale() const { return m_LocalScale; }
		inline const glm::vec3& GetLocalRotation() const { return m_LocalRotation; }
		inline const glm::vec3& GetPosition() const { return m_Position; }
		inline const glm::quat& GetOrientation() const { return m_Orientation; }
		inline const glm::vec3& GetScale() const { return m_Scale; }
		inline const glm::vec3& GetForward() const { return m_Forward; }
		inline const glm::vec3& GetRight() const { return m_Right; }
		inline const glm::vec3& GetUp() const { return m_Up; }
		inline const Transform* GetParent() const { return m_Parent; }

		/**
		 * \brief Constructor
		 */
		Transform();

		/**
		* \brief Translates the transform by the given offset
		* \param offset The amount the transform should be translated
		*/
		void Translate(glm::vec3 offset);

		/**
		* \brief Translates the transform by the given offset
		* \param offset The amount the transform should be translated
		*/
		void TranslateXY(glm::vec2 offset);

		/**
		* \brief Translates the transform by the given offset
		* \param offset The amount the transform should be translated
		*/
		void TranslateXZ(glm::vec2 offset);

		/**
		* \brief Sets the transfrom's position
		* \param newPosition The new position
		*/
		void SetPosition(glm::vec3 newPosition);


		/**
		* \brief Sets the transfrom's position
		* \param newPosition The new position
		*/
		void SetPositionXY(glm::vec2 newPosition);

		/**
		* \brief Sets the transfrom's position
		* \param newPosition The new position
		*/
		void SetPositionXZ(glm::vec2 newPosition);

		/**
		 * \brief Rotates the transform by the given offset
		 * \param offset 
		 */
		void Rotate(glm::quat offset);

		/**
		* \brief Rotates the transform around an axis
		* \param axis The axis to rotate around
		* \param angle The angle in degrees
		*/
		void Rotate(glm::vec3 axis, float angle);

		/**
		* \brief Rotates the transform around the Y axis
		* \param angle The angle in degrees
		*/
		void Rotate(float angle);

		/**
		* \brief Rotates z degrees around the z axis, x degrees around the x axis, and y degrees around the y axis
		* \param euler The angles in degrees
		*/
		void Rotate(glm::vec3 euler);

		/**
		 * \brief Sets the transform's orientation
		 * \param newOrientation The new orientation
		 */
		void SetOrientation(glm::quat newOrientation);

		/**
		* \brief Sets the transform's orientation defined by a rotation around an axis
		* \param axis The axis to rotate around
		* \param angle The angle in degrees
		*/
		void SetOrientation(glm::vec3 axis, float angle);

		/**
		* \brief Sets the transform's orientation defined by a rotation around the Z axis
		* \param angle The angle in degrees
		*/
		void SetOrientation(float angle);

		/**
		* \brief Rotates z degrees around the z axis, x degrees around the x axis, and y degrees around the y axis
		* \param euler The angles in degrees
		*/
		void SetOrientation(glm::vec3 euler);

		/**
		* \brief Rotates z degrees around the z axis, x degrees around the x axis, and y degrees around the y axis
		* \param x The rotation around the x axis in degrees
		* \param y The rotation around the y axis in degrees
		* \param z The rotation around the z axis in degrees
		*/
		void SetOrientation(float x, float y, float z);

		/**
		 * \brief Scales the transform by a given amount on each axis
		 * \param amount The amount
		 */
		void Scale(glm::vec3 amount);

		/**
		 * \brief Scales the transform by a given amount on each axis
		 * \param amount The amount
		 */
		void ScaleXY(glm::vec2 amount);

		/**
		 * \brief Scales the transform by a given amount on each axis
		 * \param amount The amount
		 */
		void ScaleXZ(glm::vec2 amount);

		/**
		 * \brief Scales the transform by a given amount
		 * \param amount The amount
		 */
		void Scale(float amount);

		/**
		 * \brief Sets the transform's scale
		 * \param newScale The new scale
		 */
		void SetScale(glm::vec3 newScale);

		/**
		 * \brief Sets the transform's scale. (X, Y) => (X, Y, 1)
		 * \param newScale The new scale
		 */
		void SetScaleXY(glm::vec2 newScale);

		/**
		 * \brief Sets the transform's scale. (X, Y) => (X, 1, Y)
		 * \param newScale The new scale
		 */
		void SetScaleXZ(glm::vec2 newScale);

		/**
		 * \brief Sets the transform's scale
		 * \param newScale The new scale
		 */
		void SetScale(float newScale);

		/**
		 * \brief Recalculates the transform's model matrix (and all children recursively)
		 */
		void UpdateModelMatrix();

		/**
		 * \brief Sets the transform's parent
		 * \param newParent The new parent
		 */
		void SetParent(Transform* newParent);

		/**
		 * \brief Gets the child's index in the transforms list of children
		 * \param child The child
		 * \return The child's index
		 */
		int GetChildIndex(Transform *child) const;

		glm::vec3 LocalToWorld(glm::vec3 localPosition) const;

		std::vector<Transform *>& GetChildren();
	};
}