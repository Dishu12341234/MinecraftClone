#include "GameObject.h"

MeshUploader GameObject::meshUploader{};
std::atomic<uint32_t> GameObject::globalGOIDCounter{0};

GameObject::GameObject(VulkanContext vkContext)
{
    GOID = globalGOIDCounter.fetch_add(1, std::memory_order_relaxed);
    this->vkContext = vkContext;
}

void GameObject::loadMesh(GameMeshObject *mesh)
{
    this->mesh = mesh;
}

void GameObject::loadGeometry(std::string modelPath)
{
    this->mesh = new GameMeshObject(modelPath);
}

void GameObject::loadGeometry(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
{
    this->mesh = new GameMeshObject();
    this->mesh->vertices = vertices;
    this->mesh->indices = indices;
}

void GameObject::uploadVBOsAndIBOs()
{
    if(!mesh)
        return;
    
    meshUploader.upload(vkContext, *(this->mesh));
}

void GameObject::drawIndexed(VkCommandBuffer &commandBuffer, std::vector<VkDescriptorSet> &descriptorSets, u_GraphicsPipeline &graphicsPipeline, VkExtent2D &swapChainExtent, uint64_t &instanceCount, uint32_t &currentFrame)
{
    if(!mesh)
        return;

    VkBuffer vertexBuffers[] = {mesh->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipelineLayout, 0, 1, &(descriptorSets[currentFrame]), 0, nullptr);

    PushConstantC1 c1;
    c1.data = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
    c1.data[1][1] *= -1;

    vkCmdPushConstants(commandBuffer, graphicsPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantC1), &c1);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh->indices.size()), instanceCount, 0, 0, 0);
}

uint32_t GameObject::getID()
{
    return GOID;
}

void GameObject::cleanUpResources()
{
    this->mesh->cleanUpResources();
}

GameObject::~GameObject()
{
    delete mesh;
    mesh = nullptr;
}
