#ifndef GAME_OBJECT_POOL
#define GAME_OBJECT_POOL

#include <vector>
#include <string.h>
#include "GameObject.h"

class GameObjectPool
{
private:
    std::vector<GameObject *> gameObjects;
    VulkanContext vkContext;

public:
    GameObjectPool();
    void init(VulkanContext context);
    GameObject* createNewGameObject(std::string modelPath);
    GameObject* createNewGameObject();
    void appendGameObject(GameObject *);
    void uploadVBOsAndIBOs();
    void drawIndexed(VkCommandBuffer &commandBuffer, std::vector<VkDescriptorSet> &descriptorSets, u_GraphicsPipeline &graphicsPipeline, VkExtent2D &swapChainExtent, uint64_t instanceCount, uint32_t &currentFrame);
    void cleanUpResources();
    ~GameObjectPool();
};

#endif