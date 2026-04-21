#include "Terrain.h"
#include <fmt/base.h>
#include <fmt/core.h>
#include <ranges>

Terrain::Terrain(VulkanContext &vkContext, GameObjectPool &gop)
    : vkContext{vkContext}, gop{gop} {}

void Terrain::generateChunks() {

    int nChunks = 0;

  for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; x++) {
    for (int y = -RENDER_DISTANCE; y <= RENDER_DISTANCE; y++) {

      Chunk *c = new Chunk(x, y, vkContext, gop);
      chunks.emplace(chunkKey(x, y), c);

      c->generateMesh();
      c->createBuffers();
      nChunks++;
    }
  }

  ready = true;
  fmt::println("total chunks created is: {}", nChunks);
}

void Terrain::updateChunkMesh(int cmx, int cmy) {
  auto it = chunks.find(chunkKey(cmx, cmy));
  if (it == chunks.end())
    return;

  auto chunk = it->second;

  chunk->updateChunkMesh();
}

void Terrain::draw(VkCommandBuffer commandBuffer,
                   VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
                   std::vector<VkDescriptorSet> &descriptorSets,
                   uint32_t currentFrame, VkExtent2D &swapChainExtent) {
  PushConstantC1 c1;
  c1.data = glm::mat4(1.f);
  for (auto it = chunks.begin(); it != chunks.end(); it++) {
    it->second->draw(commandBuffer, pipelineLayout, graphicsPipeline,
                     descriptorSets, currentFrame, swapChainExtent, c1);
  }
}

Chunk *Terrain::getChunkByKey(uint64_t key) {
  auto it = chunks.find(key);

  if (it != chunks.end())
    return it->second;

  return nullptr;
}

void Terrain::cleanup() {
  for (auto it = chunks.begin(); it != chunks.end(); it++) {
    it->second->cleanup();
    delete it->second;
  }
}

Terrain::~Terrain() {
}
