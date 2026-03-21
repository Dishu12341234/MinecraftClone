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

    aabb.max = camera->cameraPos + glm::vec3(.15f, .15f, 0);
    aabb.min = camera->cameraPos - glm::vec3(.15f, .15f, 2);

    handleWallColliders();
    handleGroundCollision();

    if (event.getKeyPressed(GLFW_KEY_W) && !playerState.frontBlock)
        camera->cameraPos += camera->forwardFlat * camera->speed * camera->x_velocityMultiplier.t;

    if (event.getKeyPressed(GLFW_KEY_S))
        camera->cameraPos -= camera->forwardFlat * camera->speed * camera->x_velocityMultiplier.s;

    if (event.getKeyPressed(GLFW_KEY_A))
        camera->cameraPos -= -camera->right * camera->speed * camera->y_velocityMultiplier.t;
    if (event.getKeyPressed(GLFW_KEY_D))
        camera->cameraPos += -camera->right * camera->speed * camera->y_velocityMultiplier.s;

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
    camera->updateUBO(UBO, swapChainExtent, event);
}

void Player::handleGroundCollision()
{
    int minX = floor(aabb.min.x);
    int maxX = floor(aabb.max.x);

    int minY = floor(aabb.min.y);
    int maxY = floor(aabb.max.y);

    int z = floor(aabb.min.z); // feet level

    playerState.onGround = false;

    for (int x = minX; x <= maxX; x++)
        for (int y = minY; y <= maxY; y++)
        {
            Voxel *voxel = gameObjectPool.getVoxelGlobal(BlockCoordinates{x, y, z});

            if (!voxel || voxel->getBlockType() == AIR)
                continue;
            // Axis checks
            bool xOverlap = (aabb.max.x > voxel->aabb.min.x && aabb.min.x < voxel->aabb.max.x);
            bool yOverlap = (aabb.max.y > voxel->aabb.min.y && aabb.min.y < voxel->aabb.max.y);
            bool zOverlap = (aabb.max.z > voxel->aabb.min.z && aabb.min.z < voxel->aabb.max.z);

            if (xOverlap && yOverlap && zOverlap)
            {
                playerState.onGround = true;
            }
        }
}

void Player::handleWallColliders()
{
    int minX = floor(aabb.min.x);
    int maxX = floor(aabb.max.x);

    int minY = floor(aabb.min.y);
    int maxY = floor(aabb.max.y);

    int feet = floor(aabb.min.z);
    int head = floor(aabb.max.z);

    std::cout << "minX: " << minX << std::endl;
    std::cout << "maxX: " << maxX << std::endl;

    std::cout << "minY: " << minY << std::endl;
    std::cout << "maxY: " << maxY << std::endl;

    std::cout << "feet (minZ): " << feet << std::endl;
    std::cout << "head (maxZ): " << head << std::endl;

    playerState.frontBlock = false;
    playerState.backBlock = false;
    playerState.leftBlock = false;
    playerState.rightBlock = false;

    camera->x_velocityMultiplier.t = 1;
    camera->x_velocityMultiplier.s = 1;

    camera->y_velocityMultiplier.t = 1;
    camera->y_velocityMultiplier.s = 1;

    for (size_t z = feet + 1; z <= head; z++)
    {
        Voxel *frontVoxel = gameObjectPool.getVoxelGlobal({camera->cameraPos.x + 1, floor(camera->cameraPos.y), z});
        if (!frontVoxel || frontVoxel->getBlockType() == AIR)
            continue;
        bool xOverlap = (aabb.max.x > frontVoxel->aabb.min.x && aabb.min.x < frontVoxel->aabb.max.x);
        bool yOverlap = (aabb.max.y > frontVoxel->aabb.min.y && aabb.min.y < frontVoxel->aabb.max.y);
        bool zOverlap = (aabb.max.z > frontVoxel->aabb.min.z && aabb.min.z < frontVoxel->aabb.max.z);

        if (xOverlap && yOverlap && zOverlap)

        {
            camera->x_velocityMultiplier.t = 0;
        }
    }

    for (size_t z = feet + 1; z <= head; z++)
    {
        Voxel *backVoxel = gameObjectPool.getVoxelGlobal({camera->cameraPos.x - 1, floor(camera->cameraPos.y), z});
        if (!backVoxel || backVoxel->getBlockType() == AIR)
            continue;
        bool xOverlap = (aabb.max.x > backVoxel->aabb.min.x && aabb.min.x < backVoxel->aabb.max.x);
        bool yOverlap = (aabb.max.y > backVoxel->aabb.min.y && aabb.min.y < backVoxel->aabb.max.y);
        bool zOverlap = (aabb.max.z > backVoxel->aabb.min.z && aabb.min.z < backVoxel->aabb.max.z);

        if (xOverlap && yOverlap && zOverlap)

        {
            camera->x_velocityMultiplier.s = 0;
        }
    }

    for (size_t z = feet + 1; z <= head; z++)
    {
        Voxel *leftVoxel = gameObjectPool.getVoxelGlobal({floor(camera->cameraPos.x), camera->cameraPos.y + 1, z});
        if (!leftVoxel || leftVoxel->getBlockType() == AIR)
            continue;
        bool xOverlap = (aabb.max.x > leftVoxel->aabb.min.x && aabb.min.x < leftVoxel->aabb.max.x);
        bool yOverlap = (aabb.max.y > leftVoxel->aabb.min.y && aabb.min.y < leftVoxel->aabb.max.y);
        bool zOverlap = (aabb.max.z > leftVoxel->aabb.min.z && aabb.min.z < leftVoxel->aabb.max.z);

        if (xOverlap && yOverlap && zOverlap)

        {
            camera->y_velocityMultiplier.t = 0;
        }
    }

    for (size_t z = feet + 1; z <= head; z++)
    {
        Voxel *rightVoxel = gameObjectPool.getVoxelGlobal({floor(camera->cameraPos.x), floor(camera->cameraPos.y - 1), z});
        if (!rightVoxel || rightVoxel->getBlockType() == AIR)
            continue;
        bool xOverlap = (aabb.max.x > rightVoxel->aabb.min.x && aabb.min.x < rightVoxel->aabb.max.x);
        bool yOverlap = (aabb.max.y > rightVoxel->aabb.min.y && aabb.min.y < rightVoxel->aabb.max.y);
        bool zOverlap = (aabb.max.z > rightVoxel->aabb.min.z && aabb.min.z < rightVoxel->aabb.max.z);

        if (xOverlap && yOverlap && zOverlap)

        {
            camera->y_velocityMultiplier.s = 0;
        }
    }
}

Player::~Player()
{
}
