#ifndef GAME_OBJECT_POOL
#define GAME_OBJECT_POOL
#include "GraphicsPipeline.h"
#include <vector>
#include <string.h>
#include "Voxel.h"

class Terrain;
class Chunk;

class GameObjectPool
{
private:
    VulkanContext vkContext;
    Terrain *terrain{nullptr};

    friend class HelloTriangleApplication;
    
    public:
    Chunk *getChunk(int chunkX, int chunkY);

    GameObjectPool();
    void init(VulkanContext context);
    void uploadVBOsAndIBOs();
    void drawIndexed(VkCommandBuffer &commandBuffer, std::vector<VkDescriptorSet> &descriptorSets, u_GraphicsPipeline &graphicsPipeline, VkExtent2D &swapChainExtent, uint64_t instanceCount, uint32_t &currentFrame);
    void cleanUpResources();

    Voxel *getVoxelGlobal(BlockCoordinates globalCoords);

    ~GameObjectPool();
};

#endif