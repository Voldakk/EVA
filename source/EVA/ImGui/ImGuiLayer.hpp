#pragma once

#include "EVA/Core/Layer.hpp"

namespace EVA
{
	class ImGuiLayer : public Layer
	{
		float m_Time = 0.0f;
		bool m_BlockEvents;

	public:
		ImGuiLayer();
		virtual ~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		void Begin();
		void End();
		virtual void OnEvent(Event& event) override;
		virtual void OnImGuiRender() override;

		void BlockEvents(bool block) { m_BlockEvents = block; };
	};
}
