#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "EVA.hpp"
#include "EVA/Utility/SlidingWindow.hpp"
#include "Platform\OpenGL\OpenGLShader.hpp"

class ExampleLayer : public EVA::Layer
{
	EVA::Ref<EVA::VertexArray> m_TriangleVertexArray;
	EVA::Ref<EVA::VertexArray> m_SquareVertexArray;

	EVA::Ref<EVA::Shader> m_FlatColorShader, m_TextureShader;

	EVA::Ref<EVA::Texture2D> m_Texture;

	EVA::OrthographicCameraController m_CameraController;

	EVA::SlidingWindow<float> m_FrameTimes;

	glm::vec3 m_SquareColor = glm::vec3(0.2f, 0.3f, 0.8f);

public:
	ExampleLayer() : Layer("Example"), m_FrameTimes(10),
		m_CameraController(EVA::Application::Get().GetWindow().GetWidth() / EVA::Application::Get().GetWindow().GetHeight())
	{
		{
			// Triangle
			m_TriangleVertexArray = EVA::VertexArray::Create();

			// Vertex buffer
			float vertices[3 * 7] = {
				-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
				 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
				 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f,
			};
			EVA::Ref<EVA::VertexBuffer> vb;
			vb = EVA::VertexBuffer::Create(vertices, sizeof(vertices));
			EVA::BufferLayout layout = {
				{ EVA::ShaderDataType::Float3, "a_Position" },
				{ EVA::ShaderDataType::Float4, "a_Color" }
			};
			vb->SetLayout(layout);
			m_TriangleVertexArray->AddVertexBuffer(vb);

			// Index buffer
			uint32_t indices[3] = {
				0, 1, 2
			};
			EVA::Ref<EVA::IndexBuffer> ib;
			ib = EVA::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
			m_TriangleVertexArray->SetIndexBuffer(ib);
		}
		{
			// Square
			m_SquareVertexArray = EVA::VertexArray::Create();

			float vertices[5 * 4] = {
				-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
				 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
				 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
				-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
			};
			EVA::Ref<EVA::VertexBuffer> vb;
			vb = EVA::VertexBuffer::Create(vertices, sizeof(vertices));
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
			ib = EVA::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
			m_SquareVertexArray->SetIndexBuffer(ib);
		}

		// Shaders
		m_FlatColorShader = EVA::Shader::Create("./assets/shaders/color.glsl");
		m_TextureShader = EVA::Shader::Create("./assets/shaders/texture.glsl");

		// Texture
		m_Texture = EVA::Texture2D::Create("assets/textures/uv.png");
		m_Texture->Bind(0);

		m_TextureShader->Bind();
		std::dynamic_pointer_cast<EVA::OpenGLShader>(m_TextureShader)->SetUniformInt("u_Texture", 0);
	}

	void OnUpdate() override
	{
		auto dt = EVA::Platform::GetDeltaTime();
		m_FrameTimes.Add(dt);

		m_CameraController.OnUpdate();

		// Render
		EVA::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		EVA::RenderCommand::Clear();

		EVA::Renderer::BeginScene(m_CameraController.GetCamera());

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

		EVA::Renderer::EndScene();
	}

	void OnEvent(EVA::Event& e) override
	{
		m_CameraController.OnEvent(e);
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
	app.GetWindow().SetVSync(false);
	app.Run();

	return 0;
}
