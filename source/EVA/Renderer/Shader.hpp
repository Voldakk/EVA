#pragma once

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
	};
}