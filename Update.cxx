#include <iostream>
#include <cstdlib>
#include <cstring>

#include "HelloTriangleApplication.hpp"
#include "Camera.h"
#include "Chunk.h"
#include "Event.h"
#include <memory>
#include "Player.h"
#include "Ray.h"
#include "Terrain.h"
#include "UI.h"
#include <random>

void HelloTriangleApplication::initGameObjects()
{
    static VulkanContext context{};
    context.device = device;
    context.physicalDevice = physicalDevice;
    context.graphicsQueue = graphicsQueue;
    context.instance = instance;
    context.presentQueue = presentQueue;
    context.commandPool = commandPool;

    playerS1 = std::make_unique<Player>(context, gameObjectPool);

    terrain = std::make_unique<Terrain>(context, gameObjectPool);
    ui = std::move(UI(context));
    gameObjectPool.uploadVBOsAndIBOs();

    srand(87844057);

    float aspect = float(swapChainExtent.width) / float(swapChainExtent.height);

    // UI Components
    
    Inventory = std::make_unique<UIComponents>(context);
    Crosshair = std::make_unique<UIComponents>(context);
    Heart = std::make_unique<UIComponents>(context);


    Inventory->initUIComponent(glm::vec2(0.f), glm::vec2(2.5f, 1.5f));

    
    Crosshair->setTextureIDX(1);
    Crosshair->initUIComponent(glm::vec2(0, 0), glm::vec2(.05f, .05f));

    
    Heart->setTextureIDX(2);
    Heart->initUIComponent(glm::vec2(-1 * aspect + .04f, 1 - .04f), glm::vec2(.08f, .08f));
    Heart->setInstanceCount(0);

    ui->attachComponent(Inventory.get());
    ui->attachComponent(Crosshair.get());
    ui->attachComponent(Heart.get());
    
    gameObjectPool.terrain = terrain.get();
    terrain->generateChunks();
}

KeyTracker keys;

void HelloTriangleApplication::updateUniformBuffer(uint32_t currentImage)
{
    
    std::cout << "Coordinates: " << "(x,y,z) (x" << playerS1->camera->gePositionInWorldCoords().x << ", " << playerS1->camera->gePositionInWorldCoords().y << ", " << playerS1->camera->gePositionInWorldCoords().z << ")" << std::endl;
    std::cout << "CMs: " << "(x,y) (x" << (int(playerS1->camera->gePositionInWorldCoords().x) / 16) << ", " << (int(playerS1->camera->gePositionInWorldCoords().y) / 16) << ")" << std::endl;
    
    Heart->setInstanceCount(playerS1->getHealthPoints());
    HitInfo hitInfo{};
    playerS1->camera->getHitInfo(hitInfo);

    if (event->getMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        if (hitInfo.hitVoxel)
        {
            std::cout << "Hit block type: " << hitInfo.hitVoxel->getBlockType() << std::endl;
            std::cout << "Hit block coordinates: (x,y,z) (" << hitInfo.blockCoords.x << ", " << hitInfo.blockCoords.y << ", " << hitInfo.blockCoords.z << ")" << std::endl;

            if (hitInfo.hitVoxel->getBlockType() == BEDROCK)
                goto _no;

            hitInfo.hitVoxel->setType(AIR);

            terrain->updateChunkMesh(hitInfo.blockCoords.x >> 4, hitInfo.blockCoords.y >> 4);

            if ((hitInfo.blockCoords.x & 15) == 0)
                terrain->updateChunkMesh((hitInfo.blockCoords.x >> 4) - 1, hitInfo.blockCoords.y >> 4);
            if ((hitInfo.blockCoords.x & 15) == 15)
                terrain->updateChunkMesh((hitInfo.blockCoords.x >> 4) + 1, hitInfo.blockCoords.y >> 4);
            if ((hitInfo.blockCoords.y & 15) == 0)
                terrain->updateChunkMesh(hitInfo.blockCoords.x >> 4, (hitInfo.blockCoords.y >> 4) - 1);
            if ((hitInfo.blockCoords.y & 15) == 15)
                terrain->updateChunkMesh(hitInfo.blockCoords.x >> 4, (hitInfo.blockCoords.y >> 4) + 1);
        _no:;
        }

    if (keys.justPressed(event.get(), GLFW_KEY_ESCAPE))
    {
        if (!playerS1->playerState.inInventory)
            glfwSetWindowShouldClose(_window, GLFW_TRUE);
        else
            playerS1->playerState.inInventory = false;
    }

    if (keys.justPressed(event.get(), GLFW_KEY_E))
    {
        playerS1->playerState.inInventory = !playerS1->playerState.inInventory;
    }

    UniformBufferObject ubo{};

    playerS1->handlePlayerMovement(ubo, swapChainExtent, *event.get());

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    keys.update(event.get(), {GLFW_KEY_E, GLFW_KEY_ESCAPE});
}