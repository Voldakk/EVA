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

            m_EquirectangularMap = TextureManager::LoadTexture(equirectangularMap);
            m_EnvironmentMap     = TextureUtilities::EquirectangularToCubemap(m_EquirectangularMap);
            m_IrradianceMap      = TextureUtilities::ConvoluteCubemap(m_EnvironmentMap);
            m_PreFilterMap       = TextureUtilities::PreFilterEnviromentMap(m_EnvironmentMap);
            m_BrdfLUT            = TextureUtilities::PreComputeBRDF();

            m_SkyboxMesh = Mesh::LoadMesh("./assets/models/cube_inverted.obj")[0];
            m_SkyboxShader = Shader::Create("./assets/shaders/skybox.glsl");
        }

        void DrawSkyBox()  const
        {
            EVA_PROFILE_FUNCTION();

            m_SkyboxShader->Bind();
            m_SkyboxShader->ResetTextureUnit();
            RenderCommand::EnableDepth(false);
            Renderer::Submit(m_SkyboxShader, m_SkyboxMesh->GetVertexArray());
            RenderCommand::EnableDepth(true);
        }

        Ref<Texture> GetEquirectangularMap() const { return m_EquirectangularMap; }
        Ref<Texture> GetEnvironmentMap() const { return m_EnvironmentMap; }
        Ref<Texture> GetIrradianceMap() const { return m_IrradianceMap; }
        Ref<Texture> GetPreFilterMap() const { return m_PreFilterMap; }
        Ref<Texture> GetBrdfLUT() const { return m_BrdfLUT; }

      private:
        Ref<Texture> m_EquirectangularMap;
        Ref<Texture> m_EnvironmentMap;
        Ref<Texture> m_IrradianceMap;
        Ref<Texture> m_PreFilterMap;
        Ref<Texture> m_BrdfLUT;

        Ref<Mesh> m_SkyboxMesh;
        Ref<Shader> m_SkyboxShader;
    };
}