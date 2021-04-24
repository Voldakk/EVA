#include "Mesh.hpp"

#include "FileSystem.hpp"
#include "TextureManager.hpp"

#include "obj/OBJ_Loader.h"

namespace EVA
{
    void Material::Bind(const Ref<Shader> shader)
    {
        if (albedo) shader->BindTexture("u_AlbedoMap", albedo);
        if (emissive) shader->BindTexture("u_EmissiveMap", emissive);
        if (normal) shader->BindTexture("u_NormalMap", normal);
        if (metallic) shader->BindTexture("u_MetallicMap", metallic);
        if (roughness) shader->BindTexture("u_RoughnessMap", roughness);
    }

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Ref<Material> material) : m_Material(material)
    {
        EVA_PROFILE_FUNCTION();

        m_VertexArray = EVA::VertexArray::Create();

        // Vertex buffer
        auto vb = EVA::VertexBuffer::Create(vertices.data(), sizeof(Vertex) * vertices.size());

        EVA::BufferLayout layout = {
          {EVA::ShaderDataType::Float3, "a_Position"},  {EVA::ShaderDataType::Float2, "a_TexCoords"},
          {EVA::ShaderDataType::Float3, "a_Normal"},    {EVA::ShaderDataType::Float3, "a_Tangent"},
          {EVA::ShaderDataType::Float3, "a_Bitangent"},
        };
        vb->SetLayout(layout);
        m_VertexArray->AddVertexBuffer(vb);

        // Index buffer
        auto ib = EVA::IndexBuffer::Create(indices.data(), indices.size());
        m_VertexArray->SetIndexBuffer(ib);
    }

    std::vector<Ref<Mesh>> Mesh::LoadMesh(const std::filesystem::path& path)
    {
        EVA_PROFILE_FUNCTION();

        EVA_INTERNAL_TRACE("Loading mesh: {}", FileSystem::ToString(path));

        objl::Loader loader;
        bool loaded = loader.LoadFile(FileSystem::ToString(path));
        EVA_INTERNAL_ASSERT(loaded, "Failed to load file");

        std::vector<Ref<Mesh>> meshes;

        for (const auto& mesh : loader.LoadedMeshes)
        {
            std::vector<Vertex> vertices;
            vertices.reserve(mesh.Vertices.size());
            {
                // Copy vertices
                for (const auto& v : mesh.Vertices)
                {
                    Vertex vertex;
                    vertex.position  = glm::vec3(v.Position.X, v.Position.Y, v.Position.Z);
                    vertex.texCoords = glm::vec2(v.TextureCoordinate.X, v.TextureCoordinate.Y);
                    vertex.normal    = glm::vec3(v.Normal.X, v.Normal.Y, v.Normal.Z);
                    vertex.tangent   = glm::vec3(0);
                    vertex.bitangent = glm::vec3(0);
                    vertices.push_back(vertex);
                }

                // Calculate tangents
                for (long i = 0; i < mesh.Indices.size(); i += 3)
                {
                    long i1 = mesh.Indices[i];
                    long i2 = mesh.Indices[i + 1];
                    long i3 = mesh.Indices[i + 2];

                    const glm::vec3& v1 = vertices[i1].position;
                    const glm::vec3& v2 = vertices[i2].position;
                    const glm::vec3& v3 = vertices[i3].position;

                    const glm::vec2& w1 = vertices[i1].position;
                    const glm::vec2& w2 = vertices[i2].texCoords;
                    const glm::vec2& w3 = vertices[i3].texCoords;

                    float x1 = v2.x - v1.x;
                    float x2 = v3.x - v1.x;
                    float y1 = v2.y - v1.y;
                    float y2 = v3.y - v1.y;
                    float z1 = v2.z - v1.z;
                    float z2 = v3.z - v1.z;

                    float s1 = w2.x - w1.x;
                    float s2 = w3.x - w1.x;
                    float t1 = w2.y - w1.y;
                    float t2 = w3.y - w1.y;

                    float r = 1.0F / (s1 * t2 - s2 * t1);
                    glm::vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
                    glm::vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

                    vertices[i1].tangent += sdir;
                    vertices[i2].tangent += sdir;
                    vertices[i3].tangent += sdir;

                    vertices[i1].bitangent += tdir;
                    vertices[i2].bitangent += tdir;
                    vertices[i3].bitangent += tdir;
                }

                for (auto& v : vertices)
                {
                    v.tangent   = glm::normalize(v.tangent);
                    v.bitangent = glm::normalize(v.bitangent);
                }
            }

            Ref<Material> material = CreateRef<Material>();
            {
                if (mesh.MeshMaterial.map_Kd != "")
                    material->albedo = TextureManager::LoadTexture(path.parent_path() / mesh.MeshMaterial.map_Kd);

                if (mesh.MeshMaterial.map_Kd != "")
                    material->emissive = TextureManager::LoadTexture(path.parent_path() / mesh.MeshMaterial.map_Ke);

                if (mesh.MeshMaterial.map_Kd != "")
                    material->normal = TextureManager::LoadTexture(path.parent_path() / mesh.MeshMaterial.norm);

                if (mesh.MeshMaterial.map_Kd != "")
                    material->metallic = TextureManager::LoadTexture(path.parent_path() / mesh.MeshMaterial.map_Pm);

                if (mesh.MeshMaterial.map_Kd != "")
                    material->roughness = TextureManager::LoadTexture(path.parent_path() / mesh.MeshMaterial.map_Pr);
            }

            meshes.push_back(CreateRef<Mesh>(vertices, mesh.Indices, material));
        }

        return meshes;
    }
} // namespace EVA