#pragma once

#include <unordered_set>

#include "Mesh.h"
#include "PassInfo.hpp"
#include "Structs.h"
#include "Voxel.h"

#include <chrono>
#include <array>

#define TIMER_START(name)                                                      \
  auto name##_start = std::chrono::high_resolution_clock::now();

#define TIMER_END(name)                                                        \
  {                                                                            \
    auto name##_end = std::chrono::high_resolution_clock::now();               \
    auto name##_dur = std::chrono::duration_cast<std::chrono::milliseconds>(   \
                          name##_end - name##_start)                           \
                          .count();                                            \
    std::cout << #name << " took " << name##_dur << " ms\n";                   \
  }

struct Layer {
  int z;
  std::unordered_set<int> blocksTypes{0};
  Voxel voxels[16][16];
};

class GameObjectPool;

class Chunk {
private:
  VulkanContext &vkContext;

  std::array<Layer, 256> layers;

  int chunkOffset[2] = {0, 0}; // x,y

  Mesh chunkMesh;
  GameObjectPool &gop;



  friend class GameObjectPool;
  friend class Terrain;


public:
  Chunk(int cmx, int cmy, VulkanContext &vkContext, GameObjectPool &gop);

  void makeVisible();
  void generateMesh();
  void createBuffers();

  void updateChunkMesh();


  void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
            VkPipeline graphicsPipeline,
            std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame,
            VkExtent2D &swapChainExtent, PushConstantC1 &c1);

  void cleanup();
  ~Chunk();
};
