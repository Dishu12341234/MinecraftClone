#include "GameObjectPool.h"
#include "Terrain.h"
#include "Chunk.h"

Chunk *GameObjectPool::getChunk(int chunkX, int chunkY)
{
    if (!terrain) return nullptr;
    auto it = terrain->chunkLookup.find(terrain->chunkKey(chunkX, chunkY));
    return it != terrain->chunkLookup.end() ? it->second : nullptr;
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
    if (!terrain) return nullptr;

    const int localZ = globalCoords.z;
    if (localZ < 0 || localZ >= 256) return nullptr;

    const int chunkX = globalCoords.x >> 4;
    const int chunkY = globalCoords.y >> 4;

    auto it = terrain->chunkLookup.find(terrain->chunkKey(chunkX, chunkY));
    if (it == terrain->chunkLookup.end()) return nullptr;

    const int localX = globalCoords.x & 15;
    const int localY = globalCoords.y & 15;
    auto chunk = it->second;
    if(!chunk || !chunk->populated)
        return nullptr;
    return &chunk->voxels[localX * 16 + localY + localZ * 256];
}

GameObjectPool::~GameObjectPool()
{
}
