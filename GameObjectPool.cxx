#include "GameObjectPool.h"

GameObjectPool::GameObjectPool()
{
}

void GameObjectPool::init(VulkanContext context)
{
    this->vkContext = context;  
}

GameObject *GameObjectPool::createNewGameObject(std::string modelPath)
{
    GameObject* gameObject = new GameObject(vkContext);
    gameObject->loadGeometry(modelPath);
    return gameObject;
}

GameObject *GameObjectPool::createNewGameObject()
{
    GameObject* gameObject = new GameObject(vkContext);
    return gameObject;
}

void GameObjectPool::appendGameObject(GameObject *gameObject)
{
    this->gameObjects.push_back(gameObject);
}

void GameObjectPool::uploadVBOsAndIBOs()
{
    for (auto &&gameObject : gameObjects)
    {
        gameObject->uploadVBOsAndIBOs();
    }
}

void GameObjectPool::drawIndexed(VkCommandBuffer &commandBuffer, std::vector<VkDescriptorSet> &descriptorSets, u_GraphicsPipeline &graphicsPipeline, VkExtent2D &swapChainExtent, uint64_t instanceCount, uint32_t &currentFrame)
{
    for (auto &&gameObject : gameObjects)
    {
        gameObject->drawIndexed(commandBuffer, descriptorSets, graphicsPipeline, swapChainExtent, instanceCount, currentFrame);
    }
}

void GameObjectPool::cleanUpResources()
{
    for (auto &&gameObject : gameObjects)
    {
        gameObject->cleanUpResources();
        delete gameObject;
    }

    gameObjects.clear();
}

GameObjectPool::~GameObjectPool()
{
}
