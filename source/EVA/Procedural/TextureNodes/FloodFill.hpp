#pragma once

#include "Base.hpp"

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
                m_TextureSettings.wrapping  = TextureWrapping::ClampToEdge;
                m_TextureSettings.minFilter = TextureMinFilter::Nearest;
                m_TextureSettings.magFilter = TextureMagFilter::Nearest;
                // m_Texture                   = TextureManager::CreateTexture(TextureSize, TextureSize, TextureRGBA, m_TextureSettings);
            }
            virtual ~FloodFill() = default;

            bool ProcessTextureNode() override
            {
                EVA_PROFILE_FUNCTION();
                const Ref<Texture>* ref = GetInputDataPtr<Ref<Texture>>(0);

                if (ref != nullptr && *ref)
                {
                    Ref<GridData<float>> data;
                    {
                        EVA_PROFILE_SCOPE("GetDataFromGpu");
                        data = TextureManager::GetDataFromGpu<float>(*ref);
                    }
                    m_Texture = GenerateTexture(*data, m_Threshold);
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
            TextureSettings m_TextureSettings;
            Ref<Texture> m_Texture = nullptr;
            float m_Threshold      = 0.5f;

            Ref<Texture> GenerateTexture(const GridData<float>& data, float threshold)
            {
                uint32_t width = data.Width();
                uint32_t height = data.Height();

                GridData<uint32_t> labels(width, height, 0);
                uint32_t nextLabel = 1;
                std::vector<uint32_t> neighbors;

                std::unordered_map<uint32_t, std::unordered_set<uint32_t>> linked;

                auto get = [&](int32_t x, int32_t y) 
                {
                    if (x < 0 || y < 0) return -1.0f;
                    return data[y][x];
                };

                {
                    EVA_PROFILE_SCOPE("Label");
                    for (int32_t y = 0; y < height; y++)
                    {
                        for (int32_t x = 0; x < width; x++)
                        {
                            if (data[y][x] > threshold)
                            {
                                neighbors.clear();
                                if (get(x - 1, y) > threshold) { neighbors.push_back(labels[y][x - 1]); }
                                if (get(x, y - 1) > threshold) { neighbors.push_back(labels[y - 1][x]); }

                                if (neighbors.empty())
                                {
                                    linked[nextLabel] = {nextLabel};
                                    labels[y][x]      = nextLabel;
                                    nextLabel++;
                                }
                                else
                                {
                                    labels[y][x] = neighbors[0];
                                    {
                                        for (const auto l : neighbors)
                                        {
                                            for (const auto ll : neighbors)
                                            {
                                                if (l != ll) { linked[l].insert(linked[ll].begin(), linked[ll].end()); }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                std::unordered_set<uint32_t> minLabels;
                std::unordered_map<uint32_t, uint32_t> minLabelsMap;

                std::unordered_map<uint32_t, uint32_t> minLinked;
                {
                    EVA_PROFILE_SCOPE("Merge");

                    for (const auto& [k, v] : linked)
                    {
                        auto min = *std::min_element(v.begin(), v.end());
                        minLinked[k] = min;
                        minLabels.insert(min);
                    }
                    minLabels.erase(0);

                    uint32_t index = 1;
                    for (const auto& l : minLabels) 
                    {
                        minLabelsMap[l] = index++;
                    }

                    for (int32_t y = 0; y < data.Height(); y++)
                    {
                        for (int32_t x = 0; x < data.Width(); x++)
                        {
                            if (labels[y][x] != 0) { labels[y][x] = minLabelsMap[minLinked[labels[y][x]]]; }
                        }
                    }
                }

                uint32_t maxLabel = minLabels.size();

                struct Extents
                {
                    uint32_t minX, maxX, minY, maxY;
                };

                std::vector<Extents> extents(maxLabel, {labels.Width(), 0, labels.Height(), 0});
                {
                    EVA_PROFILE_SCOPE("Find extents");

                    for (uint32_t y = 0; y < labels.Height(); y++)
                    {
                        for (uint32_t x = 0; x < labels.Width(); x++)
                        {
                            int32_t l = labels[y][x];
                            if (l == 0) continue;
                            auto& e = extents[l - 1];

                            e.minX = glm::min(e.minX, x);
                            e.maxX = glm::max(e.maxX, x);

                            e.minY = glm::min(e.minY, y);
                            e.maxY = glm::max(e.maxY, y);
                        }
                    }
                }

                Ref<Texture> texture;
                {
                    EVA_PROFILE_SCOPE("GenerateTexture");

                    texture = TextureManager::CreateTexture(labels.Width(), labels.Height(), TextureRGBA, m_TextureSettings);

                    auto shader = AssetManager::Load<Shader>(std::string(ShaderPath) + "flood_fill/gen_texture.glsl");
                    shader->Bind();

                    auto labelSsbo = ShaderStorageBuffer::Create(labels.Data(), labels.Size());
                    shader->SetUniformInt("u_LabelCount", labels.Count());
                    shader->BindStorageBuffer(1, labelSsbo);

                    auto extentsSsbo = ShaderStorageBuffer::Create(extents.data(), extents.size() * sizeof(Extents));
                    shader->SetUniformInt("u_ExtentsCount", extents.size());
                    shader->BindStorageBuffer(2, extentsSsbo);

                    float step = 1.0f / maxLabel;
                    shader->SetUniformFloat("u_Step", step);

                    shader->BindImageTexture(0, texture, TextureAccess::WriteOnly);

                    uint32_t workGroupSize  = 16;
                    uint32_t numWorkGroupsX = (texture->GetWidth() + workGroupSize - 1) / workGroupSize;
                    uint32_t numWorkGroupsY = (texture->GetHeight() + workGroupSize - 1) / workGroupSize;

                    shader->DispatchCompute(numWorkGroupsX, numWorkGroupsY, 1, workGroupSize, workGroupSize, 1);
                }
                return texture;
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