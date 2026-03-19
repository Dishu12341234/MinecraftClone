#include "Player.h"
#include "Camera.h"

Player::Player(VulkanContext &vkContext, GameObjectPool &gop)
{
    this->camera = std::move(Camera(vkContext, gop));
}

void Player::drawUIIfPossible(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, UI &ui)
{
    if (playerState.inInventory)
        camera->drawUIAt(commandBuffer, pipelineLayout, graphicsPipeline, descriptorSets, currentFrame, swapChainExtent, ui, 0);
}

void Player::handlePlayerMovement(UniformBufferObject &UBO, VkExtent2D &swapChainExtent, Event &event)
{
    playerState.onGround = camera->updateHitBox().onGround;

    this->camera->updateUBO(UBO, swapChainExtent, event);

    if (playerState.onGround)
    {
        if (event.getKeyPressed(GLFW_KEY_SPACE))
            camera->cameraPos.z += camera->speed;
    }
    else
        camera->cameraPos.z -= (camera->speed / 1.61f);
}

Player::~Player()
{
}
