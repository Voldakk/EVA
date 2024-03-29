#pragma once

#include "EVA/Events/Event.hpp"

namespace EVA
{
    class Layer
    {
      protected:
        std::string m_DebugName;

      public:
        explicit Layer(const std::string& name = "Layer");
        virtual ~Layer();

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate() {}
        virtual void OnImGuiRender() {}
        virtual void OnEvent(Event& event) {}

        inline const std::string& GetName() const { return m_DebugName; }
    };
} // namespace EVA
