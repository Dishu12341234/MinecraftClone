#include "GameObjectPool.h"
#include "Terrain.h"
#include "Chunk.h"

Chunk *GameObjectPool::getChunk(int chunkX, int chunkY)
{
    if(!terrain)
        return nullptr;

    for (auto &chunk : terrain->chunks)
    {
        if (chunk.chunkOffsetX == chunkX && chunk.chunkOffsetY == chunkY)
            return &chunk;
    }

    return nullptr;
}   

GameObjectPool::GameObjectPool()
{
}

void GameObjectPool::init(VulkanContext context)
{
    this->vkContext = context;
}

void GameObjectPool::uploadVBOsAndIBOs()
{
}

void GameObjectPool::drawIndexed(VkCommandBuffer &commandBuffer, std::vector<VkDescriptorSet> &descriptorSets, u_GraphicsPipeline &graphicsPipeline, VkExtent2D &swapChainExtent, uint64_t instanceCount, uint32_t &currentFrame)
{
}

void GameObjectPool::cleanUpResources()
{
}

Voxel *GameObjectPool::getVoxelGlobal(BlockCoordinates globalCoords)
{
    if (!terrain)
        return nullptr;

    int chunkX = globalCoords.x >> 4;
    int chunkY = globalCoords.y >> 4;
    int localX = globalCoords.x & 15;
    int localY = globalCoords.y & 15;
    int localZ = globalCoords.z;

    // Bounds check before touching any chunk
    if (localZ < 0 || localZ >= 256)
        return nullptr;

    for (auto &chunk : terrain->chunks)
    {
        if (chunk.chunkOffsetX == chunkX && chunk.chunkOffsetY == chunkY)
        {
            // Access voxels directly — never call getVoxelChunkLocal here
            int idx = localX + localY * 16 + localZ * 256;
            return &chunk.voxels[idx];
        }
    }

    return nullptr;
}

GameObjectPool::~GameObjectPool()
{
}
