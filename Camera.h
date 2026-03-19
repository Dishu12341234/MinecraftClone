#ifndef CAMERA_H
#define CAMERA_H

#include "GraphicsPipeline.h"
#include "Event.h"
#include "GameObjectPool.h"
#include "Ray.h"
#include "UI.h"
#include "Structs.h"

struct HitInfo
{
    BlockCoordinates blockCoords{};
    glm::vec3 hitNormal{};
    Voxel *hitVoxel{nullptr};
};

class Camera
{
private:
    VulkanContext vkContext;
    glm::vec3 cameraPos{1.f, 0.f, 66.f};
    float pitch;
    float yaw;
    float speed;
    GameObjectPool &gameObjectPool;
    glm::vec3 forwardCR;
    glm::vec3 worldUp{0, 0, 1};

    Ray cameraRay;
    Ray hitBoxR1;

    friend class Player;

public:
    Camera(const Camera &) = delete;
    Camera &operator=(const Camera &other)
    {
        // allocate new Vulkan buffers, copy data
        return *this;
    }

    Camera(Camera &&) = default; // allow moves
    Camera &operator=(Camera &&) = default;

    void getHitInfo(HitInfo &hitInfo);
    Camera(VulkanContext &vkContext, GameObjectPool &gop);
    void updateUBO(UniformBufferObject &UBO, VkExtent2D &swapChainExtent, Event &event);
    glm::vec3 gePositionInWorldCoords();
    void cleanup();

    void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
              std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent);

    void drawUI(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, UI &ui);
    void drawUIAt(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, UI &ui, uint32_t idx);

    PlayerState updateHitBox();

    ~Camera();
};

#endif