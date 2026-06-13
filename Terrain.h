#pragma once
#include "Chunk.h"
#include "GameObjectPool.h"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <set>
#include <thread>
#include <vector>
#define RENDER_DISTANCE 3

#include <unordered_map>

struct chunkSubmitCreateInfo {
  int x{};
  int y{};
};

class Terrain {
private:
  VulkanContext &vkContext;
  std::unordered_map<uint64_t, Chunk *> chunks;
  GameObjectPool &gop;

  std::atomic<bool> running{true};
  bool ready{false};

  std::vector<chunkSubmitCreateInfo> chunksSubmitVector;
  std::set<uint64_t> chunksLoaded;
  std::atomic<uint64_t> nChunksLoaded{0};
  std::thread chunkBuilderThread;
  std::recursive_mutex chunkMutex;

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

  void apppendNewChunkAsyncronously(int cx, int cy);

  void generateNewChunks(int chunkX, int chunkY);

  void sweepHandleTerrain(int, int);

  void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
            VkPipeline graphicsPipeline,
            std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame,
            VkExtent2D &swapChainExtent);

  void cleanup();
  ~Terrain();
};
// TODO: Handle multi-thread sync issues
