#pragma once

#include <glm/glm.hpp>

namespace EVA
{
	class Shader
	{
		uint32_t m_RendererId;

	public:
		explicit Shader(const std::string& vertexSource, const std::string& fragmentSource);
		~Shader();

		void Bind() const;
		void Unbind() const;

		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
	};
}