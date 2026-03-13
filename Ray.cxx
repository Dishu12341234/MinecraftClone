#include "Ray.h"
#include "GameObjectPool.h"
#include <vector>
#include "GraphicsPipeline.h"

Ray::Ray(VulkanContext &vkContext, GameObjectPool &gameObjectPool) : vkContext{vkContext}, gameObjectPool{gameObjectPool}, rayMesher{vkContext}
{
    std::vector<RayPoints> vertices = {
        {{0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}},
        {{1.f, 0.f, 0.f}, {0.2f, 1.f, 0.f}},
        {{1.f, 1.f, 0.f}, {1.f, 1.f, 0.2f}},
        {{0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}},
    };

    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
    rayMesher.loadRayPoints(vertices);
    rayMesher.loadIndices(indices);

    rayMesher.createVertexBuffer();
    rayMesher.createIndexBuffer();
}

void Ray::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent)
{
    PushConstantC1 c1{};
    c1.data = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, 0.f));
    rayMesher.draw(commandBuffer, pipelineLayout, graphicsPipeline, descriptorSets, currentFrame, swapChainExtent, c1);
}

void Ray::cleanup()
{
    rayMesher.cleanup();
}

Ray::~Ray()
{
}
