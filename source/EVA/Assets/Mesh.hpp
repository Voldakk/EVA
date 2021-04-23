#pragma once

#include "EVA/Renderer/VertexArray.hpp"
#include "EVA/Renderer/Texture.hpp"
#include "EVA//Renderer/Shader.hpp"

namespace EVA
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texCoords;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };

    struct Material
    {
        Ref<Texture> albedo;
        Ref<Texture> emissive;
        Ref<Texture> normal;
        Ref<Texture> metallic;
        Ref<Texture> roughness;

        void Bind(const Ref<Shader> shader);
    };

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
