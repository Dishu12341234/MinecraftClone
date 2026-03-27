#include "Terrain.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_set>

void Terrain::rebuildChunkLookup()
{
    chunkLookup.clear();
    chunkLookup.reserve(chunks.size());

    for (Chunk &c : chunks)
        chunkLookup[chunkKey(c.chunkOffsetX, c.chunkOffsetY)] = &c;
}

Terrain::Terrain(VulkanContext &vkContext, GameObjectPool &gameObjectPool)
    : vkContext{vkContext}, gameObjectPool{gameObjectPool}
{
}

void Terrain::generateChunks()
{
    TIMER_START(GENERATE_CHUNKS);

    chunks.reserve((RENDER_DISTANCE * 2) * (RENDER_DISTANCE * 2));

    for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; x++)
    {
        for (int y = -RENDER_DISTANCE; y < RENDER_DISTANCE; y++)
        {
            chunks.emplace_back(vkContext, gameObjectPool);
            chunks.back().setOffset(x, y);
            chunkIndices.emplace(chunkIndex(x, y));

            {
                std::lock_guard<std::mutex> lock(chunkBuilderMutex);
                newChunks.push_back(&chunks.back());
            }
        }
    }

    rebuildChunkLookup();

    // Worker thread
    chunkBuilder = std::thread([this]()
                               {
        while (true)
        {
            std::unique_lock<std::mutex> lock(chunkBuilderMutex);
            if (closed)
                return;

            cv.wait(lock, [this]()
            {
                return !newChunks.empty() || closed;
            });


            std::vector<Chunk*> localQueue;
            localQueue.swap(newChunks); // take work

            lock.unlock();

            for (Chunk* chunk : localQueue)
            {
                chunk->populateBlocks();
                chunk->populated = true;

                newChunks_r.push_back(chunk);

            }

            lock.lock();
        if (newChunks.empty())
        {
            populationDone = true;
        }
        lock.unlock();
        } });

    cv.notify_one();

    TIMER_END(GENERATE_CHUNKS);
}

void Terrain::makeChunksRenderable()
{
    std::vector<Chunk *> localQueue;

    {
        std::lock_guard<std::mutex> lock(chunkBuilderMutex);

        if (!populationDone)
            return;

        if (newChunks_r.empty() || !newChunks.empty())
            return;

        localQueue.swap(newChunks_r);
    }

    for (Chunk *chunk : localQueue)
    {
        if (chunk->populated)
        {
            chunk->buildChunkMesh();
        }
    }
}

void Terrain::updateChunkMesh(int chunkX, int chunkY)
{

    int idx = chunkIndex(chunkX, chunkY);

    if (chunkIndices.find(idx) == chunkIndices.end())
    {

        return;
    }
    auto it = chunkLookup.find(chunkKey(chunkX, chunkY));
    if (it == chunkLookup.end())
        return;
    Chunk &chunk = *it->second;
    chunk.dirty = true;

    chunk.updateChunkMesh();
}

std::unordered_set<int> appendedChunks;

void Terrain::generateNewChunks(int chunkX, int chunkY)
{
    int idx = chunkIndex(chunkX, chunkY);

    auto it = chunkLookup.find(chunkKey(chunkX, chunkY));

    if (it == chunkLookup.end())
    {
        std::cerr << "new chunk at (" << chunkX << "," << chunkY << ");\n";
        appendedChunks.emplace(idx);

        chunks.emplace_back(vkContext, gameObjectPool);
        Chunk *newChunk = &chunks.back();

        newChunk->setOffset(chunkX, chunkY);
        chunkIndices.emplace(chunkIndex(chunkX, chunkY));

        {
            std::lock_guard<std::mutex> lock(chunkBuilderMutex);
            populationDone = false;
            newChunks.push_back(newChunk);
        }

        rebuildChunkLookup();

        // 🔥 CRITICAL: wake worker thread
        cv.notify_one();
    }
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

void Terrain::draw(VkCommandBuffer cmdBuffer,
                   VkPipelineLayout pipelineLayout,
                   VkPipeline graphicsPipeline,
                   std::vector<VkDescriptorSet> &descriptorSets,
                   uint32_t currentFrame,
                   VkExtent2D &swapChainExtent)
{
    for (auto &chunk : chunks)
    {
        chunk.draw(cmdBuffer, pipelineLayout, graphicsPipeline,
                   descriptorSets, currentFrame, swapChainExtent);
    }
}

void Terrain::cleanup()
{
    {
        std::lock_guard<std::mutex> lock(chunkBuilderMutex);
        closed = true;
    }

    cv.notify_all();

    if (chunkBuilder.joinable())
        chunkBuilder.join();

    std::cout << "Cleaning Chunks" << std::endl;
    for (auto &c : chunks)
    {
        c.cleanup();
    }
}

Terrain::~Terrain()
{
}