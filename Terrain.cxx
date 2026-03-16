#include "Terrain.h"
#include <iostream>
#include <chrono>
#define RENDER_DISTANCE 5

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
    for (auto &chunk : chunks)
    {
        if (chunk.chunkOffsetX == chunkX && chunk.chunkOffsetY == chunkY)
        {
            chunk.dirty = true;
            chunk.updateChunkMesh();
            break;
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
