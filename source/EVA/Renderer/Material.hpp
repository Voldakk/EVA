#pragma once

#include "Texture.hpp"
#include "Shader.hpp"
#include "EVA/Assets.hpp"

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

        inline static Ref<Texture> s_DefaultAlbedo;
        inline static Ref<Texture> s_DefaultNormal;
        inline static Ref<Texture> s_DefaultMetallic;
        inline static Ref<Texture> s_DefaultRoughness;
        inline static Ref<Texture> s_DefaultAmbientOcclusion;
        inline static Ref<Texture> s_DefaultEmissive;
        inline static Ref<Texture> s_DefaultHeight;

        glm::vec2 tiling = glm::vec2(1.0f);
        float heightScale = 0.0f;
        bool enableParalax = true;
        bool paralaxClip   = false;

        void Bind(const Ref<Shader> shader)
        {
            if (albedo)
                shader->BindTexture("u_AlbedoMap", albedo);
            else
                shader->BindTexture("u_AlbedoMap", s_DefaultAlbedo);

            if (normal)
                shader->BindTexture("u_NormalMap", normal);
            else
                shader->BindTexture("u_NormalMap", s_DefaultNormal);

            if (metallic)
                shader->BindTexture("u_MetallicMap", metallic);
            else
                shader->BindTexture("u_MetallicMap", s_DefaultMetallic);

            if (roughness)
                shader->BindTexture("u_RoughnessMap", roughness);
            else
                shader->BindTexture("u_RoughnessMap", s_DefaultRoughness);

            if (ambientOcclusion)
                shader->BindTexture("u_AmbientOcclusionMap", ambientOcclusion);
            else
                shader->BindTexture("u_AmbientOcclusionMap", s_DefaultAmbientOcclusion);

            if (emissive)
                shader->BindTexture("u_EmissiveMap", emissive);
            else
                shader->BindTexture("u_EmissiveMap", s_DefaultEmissive);

            if (height)
                shader->BindTexture("u_HeightMap", height);
            else
                shader->BindTexture("u_HeightMap", s_DefaultHeight);
        }

        static void LoadDefaults() 
        { 
            s_DefaultAlbedo = AssetManager::Load<Texture>("textures/default_albedo.png");
            s_DefaultNormal = AssetManager::Load<Texture>("textures/default_normal.png");
            s_DefaultMetallic = AssetManager::Load<Texture>("textures/default_metallic.png");
            s_DefaultRoughness = AssetManager::Load<Texture>("textures/default_roughness.png");
            s_DefaultAmbientOcclusion = AssetManager::Load<Texture>("textures/default_ao.png");
            s_DefaultEmissive = AssetManager::Load<Texture>("textures/default_emission.png");
            s_DefaultHeight = AssetManager::Load<Texture>("textures/default_height.png");
        }
    };
} // namespace EVA