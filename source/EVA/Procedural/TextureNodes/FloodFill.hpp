#pragma once

#include "Base.hpp"
#include "Platform/OpenGL/OpenGLContext.hpp"
namespace EVA
{
    namespace TextureNodes
    {
        class FloodFill : public TextureNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::FloodFill);

          public:
            FloodFill()
            {
                TextureSettings textureSettings;
                textureSettings.wrapping    = TextureWrapping::ClampToEdge;
                textureSettings.minFilter   = TextureMinFilter::Nearest;
                textureSettings.magFilter   = TextureMagFilter::Nearest;
                m_Texture                   = TextureManager::CreateTexture(TextureSize, TextureSize, TextureRGBA, textureSettings);
                m_LabelsTexture             = TextureManager::CreateTexture(TextureSize, TextureSize, TextureFormat::R32UI);

                uint32_t nextIndex = 1;
                m_NextIndexSsbo = ShaderStorageBuffer::Create(&nextIndex, sizeof(nextIndex), Usage::DeviceModifiedRepeatedlyAppUsedRepeatedly);
            
                m_LabelShader      = AssetManager::Load<Shader>(std::string(ShaderPath) + "flood_fill/label.glsl");
                m_FindLinksShader  = AssetManager::Load<Shader>(std::string(ShaderPath) + "flood_fill/find_links.glsl");
                m_ApplyLinksShader = AssetManager::Load<Shader>(std::string(ShaderPath) + "flood_fill/apply_links.glsl");
                m_ExtentsShader = AssetManager::Load<Shader>(std::string(ShaderPath) + "flood_fill/extents.glsl");
                m_GenTextureShader = AssetManager::Load<Shader>(std::string(ShaderPath) + "flood_fill/gen_texture.glsl");
            }
            virtual ~FloodFill() = default;

            bool ProcessTextureNode() override
            {
                EVA_PROFILE_FUNCTION();
                const Ref<Texture>* ref = GetInputDataPtr<Ref<Texture>>(0);

                if (ref != nullptr && *ref)
                {
                    GenerateTexture(*ref);
                    return true;
                }
                return false;
            }

