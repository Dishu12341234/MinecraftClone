#include <cstdlib>
#include <cstring>
#include <fmt/base.h>
#include <iostream>

#include "Camera.h"
#include "Event.h"
#include "HelloTriangleApplication.hpp"
#include "Inventory.h"
#include "Player.h"
#include "Properties.h"
#include "Ray.h"
#include "Terrain.h"
#include "UI.h"
#include <memory>
#include <random>

void HelloTriangleApplication::initGameObjects() {
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
  font = std::make_unique<Fonts>(context, ft);
  srand(87844057);

  float aspect = float(swapChainExtent.width) / float(swapChainExtent.height);

  // UI Components

  inventory = std::make_unique<Inventory>(context, dimensions);
  inventory->attachPlayerState(&playerS1->playerState);
  inventory->populateSlots(itemPool);

  Crosshair = std::make_unique<UIComponents>(context);
  Heart = std::make_unique<UIComponents>(context);
  Hotbar = std::make_unique<UIComponents>(context);

  Crosshair->setTextureIDX(1);
  Crosshair->initUIComponent(glm::vec2(0, 0), glm::vec2(.05f, .05f));

  Heart->setTextureIDX(2);
  Heart->initUIComponent(glm::vec2(-1 * aspect + .04f, 1 - .04f),
                         glm::vec2(.08f, .08f));
  Heart->setInstanceCount(0);

  Hotbar->setTextureIDX(4);
  Hotbar->initUIComponent(glm::vec2(.0f, 1.f - .0625f), glm::vec2(1.5f, .125f));

  ui->attachComponent(inventory->getInventoryComponentPointer());
  ui->attachComponent(inventory->getFilterComponentPointer());
  ui->attachComponent(Crosshair.get());
  ui->attachComponent(Heart.get());
  ui->attachComponent(Hotbar.get());

  gameObjectPool.terrain = terrain.get();
  terrain->generateChunks();
}

KeyTracker keys;

