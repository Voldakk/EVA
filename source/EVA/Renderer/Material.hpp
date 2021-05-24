#pragma once

#include "Texture.hpp"
#include "Shader.hpp"

namespace EVA
{
    struct Material
    {
        Ref<Texture> albedo;
        Ref<Texture> normal;
        Ref<Texture> metallic;
        Ref<Texture> roughness;
        Ref<Texture> ambientOcclusion;
        Ref<Texture> emissive;
        Ref<Texture> height;

        bool enableParalax = true;
        bool paralaxClip   = false;
        float heightScale = 0.0f;

        glm::vec2 tiling = glm::vec2(1.0f);

        void Bind(const Ref<Shader> shader)
        {
            if (albedo)
                shader->BindTexture("u_AlbedoMap", albedo);
            else
                shader->BindTexture("u_AlbedoMap", TextureTarget::Texture2D, 0);

            if (normal)
                shader->BindTexture("u_NormalMap", normal);
            else
                shader->BindTexture("u_NormalMap", TextureTarget::Texture2D, 0);

            if (metallic)
                shader->BindTexture("u_MetallicMap", metallic);
            else
                shader->BindTexture("u_MetallicMap", TextureTarget::Texture2D, 0);

            if (roughness)
                shader->BindTexture("u_RoughnessMap", roughness);
            else
                shader->BindTexture("u_RoughnessMap", TextureTarget::Texture2D, 0);

            if (ambientOcclusion)
                shader->BindTexture("u_AmbientOcclusionMap", ambientOcclusion);
            else
                shader->BindTexture("u_AmbientOcclusionMap", TextureTarget::Texture2D, 0);

            if (emissive)
                shader->BindTexture("u_EmissiveMap", emissive);
            else
                shader->BindTexture("u_EmissiveMap", TextureTarget::Texture2D, 0);

            if (height)
                shader->BindTexture("u_HeightMap", height);
            else
                shader->BindTexture("u_HeightMap", TextureTarget::Texture2D, 0);

            shader->SetUniformBool("u_EnableParalax", enableParalax && height);
            shader->SetUniformBool("u_ParalaxClip", paralaxClip);
            shader->SetUniformFloat("u_HeightScale", heightScale);
            shader->SetUniformFloat2("u_Tiling", tiling);
        }
    };
} // namespace EVA