#pragma once
#include "PassInfo.hpp"
#include "Structs.h"
#include <memory>
#include "Voxel.h"
#include "Mesh.h"
#include "GameObjectPool.h"
#include <unordered_map>

#define TIMER_START(name) \
    auto name##_start = std::chrono::high_resolution_clock::now();

#define TIMER_END(name)                                                                                             \
    {                                                                                                               \
        auto name##_end = std::chrono::high_resolution_clock::now();                                                \
        auto name##_dur = std::chrono::duration_cast<std::chrono::milliseconds>(name##_end - name##_start).count(); \
        std::cout << #name << " took " << name##_dur << " ms\n";                                                    \
    }

class Voxel;

struct BlockCoordinatesHash
{
    std::size_t operator()(const BlockCoordinates &b) const noexcept
    {
        std::size_t h1 = std::hash<int>{}(b.x);
        std::size_t h2 = std::hash<int>{}(b.y);
        std::size_t h3 = std::hash<int>{}(b.z);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

class Chunk
{
private:
    VulkanContext &vkContext;
    std::vector<Voxel> voxels;
    int chunkOffsetX{0};
    int chunkOffsetY{0};

    Mesh chunkMesh;
    Mesh backMesh;

    GameObjectPool &gameObjectPool;

    friend class GameObjectPool;
    friend class Terrain;

    
    public:
    bool dirty = false;

    Chunk(VulkanContext &vkContext, GameObjectPool &gameObjectPool);
    Chunk(const Chunk &) = delete;
    Chunk &operator=(const Chunk &other)
    {
        // allocate new Vulkan buffers, copy data
        return *this;
    }

    Chunk(Chunk &&) = default; // allow moves
    Chunk &operator=(Chunk &&) = default;

    bool isFaceVisible(int x, int y, int z);
    void setOffset(int x, int y);
    void populateBlocks();

    void genMesh(bool useChunkMesh = false);
    void buildChunkMesh();
    void updateChunkMesh();

    void swapMesh();
    void draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent);

    void cleanup();

    [[nodiscard]] Voxel *getVoxelChunkLocal(BlockCoordinates);

    ~Chunk();
};
