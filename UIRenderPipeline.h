#pragma once
#include "GraphicsPipeline.h"


struct UIVertex {
    glm::vec3 pos;
    glm::vec2 uv;
    uint32_t textureIndex{0};

    //Telling vulkan how to setup binding
    //Struct 1/2
    static VkVertexInputBindingDescription getBindingDescription();
    //Struct 2/2
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    bool operator==(const UIVertex& other) const {
    return pos == other.pos && uv == other.uv;
}

};


namespace std {
    template<>
    struct hash<UIVertex> {
        size_t operator()(UIVertex const& vertex) const {
            size_t seed = 0;

            auto hashCombine = [&seed](auto const& v) {
                seed ^= std::hash<float>()(v)
                      + 0x9e3779b97f4a7c15
                      + (seed << 6)
                      + (seed >> 2);
            };

            hashCombine(vertex.pos.x);
            hashCombine(vertex.pos.y);
            hashCombine(vertex.pos.z);

            hashCombine(vertex.uv.r);
            hashCombine(vertex.uv.g);

            return seed;
        }
    };
}

class UIRenderPipeline : public u_GraphicsPipeline
{
private:
    
public:
    UIRenderPipeline();
    void createGraphicsPipeline() override;
    ~UIRenderPipeline();
};
