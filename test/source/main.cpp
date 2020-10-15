#include "EVA.hpp"
#include "EVA/Utility/SlidingWindow.hpp"

class ExampleLayer : public EVA::Layer
{
	std::shared_ptr<EVA::VertexArray> m_VertexArray;
	std::shared_ptr<EVA::VertexArray> m_SquareVertexArray;

	std::shared_ptr<EVA::Shader> m_Shader;
	std::shared_ptr<EVA::Shader> m_BlueShader;

	EVA::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition = glm::vec3(0.0f);
	float m_CameraRotation = 0.0f;

	float m_CameraSpeed = 0.1f;
	float m_CameraRotationSpeed = 5.0f;

	EVA::SlidingWindow<float> m_FrameTimes;


public:
	ExampleLayer() : Layer("Example"), m_Camera(-1.0f, 1.0f, -1.0f, 1.0f), m_FrameTimes(10)
	{
		{
			//// Vertex array
			m_VertexArray.reset(EVA::VertexArray::Create());

			// Vertex buffer
			float vertices[3 * 7] = {
				-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
				 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
				 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f,
			};
			std::shared_ptr<EVA::VertexBuffer> vb;
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
			std::shared_ptr<EVA::IndexBuffer> ib;
			ib.reset(EVA::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
			m_VertexArray->SetIndexBuffer(ib);
		}
		{
			// Square
			m_SquareVertexArray.reset(EVA::VertexArray::Create());

			float vertices[3 * 4] = {
				-0.75f, -0.75f, 0.0f,
				 0.75f, -0.75f, 0.0f,
				 0.75f,  0.75f, 0.0f,
				-0.75f,  0.75f, 0.0f
			};
			std::shared_ptr<EVA::VertexBuffer> vb;
			vb.reset(EVA::VertexBuffer::Create(vertices, sizeof(vertices)));
			vb->SetLayout({
				{ EVA::ShaderDataType::Float3, "a_Position" }
				});
			m_SquareVertexArray->AddVertexBuffer(vb);

			uint32_t indices[6] = {
				0, 1, 2,
				2, 3, 0,
			};
			std::shared_ptr<EVA::IndexBuffer> ib;
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

void main()
{
	v_Color = a_Color;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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
		m_Shader = std::make_unique<EVA::Shader>(vertexSource, fragmentSource);

		// Shader
		std::string vertexSource2 =
			R"(
#version 330 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
)";

		std::string fragmentSource2 =
			R"(
#version 330 core

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(0.2, 0.3, 0.8, 1.0);
}
)";
		m_BlueShader = std::make_unique<EVA::Shader>(vertexSource2, fragmentSource2);
	}

	void OnUpdate() override
	{
		auto dt = EVA::Platform::GetDeltaTime();
		m_FrameTimes.Add(dt);

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

		EVA::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		EVA::RenderCommand::Clear();

		EVA::Renderer::BeginScene(m_Camera);

		EVA::Renderer::Submit(m_BlueShader, m_SquareVertexArray);
		EVA::Renderer::Submit(m_Shader, m_VertexArray);

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
