#pragma once

#include "TextureManager.hpp"
#include "EVA/Renderer/Framebuffer.hpp"
#include "Mesh.hpp"

namespace EVA
{
    class TextureUtilities
    {
        inline static Ref<VertexArray> s_QuadVAO = nullptr;
        inline static Ref<Mesh> s_Cube           = nullptr;
        inline static void renderQuad()
        {
            EVA_PROFILE_FUNCTION();

            if (s_QuadVAO == nullptr)
            {
                float vertices[] = {
                  // x, y, z, u, v
                  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
                  1.0f,  1.0f,  0.0f, 1.0f, 1.0f, -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
                };

                uint32_t indices[] = {0, 1, 2, 0, 2, 3};

                s_QuadVAO = EVA::VertexArray::Create();

                auto vb = EVA::VertexBuffer::Create(&vertices[0], sizeof(vertices));

                EVA::BufferLayout layout = {{EVA::ShaderDataType::Float3, "a_Position"}, {EVA::ShaderDataType::Float2, "a_TexCoords"}};

                vb->SetLayout(layout);
                s_QuadVAO->AddVertexBuffer(vb);

                auto ib = EVA::IndexBuffer::Create(&indices[0], 6);
                s_QuadVAO->SetIndexBuffer(ib);
            }
            s_QuadVAO->Bind();
            RenderCommand::Clear();
            RenderCommand::DrawIndexed(s_QuadVAO);
        }
        inline static void renderCube()
        {
            EVA_PROFILE_FUNCTION();

            if (s_Cube == nullptr) { s_Cube = Mesh::LoadMesh("./assets/models/cube.obj")[0]; }

            s_Cube->GetVertexArray()->Bind();
            RenderCommand::Clear();
            RenderCommand::DrawIndexed(s_Cube->GetVertexArray());
        }

        inline static void ConvertCube(std::shared_ptr<Texture> in, std::string inName, std::shared_ptr<Texture> out, std::shared_ptr<Shader> shader)
        {
            EVA_PROFILE_FUNCTION();

            FramebufferSpecification spec;
            spec.width       = out->GetWidth();
            spec.height      = out->GetHeight();
            auto frameBuffer = Framebuffer::Create(spec);

            glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            glm::mat4 captureViews[] = {glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                                        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                                        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                                        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                                        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                                        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

            shader->Bind();
            shader->BindTexture(inName, in);

            frameBuffer->Bind();

            RenderCommand::SetCullMode(CullMode::None);

            for (unsigned int i = 0; i < 6; ++i)
            {
                shader->SetUniformMat4("u_Projection", captureProjection);
                shader->SetUniformMat4("u_View", captureViews[i]);

                frameBuffer->AttachCubemap(out, i);
                renderCube();
            }
        }

      public:
        inline static std::shared_ptr<Texture> EquirectangularToCubemap(std::shared_ptr<Texture> hdrTexture, uint32_t size = 512)
        {
            EVA_PROFILE_FUNCTION();

            auto shader = Shader::Create("./assets/shaders/equirectangular_to_cubemap.glsl");
            TextureSettings settings;
            settings.wrapping  = TextureWrapping::ClampToEdge;
            settings.minFilter = TextureMinFilter::LinearMipmapLinear;
            auto out           = TextureManager::CreateCubeMap(size, size, TextureFormat::RGB16F, settings);

            ConvertCube(hdrTexture, "u_EquirectangularMap", out, shader);

            TextureManager::GenerateMipMaps(out);

            return out;
        }

        inline static std::shared_ptr<Texture> ConvoluteCubemap(std::shared_ptr<Texture> cubemap, uint32_t size = 32)
        {
            EVA_PROFILE_FUNCTION();

            auto shader = Shader::Create("./assets/shaders/cubemap_convolution.glsl");
            auto out    = TextureManager::CreateCubeMap(size, size, TextureFormat::RGB16F);

            ConvertCube(cubemap, "u_EnvironmentMap", out, shader);

            return out;
        }

        inline static std::shared_ptr<Texture> PreFilterEnviromentMap(std::shared_ptr<Texture> hdrTexture, uint32_t size = 128)
        {
            EVA_PROFILE_FUNCTION();

            auto shader = Shader::Create("./assets/shaders/pre_filter_map.glsl");
            TextureSettings settings;
            settings.minFilter = TextureMinFilter::LinearMipmapLinear;
            auto out           = TextureManager::CreateCubeMap(size, size, TextureFormat::RGB16F, settings);
            TextureManager::GenerateMipMaps(out);

            glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            glm::mat4 captureViews[] = {glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                                        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                                        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                                        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                                        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                                        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

            shader->Bind();
            shader->BindTexture("u_EnvironmentMap", hdrTexture);
            shader->SetUniformMat4("u_Projection", captureProjection);
            shader->SetUniformFloat("u_Resolution", (float)hdrTexture->GetWidth());

            FramebufferSpecification spec;
            spec.width       = out->GetWidth();
            spec.height      = out->GetHeight();
            auto frameBuffer = Framebuffer::Create(spec);
            frameBuffer->Bind();

            RenderCommand::SetCullMode(CullMode::None);

            unsigned int maxMipLevels = 5;
            for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
            {
                // Reisze framebuffer according to mip-level size.
                uint32_t mipWidth  = size * std::pow(0.5, mip);
                uint32_t mipHeight = size * std::pow(0.5, mip);

                frameBuffer->Resize(mipWidth, mipHeight);

                float roughness = (float)mip / (float)(maxMipLevels - 1);
                shader->SetUniformFloat("u_Roughness", roughness);
                for (unsigned int i = 0; i < 6; ++i)
                {
                    shader->SetUniformMat4("u_View", captureViews[i]);
                    frameBuffer->AttachCubemap(out, i, mip);

                    RenderCommand::Clear();
                    renderCube();
                }
            }

            return out;
        }

        inline static std::shared_ptr<Texture> PreComputeBRDF(uint32_t size = 512)
        {
            EVA_PROFILE_FUNCTION();

            return TextureManager::LoadTexture("./assets/textures/ibl_brdf_lut.png");

            /*auto shader = Shader::Create("pre_compute_brdf.glsl");

            TextureSettings settings;
            settings.wrapping = TextureWrapping::ClampToEdge;
            auto out          = TextureManager::CreateTexture(size, size, TextureFormat::RG16F, settings);

            FramebufferSpecification spec;
            spec.width       = out->GetWidth();
            spec.height      = out->GetHeight();
            auto frameBuffer = Framebuffer::Create(spec);
            frameBuffer->Bind();

            frameBuffer->AttachTexture(out);

            shader->Bind();

            RenderCommand::Clear();
            renderQuad();


            return out;*/
        }
    };
} // namespace EVA