#pragma once

#include "GraphicsPipeline.h"


struct RayPoints {
    glm::vec3 pos;
    glm::vec3 color;

    //Telling vulkan how to setup binding
    //Struct 1/2
    static VkVertexInputBindingDescription getBindingDescription();
    //Struct 2/2
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();

    bool operator==(const RayPoints& other) const {
    return pos == other.pos && color == other.color;
}

};


namespace std {
    template<>
    struct hash<RayPoints> {
        size_t operator()(RayPoints const& vertex) const {
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

            hashCombine(vertex.color.r);
            hashCombine(vertex.color.g);
            hashCombine(vertex.color.b);

            return seed;
        }
    };
}

class RayGraphicsPipeline : public u_GraphicsPipeline
{
private:
    
public:
    RayGraphicsPipeline();
    void createGraphicsPipeline() override;
    ~RayGraphicsPipeline();
};