void HelloTriangleApplication::updateUniformBuffer(uint32_t currentImage) {

  Heart->setInstanceCount(playerS1->getHealthPoints());

  HitInfo hitInfo{};
  playerS1->camera->getHitInfo(hitInfo);
  if (event->getMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) &&
      !playerS1->playerState.inInventory)
    if (hitInfo.hitVoxel) {
      std::cout << "Hit block type: " << hitInfo.hitVoxel->getBlockType()
                << std::endl;
      std::cout << "Hit block coordinates: (x,y,z) (" << hitInfo.blockCoords.x
                << ", " << hitInfo.blockCoords.y << ", "
                << hitInfo.blockCoords.z << ")" << std::endl;

      if (hitInfo.hitVoxel->getBlockType() == BEDROCK)
        goto _no;

      hitInfo.hitVoxel->setType(AIR);

      terrain->updateChunkMesh(hitInfo.blockCoords.x >> 4,
                               hitInfo.blockCoords.y >> 4);

      if ((hitInfo.blockCoords.x & 15) == 0)
        terrain->updateChunkMesh((hitInfo.blockCoords.x >> 4) - 1,
                                 hitInfo.blockCoords.y >> 4);
      if ((hitInfo.blockCoords.x & 15) == 15)
        terrain->updateChunkMesh((hitInfo.blockCoords.x >> 4) + 1,
                                 hitInfo.blockCoords.y >> 4);
      if ((hitInfo.blockCoords.y & 15) == 0)
        terrain->updateChunkMesh(hitInfo.blockCoords.x >> 4,
                                 (hitInfo.blockCoords.y >> 4) - 1);
      if ((hitInfo.blockCoords.y & 15) == 15)
        terrain->updateChunkMesh(hitInfo.blockCoords.x >> 4,
                                 (hitInfo.blockCoords.y >> 4) + 1);
    _no:;
    }

  if (event->getMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT) &&
      !playerS1->playerState.inInventory) {
    if (hitInfo.hitVoxel) {
      fmt::println("Face:{}", (int)hitInfo.faceDirection);
      int oy = (hitInfo.faceDirection == (int)Face::Right) -
               (hitInfo.faceDirection == (int)Face::Left);
      int ox = (hitInfo.faceDirection == (int)Face::Front) -
               (hitInfo.faceDirection == (int)Face::Back);
      int oz = (hitInfo.faceDirection == (int)Face::Top) -
               (hitInfo.faceDirection == (int)Face::Bottom);

      fmt::println("Ois ({},{},{})", ox, oy, oz);
      auto voxel = gameObjectPool.getVoxelGlobal({hitInfo.blockCoords.x + ox,
                                                  hitInfo.blockCoords.y + oy,
                                                  hitInfo.blockCoords.z + oz});

      auto currentItem = inventory->getCurrentItemInPrimaryHand();
      if (currentItem != nullptr && currentItem->itemType == ItemType::BLOCK) {
        auto type =
            (int)((BlockTypeProperties *)(currentItem->properties))->type;
        fmt::println("type: {}", type);
        voxel->setType((BlockType)type);
      }

      terrain->updateChunkMesh(hitInfo.blockCoords.x >> 4,
                               hitInfo.blockCoords.y >> 4);

      if ((hitInfo.blockCoords.x & 15) == 0)
        terrain->updateChunkMesh((hitInfo.blockCoords.x >> 4) - 1,
                                 hitInfo.blockCoords.y >> 4);
      if ((hitInfo.blockCoords.x & 15) == 15)
        terrain->updateChunkMesh((hitInfo.blockCoords.x >> 4) + 1,
                                 hitInfo.blockCoords.y >> 4);
      if ((hitInfo.blockCoords.y & 15) == 0)
        terrain->updateChunkMesh(hitInfo.blockCoords.x >> 4,
                                 (hitInfo.blockCoords.y >> 4) - 1);
      if ((hitInfo.blockCoords.y & 15) == 15)
        terrain->updateChunkMesh(hitInfo.blockCoords.x >> 4,
                                 (hitInfo.blockCoords.y >> 4) + 1);
    }
  }

  if (!playerS1->playerState.inInventory) {
    if (keys.justPressed(event.get(), GLFW_KEY_0)) {
      inventory->col = 9;
    }
    if (keys.justPressed(event.get(), GLFW_KEY_1)) {
      inventory->col = 0;
    }
    if (keys.justPressed(event.get(), GLFW_KEY_2)) {
      inventory->col = 1;
    }
    if (keys.justPressed(event.get(), GLFW_KEY_3)) {
      inventory->col = 2;
    }
    if (keys.justPressed(event.get(), GLFW_KEY_4)) {
      inventory->col = 3;
    }
    if (keys.justPressed(event.get(), GLFW_KEY_5)) {
      inventory->col = 4;
    }
    if (keys.justPressed(event.get(), GLFW_KEY_6)) {
      inventory->col = 5;
    }
    if (keys.justPressed(event.get(), GLFW_KEY_7)) {
      inventory->col = 6;
    }
    if (keys.justPressed(event.get(), GLFW_KEY_8)) {
      inventory->col = 7;
    }
    if (keys.justPressed(event.get(), GLFW_KEY_9)) {
      inventory->col = 8;
    }
  }

  if (keys.justPressed(event.get(), GLFW_KEY_ESCAPE)) {
    if (!playerS1->playerState.inInventory)
      glfwSetWindowShouldClose(_window, GLFW_TRUE);
    else
      playerS1->playerState.inInventory = false;
  }

  if (keys.justPressed(event.get(), GLFW_KEY_E)) {
    playerS1->playerState.inInventory = !playerS1->playerState.inInventory;
  }

  if (playerS1->playerState.inInventory) {
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  } else {
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  UniformBufferObject ubo{};

  playerS1->handlePlayerMovement(ubo, swapChainExtent, *event.get());
  inventory->inventoryUpdates(*event);

  int baseChunksX = int(floor(playerS1->selfTransform.position.x)) >> 4;
  int baseChunksY = int(floor(playerS1->selfTransform.position.y)) >> 4;

  for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; x++) {
    for (int y = -RENDER_DISTANCE; y < RENDER_DISTANCE; y++) {
      terrain->apppendNewChunkAsyncronously(baseChunksX + x, baseChunksY + y);
    }
  }

  terrain->sweepHandleTerrain(baseChunksX, baseChunksY);
  memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
  keys.update(event.get(),
              {GLFW_KEY_E, GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3,
               GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8,
               GLFW_KEY_9, GLFW_KEY_ESCAPE});
}
