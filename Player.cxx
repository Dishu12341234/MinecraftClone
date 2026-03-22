#include "Player.h"
#include "Camera.h"
#include <cmath>
#include <iostream>

bool Player::checkAABBOverlap(const AxisAlignedBoundingBox &a,
                              const AxisAlignedBoundingBox &b)
{
    return (a.max.x > b.min.x && a.min.x < b.max.x) &&
           (a.max.y > b.min.y && a.min.y < b.max.y) &&
           (a.max.z > b.min.z && a.min.z < b.max.z);
}

void Player::resolveAxis(float amount, int axis, float &vertVel)
{
    auto rebuildAABB = [&]()
    {
        aabb.max = camera->cameraPos + glm::vec3(0.2f, 0.2f, 0.2f); // head 0.2 above eye
        aabb.min = camera->cameraPos - glm::vec3(0.2f, 0.2f, 1.75f); // feet 1.7 below eye
    };

    if (axis == 0)
        camera->cameraPos.x += amount;
    else if (axis == 1)
        camera->cameraPos.y += amount;
    else
        camera->cameraPos.z += amount;

    rebuildAABB();

    int minX = (int)floor(aabb.min.x), maxX = (int)floor(aabb.max.x);
    int minY = (int)floor(aabb.min.y), maxY = (int)floor(aabb.max.y);
    int minZ = (int)floor(aabb.min.z), maxZ = (int)floor(aabb.max.z);

    for (int x = minX; x <= maxX; ++x)
        for (int y = minY; y <= maxY; ++y)
            for (int z = minZ; z <= maxZ; ++z)
            {
                Voxel *voxel = gameObjectPool.getVoxelGlobal({x, y, z});
                if (!voxel || voxel->getBlockType() == AIR)
                    continue;
                if (!checkAABBOverlap(aabb, voxel->aabb))
                    continue;

                switch (axis)
                {
                case 0:
                    camera->cameraPos.x = (amount > 0)
                                              ? voxel->aabb.min.x - 0.2f
                                              : voxel->aabb.max.x + 0.2f;
                    break;

                case 1:
                    camera->cameraPos.y = (amount > 0)
                                              ? voxel->aabb.min.y - 0.2f
                                              : voxel->aabb.max.y + 0.2f;
                    break;

                case 2:
                    if (amount < 0)
                    {
                        // Falling — feet land on top of voxel
                        camera->cameraPos.z = voxel->aabb.max.z + 1.75f;
                        vertVel = 0.0f;
                        playerState.onGround = true;
                    }
                    else
                    {
                        // Jumping — eye hits bottom of voxel, pull down just clear
                        camera->cameraPos.z = voxel->aabb.min.z - 0.2f;
                        vertVel = 0.0f;
                    }
                    break;
                }

                rebuildAABB();
            }
}

Player::Player(VulkanContext &vkContext, GameObjectPool &gop)
    : gameObjectPool{gop}
{
    this->camera = std::move(Camera(vkContext, gop));
}

Player::~Player() {}

void Player::drawUIIfPossible(VkCommandBuffer commandBuffer,
                              VkPipelineLayout pipelineLayout,
                              VkPipeline graphicsPipeline,
                              std::vector<VkDescriptorSet> &descriptorSets,
                              uint32_t currentFrame,
                              VkExtent2D &swapChainExtent,
                              UI &ui)
{
    if (playerState.inInventory)
        camera->drawUIAt(commandBuffer, pipelineLayout, graphicsPipeline,
                         descriptorSets, currentFrame, swapChainExtent, ui, 0);
    camera->drawUIAt(commandBuffer, pipelineLayout, graphicsPipeline,
                     descriptorSets, currentFrame, swapChainExtent, ui, 1);
}
void Player::handlePlayerMovement(UniformBufferObject &UBO,
                                  VkExtent2D &swapChainExtent,
                                  Event &event)
{
    constexpr float GRAVITY = 0.00025f; // tune
    constexpr float JUMP_FORCE = 0.10f; // tune
    constexpr float TERMINAL_VEL = -.30f;

    static float vertVel = 0.0f;

    camera->updateUBO(UBO, swapChainExtent, event);

    glm::vec3 delta(0.0f);

    if (event.getKeyPressed(GLFW_KEY_W))
        delta += camera->forwardFlat * camera->speed;
    if (event.getKeyPressed(GLFW_KEY_S))
        delta -= camera->forwardFlat * camera->speed;
    if (event.getKeyPressed(GLFW_KEY_A))
        delta += camera->right * camera->speed;
    if (event.getKeyPressed(GLFW_KEY_D))
        delta -= camera->right * camera->speed;

    if (playerState.onGround && event.getKeyPressed(GLFW_KEY_SPACE))
        vertVel = JUMP_FORCE;
    if (!playerState.onGround)
        vertVel -= GRAVITY * float(event.dt);
    if (vertVel < TERMINAL_VEL)
        vertVel = TERMINAL_VEL;

    delta.z += vertVel;
    if (std::abs(vertVel) < 0.00001f)
        vertVel = 0.0f;

    resolveAxis(delta.x, 0, vertVel);
    resolveAxis(delta.y, 1, vertVel);
    playerState.onGround = false;
    resolveAxis(delta.z, 2, vertVel);
}