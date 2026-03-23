#pragma once
#include "PassInfo.hpp"
#include "Structs.h"
#include "blocks.h"
#include "utils.h"
#include "Chunk.h"
#include <vector>
#include <unordered_map>
// #include <thread>

#define RENDER_DISTANCE 5

class GameObjectPool;

static auto chunkIndex = [](int x, int y) -> int
{
    int ix = x + RENDER_DISTANCE;
    int iy = y + RENDER_DISTANCE;
    return ix * (2 * RENDER_DISTANCE) + iy;
};

class Terrain
{
private:
    VulkanContext &vkContext;
    std::vector<Chunk> chunks;
    std::unordered_map<uint64_t, Chunk *> chunkLookup;
    GameObjectPool &gameObjectPool;

    friend class GameObjectPool;

public:
    Terrain(const Terrain &) = delete;
    Terrain &operator=(const Terrain &other)
    {
        // allocate new Vulkan buffers, copy data
        return *this;
    }

    static inline uint64_t chunkKey(int x, int y)
    {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | static_cast<uint64_t>(static_cast<uint32_t>(y));
    }

    void rebuildChunkLookup();

    Terrain(Terrain &&) = default; // allow moves
    Terrain &operator=(Terrain &&) = default;
    Terrain(VulkanContext &vkContext, GameObjectPool &gameObjectPool);

    void generateChunks();
    void draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent);

    void updateChunkMesh(int chunkX, int chunkY);

    void handelDirtyChunks();

    void cleanup();
    ~Terrain();
};
