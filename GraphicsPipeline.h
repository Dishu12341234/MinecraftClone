#ifndef H_GPIPE
#define H_GPIPE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <array>
#include "PassInfo.hpp"

#include <vector>
#include <fstream>
#include <functional>

//UBO
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

//PCS
struct PushConstantC1
{       
    glm::mat4 data;
};


struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    //Telling vulkan how to setup binding
    //Struct 1/2
    static VkVertexInputBindingDescription getBindingDescription();
    //Struct 2/2
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    bool operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord;
}

};


namespace std {
    template<>
    struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
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

            hashCombine(vertex.texCoord.x);
            hashCombine(vertex.texCoord.y);

            return seed;
        }
    };
}

class u_GraphicsPipeline
{
private:
    VkDevice device;
    int height, width;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass;
    VkDescriptorSetLayout* descriptorSetLayout;
    VkSampleCountFlagBits msaaSamples;
    
    public:
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;//shared
    
    u_GraphicsPipeline();
    ~u_GraphicsPipeline();
    void destroyPipelineLayout();

    void u_PassGraphicsPipelineCreateInfo(u_GraphicsPipelineCreateInfo pCreateInfo);

    static std::vector<char> readFile(const std::string &filename);

    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::vector<char> &code);
};
#endif