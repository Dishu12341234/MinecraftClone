#pragma once
#include "PassInfo.hpp"
#include "Structs.h"
#include "Voxel.h"
#include "Mesh.h"

#include <chrono>

#define TIMER_START(name) \
    auto name##_start = std::chrono::high_resolution_clock::now();

#define TIMER_END(name)                                                                                             \
    {                                                                                                               \
        auto name##_end = std::chrono::high_resolution_clock::now();                                                \
        auto name##_dur = std::chrono::duration_cast<std::chrono::milliseconds>(name##_end - name##_start).count(); \
        std::cout << #name << " took " << name##_dur << " ms\n";                                                    \
    }

struct Layer
{
    int z;
    Voxel voxels[16][16];
};

class GameObjectPool;

class Chunk
{
private:
    VulkanContext &vkContext;

    Layer layers[256];

    int chunkOffset[2] = {0, 0}; // x,y

    Mesh chunkMesh;
    GameObjectPool &gop;

        friend class GameObjectPool;

public:
    Chunk(int cmx, int cmy, VulkanContext &vkContext, GameObjectPool &gop);
    void generateMesh();

    void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, PushConstantC1 &c1);

    void cleanup();
    ~Chunk();
};
