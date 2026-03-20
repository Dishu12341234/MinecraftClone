#pragma once
#include "Structs.h"
#include "PassInfo.hpp"
#include "UIRenderPipeline.h"

class UIComponents
{
private:
    VulkanContext &vkContext;
    glm::vec2 position{0.f, 0.f};
    glm::vec2 size{1.f, 1.f};

    // Vertices and stuff
    std::vector<UIVertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;

    VkBuffer StagingVertexBuffer = VK_NULL_HANDLE;
    VkBuffer StagingIndexBuffer = VK_NULL_HANDLE;

    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

    VkDeviceMemory StagingVertexBufferMemory = VK_NULL_HANDLE;
    VkDeviceMemory StagingIndexBufferMemory = VK_NULL_HANDLE;

    VkDeviceSize stagingVertexBufferSize = 0;
    VkDeviceSize stagingIndexBufferSize = 0;

    void *stagingIndexBufferData;
    void *stagingVertexBufferData;

    void createVertexBuffer();
    void createIndexBuffer();

    int textureIDX{0};

public:
    UIComponents(VulkanContext &vkContext);

    void initUIComponent(glm::vec2 position = glm::vec2{0.f, 0.f}, glm::vec2 size=glm::vec2{1.f, 1.f});

    void setTextureIDX(int idx);

    void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
              std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, PushConstantC1 &c1);
    void cleanup();
    ~UIComponents();
};
