#include "GameObjectPool.h"
#include "Terrain.h"

GameObjectPool::GameObjectPool()
{
}

void GameObjectPool::init(VulkanContext context)
{
    this->vkContext = context;
}

void GameObjectPool::cleanUpResources()
{
}

Voxel *GameObjectPool::getVoxelGlobal(BlockCoordinates globalCoords)
{
    if (globalCoords.z < 0 || globalCoords.z >= 256)
        return nullptr;

    int cox = globalCoords.x >> 4;
    int coy = globalCoords.y >> 4;

    Chunk *chunk = terrain->getChunkByKey(Terrain::chunkKey(cox, coy));
    if (!chunk)
        return nullptr;

    int localX = globalCoords.x & 15;
    int localY = globalCoords.y & 15;

    return &chunk->layers[globalCoords.z].voxels[localX][localY];
}

GameObjectPool::~GameObjectPool()
{
}
