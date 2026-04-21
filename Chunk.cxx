#include "Chunk.h"
#include "GameObjectPool.h"
#include "GraphicsPipeline.h"

Chunk::Chunk(int cmx, int cmy, VulkanContext &vkContext, GameObjectPool &gop)
    : vkContext{vkContext}, chunkMesh{vkContext}, gop{gop} {

  chunkOffset[0] = cmx;
  chunkOffset[1] = cmy;

  const float baseX = cmx * 16;
  const float baseY = cmy * 16;
  for (int z = 0; z < 256; ++z) {
    auto &layer = layers[z];

    for (int x = 0; x < 16; ++x) {
      auto &column = layer.voxels[x];
      const float px = baseX + x;

      for (int y = 0; y < 16; ++y) {
        Voxel &v = column[y];
        if (z < 60)
          v.setType(GRASS);
        if (z == 0)
          v.setType(BEDROCK);
        v.setPosition(glm::vec3(px + 0.5f, baseY + y + 0.5f, z + 0.5f));
      }
    }
  }
}

void Chunk::generateMesh() {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  int c = 0;

  auto appendToBuffer = [&vertices, &indices, &c](glm::vec3 pos, glm::vec2 uv,
                                                  float brightness) {
    vertices.emplace_back(Vertex{pos, glm::vec3(1.f), uv, 1.f});
    indices.push_back(c);
    c++;
  };

  auto cullTopVoxels = [&appendToBuffer](Voxel &voxel, int &x, int &y, int &z,
                                         Layer *layers) {
    if (voxel.blockType == AIR)
      return;

    if (z < 255) {
      Voxel &topVoxel = layers[z + 1].voxels[x][y];
      if (topVoxel.blockType != AIR)
        return;
    }

    glm::vec3 &vPos = voxel.transform.position;

    float vz = vPos.z + 0.5f;
    glm::vec2 v0{vPos.x - 0.5f, vPos.y - 0.5f};
    glm::vec2 v1{vPos.x - 0.5f, vPos.y + 0.5f};
    glm::vec2 v2{vPos.x + 0.5f, vPos.y + 0.5f};
    glm::vec2 v3{vPos.x + 0.5f, vPos.y - 0.5f};

    appendToBuffer(glm::vec3{v0, vz}, glm::vec2{voxel.faceTexture[TOP], 0},
                   1.f);
    appendToBuffer(glm::vec3{v1, vz}, glm::vec2{voxel.faceTexture[TOP], 1},
                   1.f);
    appendToBuffer(glm::vec3{v2, vz}, glm::vec2{voxel.faceTexture[TOP], 2},
                   1.f);
    appendToBuffer(glm::vec3{v2, vz}, glm::vec2{voxel.faceTexture[TOP], 2},
                   1.f);
    appendToBuffer(glm::vec3{v3, vz}, glm::vec2{voxel.faceTexture[TOP], 3},
                   1.f);
    appendToBuffer(glm::vec3{v0, vz}, glm::vec2{voxel.faceTexture[TOP], 0},
                   1.f);
  };

  auto cullBottomVoxels = [&appendToBuffer](Voxel &voxel, int &x, int &y,
                                            int &z, Layer *layers) {
    if (voxel.blockType == AIR)
      return;

    if (z > 0) {
      Voxel &bottomVoxel = layers[z - 1].voxels[x][y];
      if (bottomVoxel.blockType != AIR)
        return;
    }

    glm::vec3 &vPos = voxel.transform.position;

    float vz = vPos.z - 0.5f;
    glm::vec2 v0{vPos.x - 0.5f, vPos.y - 0.5f};
    glm::vec2 v1{vPos.x - 0.5f, vPos.y + 0.5f};
    glm::vec2 v2{vPos.x + 0.5f, vPos.y + 0.5f};
    glm::vec2 v3{vPos.x + 0.5f, vPos.y - 0.5f};

    appendToBuffer(glm::vec3{v0, vz}, glm::vec2{voxel.faceTexture[BOTTOM], 0},
                   .75f);
    appendToBuffer(glm::vec3{v1, vz}, glm::vec2{voxel.faceTexture[BOTTOM], 1},
                   .75f);
    appendToBuffer(glm::vec3{v2, vz}, glm::vec2{voxel.faceTexture[BOTTOM], 2},
                   .75f);
    appendToBuffer(glm::vec3{v2, vz}, glm::vec2{voxel.faceTexture[BOTTOM], 2},
                   .75f);
    appendToBuffer(glm::vec3{v3, vz}, glm::vec2{voxel.faceTexture[BOTTOM], 3},
                   .75f);
    appendToBuffer(glm::vec3{v0, vz}, glm::vec2{voxel.faceTexture[BOTTOM], 0},
                   .75f);
  };

  auto cullLeftVoxels = [&appendToBuffer, this](Voxel &voxel, int &x, int &y,
                                                int &z, Layer *layers) {
    if (voxel.blockType == AIR)
      return;

    auto voxelLeft = gop.getVoxelGlobal(
        {chunkOffset[0] * 16 + x, chunkOffset[1] * 16 + y + 1, z});
    if (voxelLeft && voxelLeft->blockType != AIR)
      return;

    glm::vec3 &vPos = voxel.transform.position;

    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y + 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[LEFT], 0}, .8f);
    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y + 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[LEFT], 1}, .8f);
    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y + 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[LEFT], 2}, .8f);
    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y + 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[LEFT], 2}, .8f);
    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y + 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[LEFT], 3}, .8f);
    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y + 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[LEFT], 0}, .8f);
  };

  auto cullRightVoxels = [&appendToBuffer, this](Voxel &voxel, int &x, int &y,
                                                 int &z, Layer *layers) {
    if (voxel.blockType == AIR)
      return;

    auto voxelRight = gop.getVoxelGlobal(
        {chunkOffset[0] * 16 + x, chunkOffset[1] * 16 + y - 1, z});
    if (voxelRight && voxelRight->blockType != AIR)
      return;

    glm::vec3 &vPos = voxel.transform.position;

    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y - 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[RIGHT], 0}, .8f);
    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y - 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[RIGHT], 1}, .8f);
    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y - 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[RIGHT], 2}, .8f);
    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y - 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[RIGHT], 2}, .8f);
    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y - 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[RIGHT], 3}, .8f);
    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y - 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[RIGHT], 0}, .8f);
  };

  auto cullFrontVoxels = [&appendToBuffer, this](Voxel &voxel, int &x, int &y,
                                                 int &z, Layer *layers) {
    if (voxel.blockType == AIR)
      return;

    auto voxelFront = gop.getVoxelGlobal(
        {chunkOffset[0] * 16 + x - 1, chunkOffset[1] * 16 + y, z});
    if (voxelFront && voxelFront->blockType != AIR)
      return;

    glm::vec3 &vPos = voxel.transform.position;

    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y - 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[FRONT], 0}, .9f);
    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y + 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[FRONT], 1}, .9f);
    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y + 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[FRONT], 2}, .9f);
    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y + 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[FRONT], 2}, .9f);
    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y - 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[FRONT], 3}, .9f);
    appendToBuffer(glm::vec3{vPos.x - 0.5f, vPos.y - 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[FRONT], 0}, .9f);
  };

  auto cullBackVoxels = [&appendToBuffer, this](Voxel &voxel, int &x, int &y,
                                                int &z, Layer *layers) {
    if (voxel.blockType == AIR)
      return;

    auto voxelBack = gop.getVoxelGlobal(
        {chunkOffset[0] * 16 + x + 1, chunkOffset[1] * 16 + y, z});
    if (voxelBack && voxelBack->blockType != AIR)
      return;

    glm::vec3 &vPos = voxel.transform.position;

    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y - 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[BACK], 0}, .9f);
    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y + 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[BACK], 1}, .9f);
    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y + 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[BACK], 2}, .9f);
    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y + 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[BACK], 2}, .9f);
    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y - 0.5f, vPos.z + 0.5f},
                   glm::vec2{voxel.faceTexture[BACK], 3}, .9f);
    appendToBuffer(glm::vec3{vPos.x + 0.5f, vPos.y - 0.5f, vPos.z - 0.5f},
                   glm::vec2{voxel.faceTexture[BACK], 0}, .9f);
  };

  for (int z = 0; z < 256; ++z) {
    for (int x = 0; x < 16; ++x) {
      for (int y = 0; y < 16; ++y) {
        Voxel &voxel = layers[z].voxels[x][y];
        cullTopVoxels(voxel, x, y, z, layers);
        cullBottomVoxels(voxel, x, y, z, layers);
        cullLeftVoxels(voxel, x, y, z, layers);
        cullRightVoxels(voxel, x, y, z, layers);
        cullFrontVoxels(voxel, x, y, z, layers);
        cullBackVoxels(voxel, x, y, z, layers);
      }
    }
  }

  chunkMesh.loadVertices(vertices);
  chunkMesh.loadIndices(indices);
}

void Chunk::createBuffers() {
  chunkMesh.createVertexBuffer();
  chunkMesh.createIndexBuffer();
}

void Chunk::updateChunkMesh() {
  generateMesh();
  vkDeviceWaitIdle(vkContext.device);
  chunkMesh.updateVertexBuffer();
  chunkMesh.updateIndexBuffer();
}

void Chunk::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                 VkPipeline graphicsPipeline,
                 std::vector<VkDescriptorSet> &descriptorSets,
                 uint32_t currentFrame, VkExtent2D &swapChainExtent,
                 PushConstantC1 &c1) {
  chunkMesh.draw(commandBuffer, pipelineLayout, graphicsPipeline,
                 descriptorSets, currentFrame, swapChainExtent, c1);
}

void Chunk::cleanup() { chunkMesh.cleanup(); }

Chunk::~Chunk() {}
