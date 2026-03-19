#pragma once
#include <optional>
#include "Camera.h"

class Camera;

class Player
{
private:
    std::optional<Camera> camera;
    friend class HelloTriangleApplication;

    PlayerState playerState;
public:
    Player(VulkanContext &vkContext, GameObjectPool &gop);
    void drawUIIfPossible(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                    VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets,
                    uint32_t currentFrame, VkExtent2D &swapChainExtent, UI &ui);

    void handlePlayerMovement(UniformBufferObject &UBO,
                       VkExtent2D &swapChainExtent,
                       Event &event);
    ~Player();
};
