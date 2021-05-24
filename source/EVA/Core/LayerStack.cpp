#include "LayerStack.hpp"

namespace EVA
{
    LayerStack::LayerStack() {}

    LayerStack::~LayerStack()
    {
        EVA_PROFILE_FUNCTION();
        for (auto layer : m_Layers)
        {
            delete layer;
        }
    }

    void LayerStack::PushLayer(Layer* layer)
    {
        m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
        m_LayerInsertIndex--;
    }

    void LayerStack::PushOverlay(Layer* overlay) { m_Layers.emplace_back(overlay); }

    void LayerStack::PopLayer(Layer* layer)
    {
        auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
        if (it != m_Layers.end())
        {
            m_Layers.erase(it);
            m_LayerInsertIndex--;
        }
    }

    void LayerStack::PopOverlay(Layer* overlay)
    {
        auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
        if (it != m_Layers.end()) { m_Layers.erase(it); }
    }
} // namespace EVA
