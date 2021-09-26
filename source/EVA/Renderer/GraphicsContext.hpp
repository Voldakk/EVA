#pragma once

namespace EVA
{
    class GraphicsContext
    {
      protected:
        inline static GraphicsContext* s_Instance = nullptr;
      public:

        static GraphicsContext& GetInstance() { return *s_Instance; }

        virtual ~GraphicsContext() = default;
        virtual void Init()        = 0;
        virtual void SwapBuffers() = 0;

        virtual uint32_t MaxUnifromBufferBindings() = 0;
        inline uint32_t TempUnifromBufferBinding() { return MaxUnifromBufferBindings() - 1; }
    };
} // namespace EVA