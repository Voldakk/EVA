#pragma once

#include "Base.hpp"
#include "Platform/OpenGL/OpenGLContext.hpp"
namespace EVA
{
    namespace TextureNodes
    {
        struct CCLData
        {
            uint32_t count;
            Ref<Texture> labelsTexture;
            Ref<Texture> extentsTexture;
        };

        class CCL : public TextureNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::CCL);

          public:
            CCL()
            {
                TextureSettings textureSettings;
                textureSettings.wrapping    = TextureWrapping::ClampToEdge;
                textureSettings.minFilter   = TextureMinFilter::Nearest;
                textureSettings.magFilter   = TextureMagFilter::Nearest;
                m_Data.labelsTexture        = TextureManager::CreateTexture(TextureSize, TextureSize, TextureFormat::R32UI);
                m_Data.extentsTexture       = TextureManager::CreateTexture(TextureSize, TextureSize, TextureRGBA, textureSettings);

                uint32_t nextIndex = 1;
                m_NextIndexSsbo = ShaderStorageBuffer::Create(&nextIndex, sizeof(nextIndex), Usage::DeviceModifiedRepeatedlyAppUsedRepeatedly);
            
                m_LabelShader      = AssetManager::Load<Shader>(std::string(ShaderPath) + "ccl/label.glsl");
                m_FindLinksShader  = AssetManager::Load<Shader>(std::string(ShaderPath) + "ccl/find_links.glsl");
                m_ApplyLinksShader = AssetManager::Load<Shader>(std::string(ShaderPath) + "ccl/apply_links.glsl");
                m_ExtentsShader = AssetManager::Load<Shader>(std::string(ShaderPath) + "ccl/extents.glsl");
                m_ExtentsTextureShader = AssetManager::Load<Shader>(std::string(ShaderPath) + "ccl/extents_texture.glsl");
            }

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
                name = "CCL";
                AddOutput<CCLData>({"Out", &m_Data});
                AddInput<Ref<Texture>, 1>({"In"});
            }

            void Serialize(DataObject& data) override
            {
                TextureNode::Serialize(data);

                data.Serialize("Threshold", m_Threshold);

                processed &= !data.changed;
            }

            Ref<Texture> GetTexture() const override { return m_Data.extentsTexture; }

          private:
            float m_Threshold = 0.5f;

            CCLData m_Data;
            Ref<ShaderStorageBuffer> m_NextIndexSsbo;

            Ref<Shader> m_LabelShader, m_FindLinksShader, m_ApplyLinksShader, m_ExtentsShader, m_ExtentsTextureShader;

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
                    m_LabelShader->ResetTextureUnit();
                    m_LabelShader->BindTexture("u_Input", dataTexture);
                    m_LabelShader->BindImageTexture(1, m_Data.labelsTexture, Access::WriteOnly);

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

                            m_FindLinksShader->BindImageTexture(0, m_Data.labelsTexture, Access::ReadOnly);
                            m_FindLinksShader->BindStorageBuffer(1, linksSsbo);
                            m_FindLinksShader->SetUniformBool("u_Wrap", false);
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

                            m_ApplyLinksShader->BindImageTexture(0, m_Data.labelsTexture, Access::ReadWrite);
                            m_ApplyLinksShader->BindStorageBuffer(1, linksSsbo);

                            m_ApplyLinksShader->DispatchCompute(numWorkGroupsSingle.x, numWorkGroupsSingle.y, 1, workGroupSize.x,
                                                                workGroupSize.y, 1);
                            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                        }
                    }
                }

                struct Extents
                {
                    uint32_t minX, minY, maxX, maxY;
                };

                std::vector<Extents> extents(nextIndex, {static_cast<uint32_t>(dims.x), static_cast<uint32_t>(dims.y), 0, 0});
                auto extentsSsbo = ShaderStorageBuffer::Create(extents.data(), extents.size() * sizeof(Extents));
                {
                    EVA_PROFILE_SCOPE("Find extents");

                    m_ExtentsShader->Bind();

                    m_ExtentsShader->BindImageTexture(0, m_Data.labelsTexture, Access::ReadOnly);
                    m_ExtentsShader->BindStorageBuffer(1, extentsSsbo);

                    m_ExtentsShader->DispatchCompute(numWorkGroupsSingle.x, numWorkGroupsSingle.y, 1, workGroupSize.x, workGroupSize.y, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                }

                
                std::vector<uint32_t> links(nextIndex);
                Ref<ShaderStorageBuffer> linksSsbo;
                
                {
                    EVA_PROFILE_SCOPE("Find links");

                    for (size_t i = 0; i < links.size(); i++)
                    {
                        links[i] = i;
                    }
                    linksSsbo = ShaderStorageBuffer::Create(links.data(), links.size() * sizeof(uint32_t));

                    m_FindLinksShader->Bind();

                    m_FindLinksShader->BindImageTexture(0, m_Data.labelsTexture, Access::ReadOnly);
                    m_FindLinksShader->BindStorageBuffer(1, linksSsbo);
                    m_FindLinksShader->SetUniformBool("u_Wrap", true);
                    m_FindLinksShader->DispatchCompute(numWorkGroupsChunk.x, numWorkGroupsChunk.y, 1, workGroupSize.x, workGroupSize.y, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                }

                std::unordered_map<uint32_t, std::set<uint32_t>> linkedExtents;
                {
                    EVA_PROFILE_SCOPE("Compact labels");
                    uint32_t* linksData = static_cast<uint32_t*>(linksSsbo->Map(Access::ReadWrite));

                    nextIndex = 1;
                    for (size_t i = 1; i < links.size(); i++)
                    {
                        auto& l = linksData[i];
                        if (l == i) 
                        { 
                            l = nextIndex++;
                        }
                        else
                        {
                            linkedExtents[i].insert(i);
                            linkedExtents[l].insert(l);
                            
                            for (const auto& n : linkedExtents[i])
                            {
                                linkedExtents[n].insert(l);
                            }
                            for (const auto& n : linkedExtents[l])
                            {
                                linkedExtents[n].insert(i);
                            }
                            linkedExtents[i].insert(linkedExtents[l].begin(), linkedExtents[l].end());
                            linkedExtents[l].insert(linkedExtents[i].begin(), linkedExtents[i].end());

                            l = linksData[l];
                        }
                    }
                    linksData[0] = 0;

                    linksSsbo->Unmap();
                }
                {
                    EVA_PROFILE_SCOPE("Combine extents");

                    std::vector<std::set<uint32_t>> uniqueLinkedExtents;
                    for (const auto& [k, v] : linkedExtents)
                    {
                        uniqueLinkedExtents.push_back(v);
                    }
                    std::sort(uniqueLinkedExtents.begin(), uniqueLinkedExtents.end());
                    auto last = std::unique(uniqueLinkedExtents.begin(), uniqueLinkedExtents.end());
                    uniqueLinkedExtents.resize(last - uniqueLinkedExtents.begin());
                    Extents* extentsData = static_cast<Extents*>(extentsSsbo->Map(Access::ReadWrite));
                    for (const auto& set : uniqueLinkedExtents) 
                    {
                        bool ex = false, ey = false;

                        glm::uvec2 min = glm::uvec2(dims);
                        glm::uvec2 max = glm::uvec2(0);

                        glm::uvec2 m = glm::uvec2(dims);
                        glm::uvec2 d   = glm::uvec2(0);

                        for (const auto& l : set) 
                        {
                            auto* e = &extentsData[l - 1];
                            min.x   = glm::min(min.x, e->minX);
                            max.x   = glm::max(max.x, e->maxX);
                            min.y   = glm::min(min.y, e->minY);
                            max.y   = glm::max(max.y, e->maxY);

                            if (e->minX == 0)
                            {
                                d.x = glm::max(d.x, e->maxX);
                                ex = true;
                            }
                            if (e->maxX == dims.x - 1)
                            {
                                m.x = glm::min(m.x, e->minX);
                                ex = true;
                            }

                            if (e->minY == 0)
                            {
                                d.y = glm::max(d.y, e->maxY);
                                ey = true;
                            }
                            if (e->maxY == dims.y - 1)
                            {
                                m.y = glm::min(m.y, e->minY);
                                ey = true;
                            }
                        }

                        for (const auto& l : set) 
                        {
                            auto* e = &extentsData[l - 1];
                            if (ex)
                            {
                                e->minX = m.x;
                                e->maxX = dims.x + d.x;
                            }
                            else
                            {
                                e->minX = min.x;
                                e->maxX = max.x;
                            }

                            if (ey)
                            {
                                e->minY = m.y;
                                e->maxY = dims.y + d.y;
                            }
                            else
                            {
                                e->minY = min.y;
                                e->maxY = max.y;
                            }
                        }
                    }
                    extentsSsbo->Unmap();
                }

                {
                    EVA_PROFILE_SCOPE("Generate extents map");

                    m_ExtentsTextureShader->Bind();

                    m_ExtentsTextureShader->BindImageTexture(0, m_Data.extentsTexture, Access::WriteOnly);
                    m_ExtentsTextureShader->BindImageTexture(1, m_Data.labelsTexture, Access::ReadOnly);
                    m_ExtentsTextureShader->BindStorageBuffer(2, extentsSsbo);

                    m_ExtentsTextureShader->DispatchCompute(numWorkGroupsSingle.x, numWorkGroupsSingle.y, 1, workGroupSize.x, workGroupSize.y, 1);
                }

                {
                    EVA_PROFILE_SCOPE("Apply links");

                    m_ApplyLinksShader->Bind();

                    m_ApplyLinksShader->BindImageTexture(0, m_Data.labelsTexture, Access::ReadWrite);
                    m_ApplyLinksShader->BindStorageBuffer(1, linksSsbo);

                    m_ApplyLinksShader->DispatchCompute(numWorkGroupsSingle.x, numWorkGroupsSingle.y, 1, workGroupSize.x, workGroupSize.y, 1);
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                }

                m_Data.count = nextIndex - 1;
            }
        };

        class CCLNode : public ComputeNode
        {
          public:
            void SetupNode() override
            {
                ComputeNode::SetupNode();
                AddInput<CCLData>({"In"});
            }

            const CCLData* GetCCLData() const 
            { 
                return GetInputDataPtr<CCLData>(0);
            }
        };

        class CCLToRandom : public CCLNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::CCLToRandom);

          public:
            CCLToRandom()
            {
                //SetShader("ccl/to_random_grayscale.glsl");
                //SetTexture(TextureR);
            }

            void SetupNode() override
            {
                CCLNode::SetupNode();
                name = "CCL to random";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
            }

            bool ProcessTextureNode() override
            {
                if (m_Grayscale)
                {
                    SetShader("ccl/to_random_grayscale.glsl");
                    SetTexture(TextureR);
                    SetOutputType<Ref<Texture>, 1>(0);
                }
                else
                {
                    SetShader("ccl/to_random_color.glsl");
                    SetTexture(TextureRGBA);
                    SetOutputType<Ref<Texture>, 4>(0);
                }

                return CCLNode::ProcessTextureNode();
            }

            void SetUniforms() const override
            {
                m_Shader->BindImageTexture(1, GetCCLData()->extentsTexture, Access::ReadOnly);
                m_Shader->SetUniformFloat("u_MinValue", m_MinValue);
                m_Shader->SetUniformFloat("u_MaxValue", m_MaxValue);
                m_Shader->SetUniformFloat("u_Seed", m_Seed);
            }

            void Serialize(DataObject& data) override
            {
                CCLNode::Serialize(data);

                data.Serialize("Grayscale", m_Grayscale);
                data.Serialize("MinValue", m_MinValue);
                data.Serialize("MaxValue", m_MaxValue);
                data.Serialize("Seed", m_Seed);

                processed &= !data.changed;
            }

          private:
            bool m_Grayscale = true;
            float m_MinValue = 0.0f;
            float m_MaxValue = 1.0f;
            float m_Seed     = 0.0f;
        };

        class CCLToGradient : public CCLNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::CCLToGradient);

          public:
            CCLToGradient()
            {
                SetShader("ccl/to_gradient.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                CCLNode::SetupNode();
                name = "CCL to gradient";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
            }

            void SetUniforms() const override 
            { 
                m_Shader->BindImageTexture(1, GetCCLData()->extentsTexture, Access::ReadOnly); 
                m_Shader->SetUniformFloat("u_Angle", m_Angle);
                m_Shader->SetUniformFloat("u_AngleVariation", m_AngleVariation);
                m_Shader->SetUniformFloat("u_Seed", m_Seed);
            }

            void Serialize(DataObject& data) override
            {
                CCLNode::Serialize(data);

                if (data.Inspector()) 
                { 
                    data.changed |= ImGui::SliderAngle("Angle", &m_Angle);
                    data.changed |= ImGui::SliderFloat("AngleVariation", &m_AngleVariation, 0, 1); 
                }
                else
                {
                    data.Serialize("Angle", m_Angle);
                    data.Serialize("AngleVariation", m_AngleVariation);
                }
                
                data.Serialize("Seed", m_Seed);

                processed &= !data.changed;
            }

          private:
            float m_Angle = 0;
            float m_AngleVariation = 0;
            float m_Seed = 0;
        };

        class CCLMap : public CCLNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::CCLMap);

          public:
            CCLMap()
            {
                SetShader("ccl/to_map.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                CCLNode::SetupNode();
                name = "CCL map";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"Map"});
            }

            void SetUniforms() const override
            {
                m_Shader->BindImageTexture(1, GetCCLData()->extentsTexture, Access::ReadOnly);
                m_Shader->BindTexture("u_MapSampler", GetInputData<Ref<Texture>>(1));
            }
        };

        class CCLToIndex : public CCLNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::CCLToIndex);

          public:
            CCLToIndex()
            {
                SetShader("ccl/to_index.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                CCLNode::SetupNode();
                name = "CCL to index";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
            }

            void SetUniforms() const override 
            { 
                auto data = GetCCLData();
                m_Shader->BindImageTexture(1, data->labelsTexture, Access::ReadOnly); 
                m_Shader->SetUniformFloat("u_Step", 1.0f / data->count);
            }
        };

        class CCLToPosition : public CCLNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::CCLToPosition);

          public:
            CCLToPosition()
            {
                SetShader("ccl/to_position.glsl");
                SetTexture(TextureRG);
            }

            void SetupNode() override
            {
                CCLNode::SetupNode();
                name = "CCL to position";
                AddOutput<Ref<Texture>, 2>({"Out", &m_Texture});
            }

            void SetUniforms() const override { m_Shader->BindImageTexture(1, GetCCLData()->extentsTexture, Access::ReadOnly); }
        };
    } // namespace TextureNodes
} // namespace EVA