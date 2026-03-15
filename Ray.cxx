#include "Ray.h"
#include "GameObjectPool.h"
#include <vector>
#include "GraphicsPipeline.h"
#include <glm/gtx/quaternion.hpp>

Ray::Ray(VulkanContext &vkContext, GameObjectPool &gameObjectPool) : vkContext{vkContext}, gameObjectPool{gameObjectPool}, rayMesher{vkContext}
{
    std::vector<RayPoints> vertices = {
        {{0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}},
        {{1.f, 0.f, 0.f}, {0.2f, 1.f, 0.f}},
    };

    std::vector<uint32_t> indices = {0, 1};
    rayMesher.loadRayPoints(vertices);
    rayMesher.loadIndices(indices);

    rayMesher.createVertexBuffer();
    rayMesher.createIndexBuffer();
}

void Ray::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent)
{
    PushConstantC1 c1{};

    glm::quat q = glm::quat(transform.rotation);
    c1.data = glm::translate(glm::mat4(1.0f), transform.position) * glm::toMat4(q);

    rayMesher.draw(commandBuffer, pipelineLayout, graphicsPipeline, descriptorSets, currentFrame, swapChainExtent, c1);
}

void Ray::cleanup()
{
    rayMesher.cleanup();
}

Ray::~Ray()
{
}
