#pragma once

#include "Renderer.hpp"
#include "EVA/Assets/TextureUtilities.hpp"

namespace EVA
{
    class Environment : public ISerializeable
    {
      public:
        Environment(const std::filesystem::path& map)
        {
            EVA_PROFILE_FUNCTION();

            m_Mesh   = AssetManager::Load<Mesh>("models/cube_inverted.obj");
            m_Shader = AssetManager::Load<Shader>("shaders/skybox.glsl");
            m_BrdfLUT            = TextureUtilities::PreComputeBRDF();
            m_EquirectangularMap = AssetManager::Load<Texture>(map);
            Compute();
        }

        void Compute() 
        {
            TextureManager::GenerateMipMaps(m_EquirectangularMap);
            m_EnvironmentMap = TextureUtilities::EquirectangularToCubemap(m_EquirectangularMap);
            m_IrradianceMap  = TextureUtilities::ConvoluteCubemap(m_EnvironmentMap);
            m_PreFilterMap   = TextureUtilities::PreFilterEnviromentMap(m_EnvironmentMap);
        }

        void DrawSkyBox() const
        {
            EVA_PROFILE_FUNCTION();

            m_Shader->Bind();
            m_Shader->ResetTextureUnit();
            RenderCommand::EnableDepth(false);
            Renderer::Submit(m_Shader, m_Mesh->GetVertexArray());
            RenderCommand::EnableDepth(true);
        }

        void Serialize(DataObject& data) override
        {
            ISerializeable::Serialize(data);
            bool hdrChanged = false;
            if (data.Inspector()) 
            {
                ImGui::PushID(this);
                ImGui::Text("Enviroment");
                hdrChanged |= ImGui::Button("Reload");
                hdrChanged |= data.Serialize("HDR", m_EquirectangularMap);
                ImGui::SliderAngle("Rotation", &rotation);
            }
            else
            {
                data.Serialize("HDR", m_EquirectangularMap);
                data.Serialize("rotation", rotation);
            }

            data.Serialize("Mesh", m_Mesh);
            data.Serialize("Shader", m_Shader);

            if (data.Inspector()) { ImGui::PopID(); }

            hdrChanged |= data.Load();

            if (hdrChanged) { Compute(); }
        }

        const Ref<Texture> GetEquirectangularMap() const { return m_EquirectangularMap; }
        const Ref<Texture> GetEnvironmentMap() const { return m_EnvironmentMap; }
        const Ref<Texture> GetIrradianceMap() const { return m_IrradianceMap; }
        const Ref<Texture> GetPreFilterMap() const { return m_PreFilterMap; }
        const Ref<Texture> GetBrdfLUT() const { return m_BrdfLUT; }

        Ref<Mesh> GetMesh() { return m_Mesh; }
        const Ref<Mesh> GetMesh() const { return m_Mesh; }

        Ref<Shader> GetShader() { return m_Shader; }
        const Ref<Shader> GetShader() const { return m_Shader; }

        float rotation = 0.0f;

      private:
        Ref<Texture> m_EquirectangularMap;
        Ref<Texture> m_EnvironmentMap;
        Ref<Texture> m_IrradianceMap;
        Ref<Texture> m_PreFilterMap;
        Ref<Texture> m_BrdfLUT;

        Ref<Mesh> m_Mesh;
        Ref<Shader> m_Shader;
    };
} // namespace EVA