#include "Terrain.h"
#include <iostream>
#include <chrono>


Terrain::Terrain(VulkanContext &vkContext, GameObjectPool &gameObjectPool) : vkContext{vkContext}, gameObjectPool{gameObjectPool}
{
}

void Terrain::generateChunks()
{
    TIMER_START(GENERATE_CHUNKS);

    // Pass 1 — populate all chunks into the pool first
    for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; x++)
        for (int y = -RENDER_DISTANCE; y < RENDER_DISTANCE; y++)
        {
            chunks.emplace_back(vkContext, gameObjectPool);
            chunks.back().setOffset(x, y);
            chunks.back().populateBlocks();
        }

    // Pass 2 — all neighbours now exist, cross-chunk culling works
    for (auto &chunk : chunks)
        chunk.buildChunkMesh();

    TIMER_END(GENERATE_CHUNKS);
}

void Terrain::draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent)
{
    for (auto &&chunk : chunks)
    {
        chunk.draw(cmdBuffer, pipelineLayout, graphicsPipeline, descriptorSets, currentFrame, swapChainExtent);
    }
}


void Terrain::updateChunkMesh(int chunkX, int chunkY)
{
    int idx = chunkIndex(chunkX, chunkY);
    if(idx < 0 || idx >= chunks.size())
    {
        std::cerr << "Invalid chunk coordinates: (" << chunkX << ", " << chunkY << ")" << std::endl;
        return;
    }
    chunks.at(idx).dirty = true;
    chunks.at(idx).updateChunkMesh();
}

void Terrain::handelDirtyChunks()
{
    for (auto &chunk : chunks)
    {
        if (chunk.dirty)
        {
            chunk.swapMesh();
            chunk.dirty = false;
        }
    }
}

void Terrain::cleanup()
{
    for (auto &&c : chunks)
    {
        c.cleanup();
    }
}

Terrain::~Terrain()
{
}
