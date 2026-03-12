#pragma once
#include "PassInfo.hpp"
#include "Structs.h"
#include <memory>
#include "Voxel.h"
#include "Mesh.h"
#include <unordered_map>
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

public:
    Chunk(VulkanContext &vkContext);
    Chunk(const Chunk &) = delete;
    Chunk &operator=(const Chunk &other)
    {
        // allocate new Vulkan buffers, copy data
        return *this;
    }

    Chunk(Chunk &&) = default; // allow moves
    Chunk &operator=(Chunk &&) = default;

    void setOffset(int x, int y);
    void populateBlocks();

    void buildChunkMesh();
    void draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent);

    void cleanup();

    [[nodiscard]] Voxel *getVoxelChunkLocal(BlockCoordinates);

    ~Chunk();
};