            void SetupNode() override
            {
                TextureNode::SetupNode();
                name = "Flood fill";
                AddOutput<Ref<Texture>, 4>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"In"});
            }

            void Serialize(DataObject& data) override
            {
                TextureNode::Serialize(data);

                data.Serialize("Threshold", m_Threshold);

                processed &= !data.changed;
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

          private:
            float m_Threshold = 0.5f;

            Ref<Texture> m_Texture;
            Ref<Texture> m_LabelsTexture;
            Ref<ShaderStorageBuffer> m_NextIndexSsbo;

            Ref<Shader> m_LabelShader, m_FindLinksShader, m_ApplyLinksShader, m_ExtentsShader, m_GenTextureShader;

            void GenerateTexture(const Ref<Texture>& dataTexture)
            {
                constexpr glm::ivec2 pixelsPerChunk = glm::ivec2(32);
                constexpr glm::ivec2 workGroupSize   = glm::ivec2(16);

                glm::ivec2 dims    = glm::ivec2(dataTexture->GetWidth(), dataTexture->GetHeight());
                glm::ivec2 threads = (dims + pixelsPerChunk - 1) / pixelsPerChunk;

                glm::ivec2 numWorkGroupsSingle = (dims + workGroupSize - 1) / workGroupSize;
                glm::ivec2 numWorkGroupsChunk  = (threads + workGroupSize - 1) / workGroupSize;

                uint32_t nextIndex = 1;
                
                {
                    EVA_PROFILE_SCOPE("Label");


                    m_LabelShader->Bind();

                    m_LabelShader->BindImageTexture(0, dataTexture, Access::ReadOnly);
                    m_LabelShader->BindImageTexture(1, m_LabelsTexture, Access::WriteOnly);

                    m_NextIndexSsbo->BufferData(&nextIndex, sizeof(nextIndex));
                    m_LabelShader->BindStorageBuffer(2, m_NextIndexSsbo);

                    m_LabelShader->DispatchCompute(numWorkGroupsChunk.x, numWorkGroupsChunk.y, 1, workGroupSize.x, workGroupSize.y, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

                    nextIndex = *static_cast<uint32_t*>(m_NextIndexSsbo->Map(Access::ReadOnly));
                    m_NextIndexSsbo->Unmap();
                }
                
                {
                    EVA_PROFILE_SCOPE("Link");
                    uint32_t prev = 0;
                    while (prev != nextIndex)
                    {
                        prev = nextIndex;
                        std::vector<uint32_t> links(nextIndex);
                        for (size_t i = 0; i < links.size(); i++)
                        {
                            links[i] = i;
                        }
                        auto linksSsbo = ShaderStorageBuffer::Create(links.data(), links.size() * sizeof(uint32_t));
                        {
                            EVA_PROFILE_SCOPE("Find links");

                            m_FindLinksShader->Bind();

                            m_FindLinksShader->BindImageTexture(0, m_LabelsTexture, Access::ReadOnly);
                            m_FindLinksShader->BindStorageBuffer(1, linksSsbo);

                            m_FindLinksShader->DispatchCompute(numWorkGroupsChunk.x, numWorkGroupsChunk.y, 1, workGroupSize.x, workGroupSize.y, 1);
                            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                        }

                        uint32_t* linksData = static_cast<uint32_t*>(linksSsbo->Map(Access::ReadWrite));

                        nextIndex = 1;
                        for (size_t i = 1; i < links.size(); i++)
                        {
                            auto& l = linksData[i];
                            l       = l == i ? nextIndex++ : linksData[l];
                        }
                        linksData[0] = 0;

                        linksSsbo->Unmap();

                        {
                            EVA_PROFILE_SCOPE("Apply links");

                            m_ApplyLinksShader->Bind();

                            m_ApplyLinksShader->BindImageTexture(0, m_LabelsTexture, Access::ReadWrite);
                            m_ApplyLinksShader->BindStorageBuffer(1, linksSsbo);

                            m_ApplyLinksShader->DispatchCompute(numWorkGroupsSingle.x, numWorkGroupsSingle.y, 1, workGroupSize.x,
                                                                workGroupSize.y, 1);
                            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                        }
                    }
                }

                struct Extents
                {
                    uint32_t minX, maxX, minY, maxY;
                };

                std::vector<Extents> extents(nextIndex, {static_cast<uint32_t>(dims.x), 0, static_cast<uint32_t>(dims.y), 0});
                auto extentsSsbo = ShaderStorageBuffer::Create(extents.data(), extents.size() * sizeof(Extents));
                {
                    EVA_PROFILE_SCOPE("Find extents");

                    m_ExtentsShader->Bind();

                    m_ExtentsShader->BindImageTexture(0, m_LabelsTexture, Access::ReadOnly);
                    m_ExtentsShader->BindStorageBuffer(1, extentsSsbo);

                    m_ExtentsShader->DispatchCompute(numWorkGroupsSingle.x, numWorkGroupsSingle.y, 1, workGroupSize.x, workGroupSize.y, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                }

                {
                    EVA_PROFILE_SCOPE("Generate texture");

                    m_GenTextureShader->Bind();

                    m_GenTextureShader->BindImageTexture(0, m_Texture, Access::WriteOnly);
                    m_GenTextureShader->BindImageTexture(1, m_LabelsTexture, Access::ReadOnly);
                    m_GenTextureShader->BindStorageBuffer(2, extentsSsbo);

                    float step = 1.0f / nextIndex;
                    m_GenTextureShader->SetUniformFloat("u_Step", step);

                    m_GenTextureShader->DispatchCompute(numWorkGroupsSingle.x, numWorkGroupsSingle.y, 1, workGroupSize.x, workGroupSize.y, 1);
                }
            }
        };

        class FloodFillMap : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::FloodFillMap);

          public:
            FloodFillMap()
            {
                SetShader("flood_fill/map.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Flood fill map";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 4>({"Flood fill"});
                AddInput<Ref<Texture>, 1>({"Map"});
            }

            void SetUniforms() const override
            {
                const Ref<Texture>& ref = GetInputData<Ref<Texture>>(1);
                m_Shader->BindTexture("u_MapSampler", ref);
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                processed &= !data.changed;
            }
        };

        class FloodFillToRandomGrayscale : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::FloodFillToRandomGrayscale);

          public:
            FloodFillToRandomGrayscale()
            {
                SetShader("flood_fill/random_grayscale.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Flood fill to random";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 4>({"Flood fill"});
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformFloat("u_MinValue", m_MinValue);
                m_Shader->SetUniformFloat("u_MaxValue", m_MaxValue);
                m_Shader->SetUniformFloat("u_Seed", m_Seed);
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                data.Serialize("MinValue", m_MinValue);
                data.Serialize("MaxValue", m_MaxValue);
                data.Serialize("Seed", m_Seed);

                processed &= !data.changed;
            }

          private:
            float m_MinValue = 0.0f;
            float m_MaxValue = 1.0f;
            float m_Seed     = 0.0f;
        };
    } // namespace TextureNodes
} // namespace EVA