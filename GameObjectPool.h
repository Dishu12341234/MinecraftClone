#ifndef GAME_OBJECT_POOL
#define GAME_OBJECT_POOL
#include <vector>
#include <string.h>
#include <memory>

#include "GraphicsPipeline.h"
#include "Voxel.h"

class Terrain;

class GameObjectPool
{
private:
    VulkanContext vkContext;

    friend class HelloTriangleApplication;
    Terrain *terrain;

public:
    GameObjectPool();
    void init(VulkanContext context);
    void cleanUpResources();

    Voxel *getVoxelGlobal(BlockCoordinates globalCoords);

    ~GameObjectPool();
};

#endif