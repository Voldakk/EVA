#pragma once

#include "EVA/Layer.hpp"

namespace EVA
{
	class ImGuiLayer : public Layer
	{
		float m_Time = 0.0f;

	public:
		ImGuiLayer();
		virtual ~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		void Begin();
		void End();
		virtual void OnEvent(Event& event) override;
		virtual void OnImGuiRender() override;
	};
}
