#pragma once

#include "Layer.hpp"

namespace EVA
{
	class LayerStack
	{
		std::vector<Layer*> m_Layers;
		int m_LayerInsertIndex = 0;
	public:
		LayerStack();
		virtual ~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
		std::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
		std::vector<Layer*>::const_iterator end() const { return m_Layers.end(); }
	};
}
