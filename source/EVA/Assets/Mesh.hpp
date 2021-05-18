#pragma once

#include "EVA/Renderer/VertexArray.hpp"
#include "EVA//Renderer/Vertex.hpp"
#include "EVA//Renderer/Material.hpp"
#include "EVA/Renderer/Texture.hpp"
#include "EVA//Renderer/Shader.hpp"

namespace EVA
{
    class Mesh
    {
      public:
        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Ref<Material> material);

        static std::vector<Ref<Mesh>> LoadMesh(const std::filesystem::path& path);

        const Ref<VertexArray> GetVertexArray() { return m_VertexArray; }
        const Ref<Material> GetMaterial() { return m_Material; }

      private:
        Ref<VertexArray> m_VertexArray;
        Ref<Material> m_Material;
    };
} // namespace EVA
