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
                    std::pair<GridData<uint32_t>, uint32_t> labelData;
                    {
                        EVA_PROFILE_SCOPE("Label");
                        labelData = Label(*data, m_Threshold);
                    }
                    auto [labels, maxLabel] = labelData;
                    GridData<glm::vec4> textureData;
                    {
                        EVA_PROFILE_SCOPE("GenerateTexture");
                        textureData = GenerateTexture(labels, maxLabel);
                    }
                    {
                        EVA_PROFILE_SCOPE("CreateTexture");
                        m_Texture = TextureManager::CreateTexture(textureData.Width(), textureData.Height(), textureData.Data(),
                                                                  TextureRGBA, m_TextureSettings);
                    }
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

            std::pair<GridData<uint32_t>, uint32_t> Label(const GridData<float>& data, float threshold)
            {
                GridData<uint32_t> lables(data.Width(), data.Height(), 0);
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
                    for (int32_t y = 0; y < data.Height(); y++)
                    {
                        for (int32_t x = 0; x < data.Width(); x++)
                        {
                            if (data[y][x] > threshold)
                            {
                                neighbors.clear();
                                if (get(x - 1, y) > threshold) { neighbors.push_back(lables[y][x - 1]); }
                                if (get(x, y - 1) > threshold) { neighbors.push_back(lables[y - 1][x]); }

                                if (neighbors.empty())
                                {
                                    linked[nextLabel] = {nextLabel};
                                    lables[y][x]      = nextLabel;
                                    nextLabel++;
                                }
                                else
                                {
                                    lables[y][x] = neighbors[0];
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
                {
                    EVA_PROFILE_SCOPE("Merge");
                    std::unordered_map<uint32_t, uint32_t> minLinked;
                    for (const auto& [k, v] : linked)
                    {
                        minLinked[k] = *std::min_element(v.begin(), v.end());
                    }
                    for (int32_t y = 0; y < data.Height(); y++)
                    {
                        for (int32_t x = 0; x < data.Width(); x++)
                        {
                            if (lables[y][x] != 0) { lables[y][x] = minLinked[lables[y][x]]; }
                        }
                    }
                }

                uint32_t maxLabel = 0;
                {
                    EVA_PROFILE_SCOPE("Find max");
                    for (const auto& [k, v] : linked)
                    {
                        uint32_t min = *std::min_element(v.begin(), v.end());
                        maxLabel     = glm::max(maxLabel, min);
                    }
                }

                return {lables, maxLabel};
            }
            GridData<glm::vec4> GenerateTexture(const GridData<uint32_t>& labels, uint32_t maxLabel)
            {
                struct Extents
                {
                    uint32_t minX, maxX, minY, maxY;
                };
                std::unordered_map<uint32_t, Extents> extents;
                for (uint32_t i = 1; i <= maxLabel; i++)
                {
                    extents[i] = {labels.Width(), 0, labels.Height(), 0};
                }

                for (uint32_t y = 0; y < labels.Height(); y++)
                {
                    for (uint32_t x = 0; x < labels.Width(); x++)
                    {
                        auto& e = extents[labels[y][x]];

                        e.minX = glm::min(e.minX, x);
                        e.maxX = glm::max(e.maxX, x);

                        e.minY = glm::min(e.minY, y);
                        e.maxY = glm::max(e.maxY, y);
                    }
                }

                float step = 1.0f / maxLabel;
                GridData<glm::vec4> tex(labels.Width(), labels.Height(), glm::vec4(0, 0, 0, 1));
                for (int32_t y = 0; y < labels.Height(); y++)
                {
                    for (int32_t x = 0; x < labels.Width(); x++)
                    {
                        auto l = labels[y][x];
                        if (l != 0)
                        {
                            const auto& e = extents[l];
                            float u       = (float)(x - e.minX) / (e.maxX - e.minX);
                            float v       = (float)(y - e.minY) / (e.maxY - e.minY);
                            float i       = l * step;
                            tex[y][x]     = glm::vec4(u, v, i, 1.0f);
                        }
                    }
                }
                return tex;
            }
        };

        class FloodFillMap : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::FloodFillMap);

          public:
            FloodFillMap()
            {
                SetShader("flood_fill_map.glsl");
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
                SetShader("flood_fill_to_random_grayscale.glsl");
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