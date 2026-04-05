#include "Terrain.h"
#include <iostream>

Terrain::Terrain(VulkanContext &vkContext, GameObjectPool &gop) : vkContext{vkContext}, gop{gop}
{
}

void Terrain::generateChunks()
{
    Chunk *c = new Chunk(0, 0, vkContext, gop);
    chunks.emplace(chunkKey(0, 0), c);

    c->generateMesh();

    ready = true;
}

void Terrain::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent)
{
    PushConstantC1 c1;
    c1.data = glm::mat4(1.f);
    for (auto it = chunks.begin(); it != chunks.end(); it++)
    {
        it->second->draw(commandBuffer, pipelineLayout, graphicsPipeline, descriptorSets, currentFrame, swapChainExtent, c1);
    }
}

Chunk *Terrain::getChunkByKey(uint64_t key)
{
    auto it = chunks.find(key);

    if (it != chunks.end())
        return it->second;

    return nullptr;
}

void Terrain::cleanup()
{
    for (auto it = chunks.begin(); it != chunks.end(); it++)
    {
        it->second->cleanup();
        delete it->second;
    }
}

Terrain::~Terrain()
{
}
