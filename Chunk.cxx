#include "Chunk.h"
#include "GameObjectPool.h"
#include "GraphicsPipeline.h"
#include <fmt/base.h>
#include <fmt/core.h>

#include <iostream>

Chunk::Chunk(int cmx, int cmy, VulkanContext &vkContext, GameObjectPool &gop)
    : vkContext{vkContext}, chunkMesh{vkContext}, gop{gop} {

  chunkOffset[0] = cmx;
  chunkOffset[1] = cmy;

  const float baseX = cmx * 16;
  const float baseY = cmy * 16;
  for (int z = 0; z < 256; ++z) {
    auto &layer = layers[z];
    layer.z = z;

    for (int x = 0; x < 16; ++x) {
      auto &column = layer.voxels[x];
      const float px = baseX + x;

      for (int y = 0; y < 16; ++y) {
        Voxel &v = column[y];
        if (z < 60)
          v.setType(BlockType(z % 6 + 1));
        if (z == 0)
          v.setType(BEDROCK);
        v.setPosition(glm::vec3(px + 0.5f, baseY + y + 0.5f, z + 0.5f));
      }
    }
  }
}

void Chunk::makeVisible() {
  for (int z = 0; z < 256; z++) {
    Layer &layer = layers[z];
    for (int x = 0; x < 16; x++) {
      for (int y = 0; y < 16; y++) {
        Voxel &v = layer.voxels[x][y];

        if (v.blockType == AIR) {
          v.visible = false;
          continue;
        }

        if (z == 0 || z == 255) [[unlikely]] {
          v.visible = true;
          continue;
        }

        Voxel &top_v = layers[z + 1].voxels[x][y];
        Voxel &bottom_v = layers[z - 1].voxels[x][y];

        int wx = chunkOffset[0] * 16 + x;
        int wy = chunkOffset[1] * 16 + y;

        Voxel *left_v = gop.getVoxelGlobal({wx, wy - 1, z});
        Voxel *right_v = gop.getVoxelGlobal({wx, wy + 1, z});
        Voxel *front_v = gop.getVoxelGlobal({wx + 1, wy, z});
        Voxel *back_v = gop.getVoxelGlobal({wx - 1, wy, z});

        bool neighborMissing = !left_v || !right_v || !front_v || !back_v;
        bool nextToAir = top_v.blockType == AIR || bottom_v.blockType == AIR ||
                         neighborMissing || left_v->blockType == AIR ||
                         right_v->blockType == AIR ||
                         front_v->blockType == AIR || back_v->blockType == AIR;

        v.visible = nextToAir;
      }
    }
  }
}
void Chunk::generateMesh() {
  TIMER_START(mesh);
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  int ind = 0;

  // World-space origin of this chunk
  const float ox = chunkOffset[0] * 16.0f;
  const float oy = chunkOffset[1] * 16.0f;

  auto emmitQuad = [&](glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
                       BlockType type, float brightness) {
    glm::vec2 uv0{(int)type, 0}, uv1{(int)type, 1};
    glm::vec2 uv2{(int)type, 2}, uv3{(int)type, 3};
    vertices.push_back({p0, {1, 1, 1}, uv0, brightness});
    vertices.push_back({p1, {1, 1, 1}, uv1, brightness});
    vertices.push_back({p2, {1, 1, 1}, uv2, brightness});
    vertices.push_back({p3, {1, 1, 1}, uv3, brightness});
    indices.push_back(ind * 4 + 0);
    indices.push_back(ind * 4 + 1);
    indices.push_back(ind * 4 + 2);
    indices.push_back(ind * 4 + 2);
    indices.push_back(ind * 4 + 3);
    indices.push_back(ind * 4 + 0);
    ind++;
  };

  auto greedify_topbottom = [&](BlockType type, Layer &layer) {
    bool topMask[16][16]{};
    bool botMask[16][16]{};

    for (int x = 0; x < 16; x++) {
      for (int y = 0; y < 16; y++) {
        Voxel &v = layer.voxels[x][y];
        if (v.blockType != type || !v.visible)
          continue;
        bool topExposed = (layer.z == 255) ||
                          (layers[layer.z + 1].voxels[x][y].blockType == AIR);
        bool botExposed = (layer.z == 0) ||
                          (layers[layer.z - 1].voxels[x][y].blockType == AIR);
        topMask[x][y] = topExposed;
        botMask[x][y] = botExposed;
      }
    }

    bool topUsed[16][16]{};
    for (int x = 0; x < 16; x++) {
      for (int y = 0; y < 16; y++) {
        if (!topMask[x][y] || topUsed[x][y])
          continue;
        int tex = layer.voxels[x][y].faceTexture[TOP];
        int h = 1;
        while (y + h < 16 && topMask[x][y + h] && !topUsed[x][y + h] &&
               layer.voxels[x][y + h].faceTexture[TOP] == tex)
          h++;
        int w = 1;
        while (x + w < 16) {
          bool ok = true;
          for (int dy = 0; dy < h && ok; dy++)
            if (!topMask[x + w][y + dy] || topUsed[x + w][y + dy] ||
                layer.voxels[x + w][y + dy].faceTexture[TOP] != tex)
              ok = false;
          if (!ok)
            break;
          w++;
        }
        for (int dx = 0; dx < w; dx++)
          for (int dy = 0; dy < h; dy++)
            topUsed[x + dx][y + dy] = true;

        emmitQuad({ox + x, oy + y, (float)layer.z + 1},
                  {ox + x + w, oy + y, (float)layer.z + 1},
                  {ox + x + w, oy + y + h, (float)layer.z + 1},
                  {ox + x, oy + y + h, (float)layer.z + 1}, (BlockType)tex,
                  1.0f);
      }
    }

    bool botUsed[16][16]{};
    for (int x = 0; x < 16; x++) {
      for (int y = 0; y < 16; y++) {
        if (!botMask[x][y] || botUsed[x][y])
          continue;
        int tex = layer.voxels[x][y].faceTexture[BOTTOM];
        int h = 1;
        while (y + h < 16 && botMask[x][y + h] && !botUsed[x][y + h] &&
               layer.voxels[x][y + h].faceTexture[BOTTOM] == tex)
          h++;
        int w = 1;
        while (x + w < 16) {
          bool ok = true;
          for (int dy = 0; dy < h && ok; dy++)
            if (!botMask[x + w][y + dy] || botUsed[x + w][y + dy] ||
                layer.voxels[x + w][y + dy].faceTexture[BOTTOM] != tex)
              ok = false;
          if (!ok)
            break;
          w++;
        }
        for (int dx = 0; dx < w; dx++)
          for (int dy = 0; dy < h; dy++)
            botUsed[x + dx][y + dy] = true;

        emmitQuad({ox + x, oy + y, (float)layer.z},
                  {ox + x, oy + y + h, (float)layer.z},
                  {ox + x + w, oy + y + h, (float)layer.z},
                  {ox + x + w, oy + y, (float)layer.z}, (BlockType)tex, 0.6f);
      }
    }
  };

  auto greedify_xfaces = [&](BlockType type) {
    for (int x = 0; x < 16; x++) {
      int wx = chunkOffset[0] * 16 + x;

      bool frontMask[16][256]{}, backMask[16][256]{};
      bool frontUsed[16][256]{}, backUsed[16][256]{};

      for (int z = 0; z < 256; z++) {
        for (int y = 0; y < 16; y++) {
          Voxel &v = layers[z].voxels[x][y];
          if (v.blockType != type || !v.visible)
            continue;
          int wy = chunkOffset[1] * 16 + y;
          auto frontNeighbor = [&]() -> BlockType {
            if (x < 15)
              return (BlockType)layers[z].voxels[x + 1][y].blockType;
            Voxel *fv = gop.getVoxelGlobal({wx + 1, wy, z});
            return fv ? (BlockType)fv->blockType : AIR;
          };
          if (frontNeighbor() == AIR)
            frontMask[y][z] = true;
          auto backNeighbor = [&]() -> BlockType {
            if (x > 0)
              return (BlockType)layers[z].voxels[x - 1][y].blockType;
            Voxel *bv = gop.getVoxelGlobal({wx - 1, wy, z});
            return bv ? (BlockType)bv->blockType : AIR;
          };
          if (backNeighbor() == AIR)
            backMask[y][z] = true;
        }
      }

      for (int y = 0; y < 16; y++) {
        for (int z = 0; z < 256; z++) {
          if (!frontMask[y][z] || frontUsed[y][z])
            continue;
          int tex = layers[z].voxels[x][y].faceTexture[FRONT];
          int dz = 1;
          while (z + dz < 256 && frontMask[y][z + dz] &&
                 !frontUsed[y][z + dz] &&
                 layers[z + dz].voxels[x][y].blockType == type &&
                 layers[z + dz].voxels[x][y].visible &&
                 layers[z + dz].voxels[x][y].faceTexture[FRONT] == tex)
            dz++;
          int dy = 1;
          while (y + dy < 16) {
            bool ok = true;
            for (int k = 0; k < dz && ok; k++)
              if (!frontMask[y + dy][z + k] || frontUsed[y + dy][z + k] ||
                  layers[z + k].voxels[x][y + dy].blockType != type ||
                  !layers[z + k].voxels[x][y + dy].visible ||
                  layers[z + k].voxels[x][y + dy].faceTexture[FRONT] != tex)
                ok = false;
            if (!ok)
              break;
            dy++;
          }
          for (int a = 0; a < dy; a++)
            for (int b = 0; b < dz; b++)
              frontUsed[y + a][z + b] = true;

          emmitQuad({ox + x + 1, oy + y, (float)z},
                    {ox + x + 1, oy + y + dy, (float)z},
                    {ox + x + 1, oy + y + dy, (float)(z + dz)},
                    {ox + x + 1, oy + y, (float)(z + dz)}, (BlockType)tex,
                    0.8f);
        }
      }

      for (int y = 0; y < 16; y++) {
        for (int z = 0; z < 256; z++) {
          if (!backMask[y][z] || backUsed[y][z])
            continue;
          int tex = layers[z].voxels[x][y].faceTexture[BACK];
          int dz = 1;
          while (z + dz < 256 && backMask[y][z + dz] && !backUsed[y][z + dz] &&
                 layers[z + dz].voxels[x][y].blockType == type &&
                 layers[z + dz].voxels[x][y].visible &&
                 layers[z + dz].voxels[x][y].faceTexture[BACK] == tex)
            dz++;
          int dy = 1;
          while (y + dy < 16) {
            bool ok = true;
            for (int k = 0; k < dz && ok; k++)
              if (!backMask[y + dy][z + k] || backUsed[y + dy][z + k] ||
                  layers[z + k].voxels[x][y + dy].blockType != type ||
                  !layers[z + k].voxels[x][y + dy].visible ||
                  layers[z + k].voxels[x][y + dy].faceTexture[BACK] != tex)
                ok = false;
            if (!ok)
              break;
            dy++;
          }
          for (int a = 0; a < dy; a++)
            for (int b = 0; b < dz; b++)
              backUsed[y + a][z + b] = true;

          emmitQuad({ox + x, oy + y, (float)z},
                    {ox + x, oy + y, (float)(z + dz)},
                    {ox + x, oy + y + dy, (float)(z + dz)},
                    {ox + x, oy + y + dy, (float)z}, (BlockType)tex, 0.8f);
        }
      }
    }
  };

  auto greedify_yfaces = [&](BlockType type) {
    for (int y = 0; y < 16; y++) {
      int wy = chunkOffset[1] * 16 + y;

      bool rightMask[16][256]{}, leftMask[16][256]{};
      bool rightUsed[16][256]{}, leftUsed[16][256]{};

      for (int z = 0; z < 256; z++) {
        for (int x = 0; x < 16; x++) {
          Voxel &v = layers[z].voxels[x][y];
          if (v.blockType != type || !v.visible)
            continue;
          int wx = chunkOffset[0] * 16 + x;
          auto rightNeighbor = [&]() -> BlockType {
            if (y < 15)
              return (BlockType)layers[z].voxels[x][y + 1].blockType;
            Voxel *rv = gop.getVoxelGlobal({wx, wy + 1, z});
            return rv ? (BlockType)rv->blockType : AIR;
          };
          if (rightNeighbor() == AIR)
            rightMask[x][z] = true;
          auto leftNeighbor = [&]() -> BlockType {
            if (y > 0)
              return (BlockType)layers[z].voxels[x][y - 1].blockType;
            Voxel *lv = gop.getVoxelGlobal({wx, wy - 1, z});
            return lv ? (BlockType)lv->blockType : AIR;
          };
          if (leftNeighbor() == AIR)
            leftMask[x][z] = true;
        }
      }

      for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 256; z++) {
          if (!rightMask[x][z] || rightUsed[x][z])
            continue;
          int tex = layers[z].voxels[x][y].faceTexture[RIGHT];
          int dz = 1;
          while (z + dz < 256 && rightMask[x][z + dz] &&
                 !rightUsed[x][z + dz] &&
                 layers[z + dz].voxels[x][y].blockType == type &&
                 layers[z + dz].voxels[x][y].visible &&
                 layers[z + dz].voxels[x][y].faceTexture[RIGHT] == tex)
            dz++;
          int dx = 1;
          while (x + dx < 16) {
            bool ok = true;
            for (int k = 0; k < dz && ok; k++)
              if (!rightMask[x + dx][z + k] || rightUsed[x + dx][z + k] ||
                  layers[z + k].voxels[x + dx][y].blockType != type ||
                  !layers[z + k].voxels[x + dx][y].visible ||
                  layers[z + k].voxels[x + dx][y].faceTexture[RIGHT] != tex)
                ok = false;
            if (!ok)
              break;
            dx++;
          }
          for (int a = 0; a < dx; a++)
            for (int b = 0; b < dz; b++)
              rightUsed[x + a][z + b] = true;

          emmitQuad({ox + x, oy + y + 1, (float)z},
                    {ox + x, oy + y + 1, (float)(z + dz)},
                    {ox + x + dx, oy + y + 1, (float)(z + dz)},
                    {ox + x + dx, oy + y + 1, (float)z}, (BlockType)tex, 0.9f);
        }
      }

      for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 256; z++) {
          if (!leftMask[x][z] || leftUsed[x][z])
            continue;
          int tex = layers[z].voxels[x][y].faceTexture[LEFT];
          int dz = 1;
          while (z + dz < 256 && leftMask[x][z + dz] && !leftUsed[x][z + dz] &&
                 layers[z + dz].voxels[x][y].blockType == type &&
                 layers[z + dz].voxels[x][y].visible &&
                 layers[z + dz].voxels[x][y].faceTexture[LEFT] == tex)
            dz++;
          int dx = 1;
          while (x + dx < 16) {
            bool ok = true;
            for (int k = 0; k < dz && ok; k++)
              if (!leftMask[x + dx][z + k] || leftUsed[x + dx][z + k] ||
                  layers[z + k].voxels[x + dx][y].blockType != type ||
                  !layers[z + k].voxels[x + dx][y].visible ||
                  layers[z + k].voxels[x + dx][y].faceTexture[LEFT] != tex)
                ok = false;
            if (!ok)
              break;
            dx++;
          }
          for (int a = 0; a < dx; a++)
            for (int b = 0; b < dz; b++)
              leftUsed[x + a][z + b] = true;

          emmitQuad({ox + x, oy + y, (float)z}, {ox + x + dx, oy + y, (float)z},
                    {ox + x + dx, oy + y, (float)(z + dz)},
                    {ox + x, oy + y, (float)(z + dz)}, (BlockType)tex, 0.9f);
        }
      }
    }
  };

  for (int i = GRASS; i < END_BLOCK_TYPE; i++) {
    BlockType bt = (BlockType)i;
    for (auto &layer : layers)
      greedify_topbottom(bt, layer);
    greedify_xfaces(bt);
    greedify_yfaces(bt);
  }

  chunkMesh.loadVertices(vertices);
  chunkMesh.loadIndices(indices);
  TIMER_END(mesh);
}
void Chunk::createBuffers() {
  chunkMesh.createVertexBuffer();
  chunkMesh.createIndexBuffer();
}

void Chunk::updateChunkMesh() {
  makeVisible();
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
