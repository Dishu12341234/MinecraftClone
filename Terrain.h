#pragma once
#include "Chunk.h"
#include "GameObjectPool.h"
#define RENDER_DISTANCE 3

#include <unordered_map>

class Terrain {
private:
  VulkanContext &vkContext;
  std::unordered_map<uint64_t, Chunk *> chunks;
  GameObjectPool &gop;
  bool ready{false};
  friend class HelloTriangleApplication;

public:
  Terrain(VulkanContext &vkContext, GameObjectPool &gop);
  static inline uint64_t chunkKey(int x, int y) {
    return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) |
           static_cast<uint64_t>(static_cast<uint32_t>(y));
  }
  Chunk *getChunkByKey(uint64_t key);

  void generateChunks();
  void updateChunkMesh(int cmx, int cmy);

  void generateNewChunks(int chunkX, int chunkY);

  void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
            VkPipeline graphicsPipeline,
            std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame,
            VkExtent2D &swapChainExtent);

  void cleanup();
  ~Terrain();
};
