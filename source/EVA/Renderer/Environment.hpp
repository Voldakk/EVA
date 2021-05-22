#pragma once

#include "Renderer.hpp"
#include "EVA/Assets/TextureUtilities.hpp"

namespace EVA
{
    class Environment
    {
      public:
        Environment(const std::filesystem::path& equirectangularMap)
        {
            EVA_PROFILE_FUNCTION();

            m_EquirectangularMap = AssetManager::Load<Texture>(equirectangularMap);
            TextureManager::GenerateMipMaps(m_EquirectangularMap);
            m_EnvironmentMap     = TextureUtilities::EquirectangularToCubemap(m_EquirectangularMap);
            m_IrradianceMap      = TextureUtilities::ConvoluteCubemap(m_EnvironmentMap);
            m_PreFilterMap       = TextureUtilities::PreFilterEnviromentMap(m_EnvironmentMap);
            m_BrdfLUT            = TextureUtilities::PreComputeBRDF();

            m_Mesh   = AssetManager::Load<Mesh>("models/cube_inverted.obj");
            m_Shader = AssetManager::Load<Shader>("shaders/skybox.glsl");
        }

        void DrawSkyBox() const
        {
            EVA_PROFILE_FUNCTION();

            m_Shader->ResetTextureUnit();
            RenderCommand::EnableDepth(false);
            Renderer::Submit(m_Shader, m_Mesh->GetVertexArray());
            RenderCommand::EnableDepth(true);
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