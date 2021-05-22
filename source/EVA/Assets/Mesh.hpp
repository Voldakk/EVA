#pragma once

#include "EVA/Renderer/VertexArray.hpp"
#include "EVA//Renderer/Vertex.hpp"
#include "EVA//Renderer/Material.hpp"
#include "EVA/Renderer/Texture.hpp"
#include "EVA//Renderer/Shader.hpp"

namespace EVA
{
    class SubMesh
    {
      public:
        SubMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Ref<Material> material);

        Ref<VertexArray> GetVertexArray() { return m_VertexArray; }
        const Ref<VertexArray> GetVertexArray() const { return m_VertexArray; }

        Ref<Material> GetMaterial() { return m_Material; }
        const Ref<Material> GetMaterial() const { return m_Material; }

        const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
        const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

      private:
        Ref<VertexArray> m_VertexArray;
        Ref<Material> m_Material;

        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
    };

    class Mesh : public Asset
    {
      public:
        Mesh() {}
        Mesh(const std::vector<Ref<SubMesh>>& subMeshes);
        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Ref<Material> material);

        static Ref<Mesh> LoadMesh(const std::filesystem::path& path);

        size_t Count() const { return m_SubMeshes.size(); }

        Ref<SubMesh>& GetSubMesh(size_t index) { return m_SubMeshes[index]; }
        const Ref<SubMesh>& GetSubMesh(size_t index) const { return m_SubMeshes[index]; }

        const Ref<VertexArray> GetVertexArray() const { return m_SubMeshes.empty() ? nullptr : m_SubMeshes[0]->GetVertexArray(); }
        const Ref<Material>& GetMaterial() const { return m_SubMeshes.empty() ? nullptr : m_SubMeshes[0]->GetMaterial(); }

      private:
        std::vector<Ref<SubMesh>> m_SubMeshes;
    };
} // namespace EVA
