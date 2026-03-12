#ifndef GAME_OBJECT
#define GAME_OBJECT
#include "GameMeshObject.h"
#include "MeshUploader.h"

struct d_Pos3D
{
    int x, y, z;
};

struct d_Rect3D // dimessions_rect
{
    int x, y, z;
    int w, h, b;
};

class GameObject
{
private:
    static std::atomic<uint32_t> globalGOIDCounter;
    uint32_t GOID = UINT32_MAX;
    GameMeshObject *mesh = nullptr;
    d_Pos3D position;
    VulkanContext vkContext;
    static MeshUploader meshUploader;

public:
    GameObject(VulkanContext vkContext);
    void loadMesh(GameMeshObject *mesh);
    void loadGeometry(std::string);
    void loadGeometry(std::vector<Vertex> vertices, std::vector<uint32_t> indices); 
    void uploadVBOsAndIBOs();
    void drawIndexed(VkCommandBuffer &commandBuffer, std::vector<VkDescriptorSet> &descriptorSets, u_GraphicsPipeline &graphicsPipeline, VkExtent2D &swapChainExtent, uint64_t &instanceCount, uint32_t &currentFrame);
    uint32_t getID();
    void cleanUpResources();
    ~GameObject();
};

#endif