#include "Terrain.h"
#include "Chunk.h"
#include <atomic>
#include <fmt/base.h>
#include <fmt/core.h>
#include <mutex>
#include <ranges>
#include <thread>
#include <vector>

Terrain::Terrain(VulkanContext &vkContext, GameObjectPool &gop)
    : vkContext{vkContext}, gop{gop} {

  chunkBuilderThread = std::thread([&]() {
    while (running.load(std::memory_order_relaxed)) {
      chunkMutex.lock();
      std::vector<chunkSubmitCreateInfo> newChunksMaps = chunksSubmitVector;
      chunkMutex.unlock();

      for (auto &chunkSubmitInfo : newChunksMaps) {
        this->generateNewChunks(chunkSubmitInfo.x, chunkSubmitInfo.y);
      }

      chunkMutex.lock();
      chunksSubmitVector.clear();
      chunkMutex.unlock();
    }
  });
}

void Terrain::generateChunks() {

  int nChunks = 0;

  std::lock_guard<std::recursive_mutex> gLock(chunkMutex);
  for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; x++) {
    for (int y = -RENDER_DISTANCE; y <= RENDER_DISTANCE; y++) {

      Chunk *c = new Chunk(x, y, vkContext, gop);
      chunks.emplace(chunkKey(x, y), c);

      c->makeVisible();

      c->generateMesh();
      c->createBuffers();
      nChunks++;
    }
  }

  ready = true;
  fmt::println("total chunks created is: {}", nChunks);
}

void Terrain::updateChunkMesh(int cmx, int cmy) {
  std::lock_guard<std::recursive_mutex> lock(chunkMutex);
  auto it = chunks.find(chunkKey(cmx, cmy));
  if (it == chunks.end())
    return; // mutex released by lock_guard
  it->second->updateChunkMesh();
}
void Terrain::draw(VkCommandBuffer commandBuffer,
                   VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
                   std::vector<VkDescriptorSet> &descriptorSets,
                   uint32_t currentFrame, VkExtent2D &swapChainExtent) {

  chunkMutex.lock();
  PushConstantC1 c1;
  c1.data = glm::mat4(1.f);
  for (auto it = chunks.begin(); it != chunks.end(); it++) {
    it->second->draw(commandBuffer, pipelineLayout, graphicsPipeline,
                     descriptorSets, currentFrame, swapChainExtent, c1);
  }
  chunkMutex.unlock();
}

Chunk *Terrain::getChunkByKey(uint64_t key) {

  chunkMutex.lock();
  auto it = chunks.find(key);
  chunkMutex.unlock();

  if (it != chunks.end())
    return it->second;

  return nullptr;
}

void Terrain::cleanup() {
  chunkMutex.lock();
  for (auto it = chunks.begin(); it != chunks.end(); it++) {
    it->second->cleanup();
    delete it->second;
  }
  chunkMutex.unlock();

  this->running.store(false);

  chunkBuilderThread.join();
}

void Terrain::generateNewChunks(int chunkX, int chunkY) {

  chunkMutex.lock();
  auto it = chunks.find(chunkKey(chunkX, chunkY));
  chunkMutex.unlock();

  if (it != chunks.end())
    return;

  Chunk *c = new Chunk(chunkX, chunkY, vkContext, gop);

  vkDeviceWaitIdle(vkContext.device);
  c->makeVisible();
  c->generateMesh();
  c->createBuffers();

  chunkMutex.lock();
  nChunksLoaded++;
  chunks.emplace(chunkKey(chunkX, chunkY), c);
  chunkMutex.unlock();
}

void Terrain::sweepHandleTerrain(int playerChunkX, int playerChunkY) {
  std::lock_guard<std::recursive_mutex> lock(chunkMutex);

  std::vector<uint64_t> toRemove;
  for (auto &[key, chunk] : chunks) {
    int cx = (int)(key >> 32);
    int cy = (int)(key & 0xFFFFFFFF);
    if (std::abs(cx - playerChunkX) > RENDER_DISTANCE ||
        std::abs(cy - playerChunkY) > RENDER_DISTANCE) {
      toRemove.push_back(key);
    }
  }

  vkDeviceWaitIdle(vkContext.device);
  for (uint64_t key : toRemove) {
    chunks[key]->cleanup();
    delete chunks[key];
    chunks.erase(key);
    nChunksLoaded--;
  }
}
void Terrain::apppendNewChunkAsyncronously(int cx, int cy) {

  chunkMutex.lock();
  this->chunksSubmitVector.emplace_back(cx, cy);
  chunkMutex.unlock();
}

Terrain::~Terrain() {}
