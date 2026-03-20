#include "Player.h"
#include "Camera.h"
#include <ctime>
#include <chrono>
#include <iostream>

Player::Player(VulkanContext &vkContext, GameObjectPool &gop) : gameObjectPool{gop}
{
    this->camera = std::move(Camera(vkContext, gop));
}

void Player::drawUIIfPossible(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, UI &ui)
{
    if (playerState.inInventory)
        camera->drawUIAt(commandBuffer, pipelineLayout, graphicsPipeline, descriptorSets, currentFrame, swapChainExtent, ui, 0);
    camera->drawUIAt(commandBuffer, pipelineLayout, graphicsPipeline, descriptorSets, currentFrame, swapChainExtent, ui, 1);
}

void Player::handlePlayerMovement(UniformBufferObject &UBO, VkExtent2D &swapChainExtent, Event &event)
{
    constexpr float JUMP_TIMESCALE_MS = 200.f;
    constexpr float JUMP_END_THRESHOLD = 4.5f;
    constexpr float JUMP_SCALE = .5f;
    constexpr float GRAVITY = 1.61f;

    camera->updateUBO(UBO, swapChainExtent, event);

    aabb.max = camera->cameraPos + glm::vec3(.15f, .15f, 0);
    aabb.min = camera->cameraPos - glm::vec3(.15f, .15f, 2);

    int minX = floor(aabb.min.x);
    int maxX = floor(aabb.max.x);

    int minY = floor(aabb.min.y);
    int maxY = floor(aabb.max.y);

    int z = floor(aabb.min.z); // feet level

    for (int x = minX; x <= maxX; x++)
        for (int y = minY; y <= maxY; y++)
        {
            Voxel *voxel = gameObjectPool.getVoxelGlobal(BlockCoordinates{x, y, z});

            if (!voxel || voxel->getBlockType() == AIR)
                continue;

            std::cout << "Player AABB: "
                      << "min(" << aabb.min.x << ", " << aabb.min.y << ", " << aabb.min.z << "), "
                      << "max(" << aabb.max.x << ", " << aabb.max.y << ", " << aabb.max.z << ")\n";

            std::cout << "Voxel AABB: "
                      << "min(" << voxel->aabb.min.x << ", " << voxel->aabb.min.y << ", " << voxel->aabb.min.z << "), "
                      << "max(" << voxel->aabb.max.x << ", " << voxel->aabb.max.y << ", " << voxel->aabb.max.z << ")\n";

            // Axis checks
            bool xOverlap = (aabb.max.x > voxel->aabb.min.x && aabb.min.x < voxel->aabb.max.x);
            bool yOverlap = (aabb.max.y > voxel->aabb.min.y && aabb.min.y < voxel->aabb.max.y);
            bool zOverlap = (aabb.max.z > voxel->aabb.min.z && aabb.min.z < voxel->aabb.max.z);

            std::cout << "Overlap X: " << xOverlap
                      << " Y: " << yOverlap
                      << " Z: " << zOverlap << "\n";

            if (xOverlap && yOverlap && zOverlap)
            {
                std::cout << "GC\n";
            }
        }

    if (event.getKeyPressed(GLFW_KEY_LEFT_SHIFT))
        camera->cameraPos.z -= camera->speed;

    return;

    auto now = std::chrono::steady_clock::now();
    long long now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                           now.time_since_epoch())
                           .count();

    if (playerState.onGround && ((now_ms - jumpStartTime) > 250))
    {
        playerState.inJump = false;
    }

    if (playerState.onGround && !playerState.inJump && event.getKeyPressed(GLFW_KEY_SPACE))
    {
        playerState.inJump = true;
        jumpStartTime = now_ms;
    }

    if (playerState.inJump)
    {
        float te = (now_ms - jumpStartTime) / JUMP_TIMESCALE_MS;
        camera->cameraPos.z += JUMP_SCALE * jump((te)) * camera->speed;

        if (te > JUMP_END_THRESHOLD)
        {
            playerState.inJump = false;
        }
        std::cout << te << std::endl;
    }
    else if (!playerState.onGround)
    {
        camera->cameraPos.z -= camera->speed / GRAVITY;
    }
}

Player::~Player()
{
}
