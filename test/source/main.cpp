#include <glm/gtc/matrix_transform.hpp>

#include <EVA.hpp>
#include <EVA/Utility/SlidingWindow.hpp>
#include <Platform\OpenGL\OpenGLShader.hpp>
#include <glm\gtc\type_ptr.hpp>

class ExampleLayer : public EVA::Layer
{
	EVA::Ref<EVA::VertexArray> m_VertexArray;
	EVA::Ref<EVA::VertexArray> m_SquareVertexArray;

	EVA::Ref<EVA::Shader> m_Shader;
	EVA::Ref<EVA::Shader> m_FlatColorShader, m_TextureShader;

	EVA::Ref<EVA::Texture2D> m_Texture;

	EVA::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition = glm::vec3(0.0f);
	float m_CameraRotation = 0.0f;

	float m_CameraSpeed = 2.0f;
	float m_CameraRotationSpeed = 90.0f;

	EVA::SlidingWindow<float> m_FrameTimes;

	glm::vec3 m_SquareColor = glm::vec3(0.2f, 0.3f, 0.8f);

public:
	ExampleLayer() : Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_FrameTimes(10)
	{
		{
			// Triangle
			m_VertexArray.reset(EVA::VertexArray::Create());

			// Vertex buffer
			float vertices[3 * 7] = {
				-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
				 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
				 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f,
			};
			EVA::Ref<EVA::VertexBuffer> vb;
			vb.reset(EVA::VertexBuffer::Create(vertices, sizeof(vertices)));
			EVA::BufferLayout layout = {
				{ EVA::ShaderDataType::Float3, "a_Position" },
				{ EVA::ShaderDataType::Float4, "a_Color" }
			};
			vb->SetLayout(layout);
			m_VertexArray->AddVertexBuffer(vb);

			// Index buffer
			uint32_t indices[3] = {
				0, 1, 2
			};
			EVA::Ref<EVA::IndexBuffer> ib;
			ib.reset(EVA::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
			m_VertexArray->SetIndexBuffer(ib);
		}
		{
			// Square
			m_SquareVertexArray.reset(EVA::VertexArray::Create());

			float vertices[5 * 4] = {
				-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
				 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
				 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
				-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
			};
			EVA::Ref<EVA::VertexBuffer> vb;
			vb.reset(EVA::VertexBuffer::Create(vertices, sizeof(vertices)));
			vb->SetLayout({
				{ EVA::ShaderDataType::Float3, "a_Position" },
				{ EVA::ShaderDataType::Float2, "a_TexCoord" }
				});
			m_SquareVertexArray->AddVertexBuffer(vb);

			uint32_t indices[6] = {
				0, 1, 2,
				2, 3, 0,
			};
			EVA::Ref<EVA::IndexBuffer> ib;
			ib.reset(EVA::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
			m_SquareVertexArray->SetIndexBuffer(ib);
		}
		// Shader
		std::string vertexSource =
			R"(
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

out vec4 v_Color;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	v_Color = a_Color;
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}
)";

		std::string fragmentSource =
			R"(
#version 330 core

layout(location = 0) out vec4 color;

in vec4 v_Color;


void main()
{
	color = v_Color;
}
)";
		m_Shader.reset(EVA::Shader::Create(vertexSource, fragmentSource));

		// Shader
		std::string vertexSourceFlatColor =
			R"(
#version 330 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}
)";

		std::string fragmentSourceFlatColor =
			R"(
#version 330 core

layout(location = 0) out vec4 color;

uniform vec3 u_Color;

void main()
{
	color = vec4(u_Color, 1.0);
}
)";
		m_FlatColorShader.reset(EVA::Shader::Create(vertexSourceFlatColor, fragmentSourceFlatColor));

		// Shader
		std::string vertexSourceTexture =
			R"(
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}
)";

		std::string fragmentSourceTexture =
			R"(
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
	color = texture(u_Texture, v_TexCoord);
}
)";
		m_TextureShader.reset(EVA::Shader::Create(vertexSourceTexture, fragmentSourceTexture));


		// Texture
		m_Texture = EVA::Texture2D::Create("assets/uv.png");
		m_Texture->Bind(0);

		m_TextureShader->Bind();
		std::dynamic_pointer_cast<EVA::OpenGLShader>(m_TextureShader)->SetUniformInt("u_Texture", 0);
	}

	void OnUpdate() override
	{
		auto dt = EVA::Platform::GetDeltaTime();
		m_FrameTimes.Add(dt);

		// Camera
		if (EVA::Input::IsKeyPressed(EVA::KeyCode::W))
			m_CameraPosition.y += m_CameraSpeed * dt;
		if (EVA::Input::IsKeyPressed(EVA::KeyCode::S))
			m_CameraPosition.y -= m_CameraSpeed * dt;

		if (EVA::Input::IsKeyPressed(EVA::KeyCode::A))
			m_CameraPosition.x -= m_CameraSpeed * dt;
		if (EVA::Input::IsKeyPressed(EVA::KeyCode::D))
			m_CameraPosition.x += m_CameraSpeed * dt;

		if (EVA::Input::IsKeyPressed(EVA::KeyCode::Q))
			m_CameraRotation += m_CameraRotationSpeed * dt;
		if (EVA::Input::IsKeyPressed(EVA::KeyCode::E))
			m_CameraRotation -= m_CameraRotationSpeed * dt;

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);
		

		// Render
		EVA::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		EVA::RenderCommand::Clear();

		EVA::Renderer::BeginScene(m_Camera);

		// Squares
		m_FlatColorShader->Bind();
		std::dynamic_pointer_cast<EVA::OpenGLShader>(m_FlatColorShader)->SetUniformFloat3("u_Color", m_SquareColor);

		auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		for (size_t y = 0; y < 20; y++)
		{
			for (size_t x = 0; x < 20; x++)
			{
				auto transform = glm::translate(glm::mat4(1.0f), glm::vec3(x * 0.11f, y * 0.11f, 0.0f)) * scale;
				EVA::Renderer::Submit(m_FlatColorShader, m_SquareVertexArray, transform);
			}
		}

		auto scale2 = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
		EVA::Renderer::Submit(m_TextureShader, m_SquareVertexArray, scale2);

		// Triangle
		//EVA::Renderer::Submit(m_Shader, m_VertexArray);

		EVA::Renderer::EndScene();
	}

	void OnEvent(EVA::Event& e) override
	{
		//EVA_INFO("Example layer: {0}", e);
	}

	void OnImGuiRender() override
	{
		auto avgFrameTime = m_FrameTimes.GetAverage();
		ImGui::Begin("Metrics");
		ImGui::Text("FPS: %.2f", 1.0f / avgFrameTime);
		ImGui::Text("Frame time: %.2f ms", avgFrameTime * 1000);
		ImGui::End();

		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}
};

int main()
{
	EVA::Application app;
	app.PushLayer(new ExampleLayer());
	app.GetWindow().SetVSync(true);
	app.Run();

	return 0;
}
