#pragma once

namespace EVA
{
    class Camera
    {
      public:
        virtual const glm::mat4& GetProjectionMatrix() const     = 0;
        virtual const glm::mat4& GetViewMatrix() const           = 0;
        virtual const glm::mat4& GetViewProjectionMatrix() const = 0;
    };
} // namespace EVA
