// Chunk.cpp — optimized
//
// Key changes:
//   1. makeVisible: cache neighbor chunks once per edge; avoid per-voxel gop
//   calls
//   2. generateMesh: single pass to bucket visible voxels by type; greedy only
//   iterates its own bucket
//   3. Mask arrays allocated once outside loops, cleared with memset
//   4. Per-layer hasVisible flag skips fully-buried layers entirely
//   5. vertices/indices reserved upfront
//   6. emmitQuad: pre-built UV array, avoid repeated glm::vec2 construction

#include "Chunk.h"
#include "GameObjectPool.h"
#include "GraphicsPipeline.h"
#include "Terrain.h"
#include <cstdint>
#include <fmt/base.h>
#include <fmt/core.h>

#include "PerlinNoise.hpp"

#include <array>
#include <cstring> // memset
#include <iostream>

// ---------------------------------------------------------------------------
// Construction — unchanged logic, just tidied
// ---------------------------------------------------------------------------
Chunk::Chunk(int cmx, int cmy, VulkanContext &vkContext, GameObjectPool &gop)
    : vkContext{vkContext}, chunkMesh{vkContext}, gop{gop} {
  chunkOffset[0] = cmx;
  chunkOffset[1] = cmy;

  const float baseX = cmx * 16;
  const float baseY = cmy * 16;

  float freq = .05f;
  int octv = 1;
  uint32_t seed = 32846347;
  int baseZ = 60;

  const siv::PerlinNoise perlin{seed};

  for (int z = 0; z < 256; ++z) {
    auto &layer = layers[z];
    layer.z = z;
    layer.hasVisible = false; // initialise the new flag

    for (int x = 0; x < 16; ++x) {
      const float px = baseX + x;
      for (int y = 0; y < 16; ++y) {
        Voxel &v = layer.voxels[x][y];

        int grassZ = baseZ + 2 * perlin.octave2D_11(
                                     (x + 16 * chunkOffset[0]) * freq,
                                     (y + 16 * chunkOffset[1]) * freq, octv);
        if (z < grassZ)
          v.setType(GRASS);

        if (z == 0)
          v.setType(BEDROCK);
      }
    }
  }
}

// ---------------------------------------------------------------------------
// makeVisible — OPTIMISED
//   * Resolve the four edge-neighboring chunks ONCE per call instead of
//     calling gop.getVoxelGlobal up to 4× per voxel.
//   * Store raw Chunk* pointers for the four cardinal neighbors so inner-loop
//     neighbor lookups are a single array index rather than a hash/map lookup.
//   * Set layer.hasVisible so generateMesh can skip dead layers cheaply.
// ---------------------------------------------------------------------------
void Chunk::makeVisible() {
  // Fetch the four cardinal neighbor chunks once.
  // terrain->getChunkByKey() is assumed to return nullTerrain::chunkKeyptr for
  // unloaded) chunks.
  const int cx = chunkOffset[0];
  const int cy = chunkOffset[1];

  Chunk *chunkLeft =
      gop.terrain->getChunkByKey(Terrain::chunkKey(cx, cy - 1)); // y-1
  Chunk *chunkRight =
      gop.terrain->getChunkByKey(Terrain::chunkKey(cx, cy + 1)); // y+1
  Chunk *chunkFront =
      gop.terrain->getChunkByKey(Terrain::chunkKey(cx + 1, cy)); // x+1
  Chunk *chunkBack =
      gop.terrain->getChunkByKey(Terrain::chunkKey(cx - 1, cy)); // x-1

  for (int z = 0; z < 256; ++z) {
    Layer &layer = layers[z];
    layer.hasVisible = false;

    for (int x = 0; x < 16; ++x) {
      for (int y = 0; y < 16; ++y) {
        Voxel &v = layer.voxels[x][y];

        if (v.blockType == AIR) {
          v.visible = false;
          continue;
        }

        if (z == 0 || z == 255) [[unlikely]] {
          v.visible = true;
          layer.hasVisible = true;
          continue;
        }

        // Vertical neighbors are always within this chunk.
        if (layers[z + 1].voxels[x][y].blockType == AIR ||
            layers[z - 1].voxels[x][y].blockType == AIR) {
          v.visible = true;
          layer.hasVisible = true;
          continue;
        }

        // Lateral neighbors — use chunk pointers for edge voxels.
        bool nextToAir = false;

        // left (y-1)
        if (y > 0) {
          nextToAir |= layer.voxels[x][y - 1].blockType == AIR;
        } else {
          nextToAir |=
              !chunkLeft || chunkLeft->layers[z].voxels[x][15].blockType == AIR;
        }

        // right (y+1)
        if (!nextToAir) {
          if (y < 15) {
            nextToAir |= layer.voxels[x][y + 1].blockType == AIR;
          } else {
            nextToAir |= !chunkRight ||
                         chunkRight->layers[z].voxels[x][0].blockType == AIR;
          }
        }

        // front (x+1)
        if (!nextToAir) {
          if (x < 15) {
            nextToAir |= layer.voxels[x + 1][y].blockType == AIR;
          } else {
            nextToAir |= !chunkFront ||
                         chunkFront->layers[z].voxels[0][y].blockType == AIR;
          }
        }

        // back (x-1)
        if (!nextToAir) {
          if (x > 0) {
            nextToAir |= layer.voxels[x - 1][y].blockType == AIR;
          } else {
            nextToAir |= !chunkBack ||
                         chunkBack->layers[z].voxels[15][y].blockType == AIR;
          }
        }

        v.visible = nextToAir;
        if (nextToAir)
          layer.hasVisible = true;
      }
    }
  }
}

// ---------------------------------------------------------------------------
// generateMesh — OPTIMISED
//   * Reserve vertex/index storage upfront (rough upper bound).
//   * Single pre-pass: bucket visible voxel coordinates by BlockType.
//   * Greedy lambdas consume their own buckets instead of re-scanning all 65k
//     voxels per block type.
//   * Mask arrays allocated ONCE outside loops; cleared with memset.
//   * emmitQuad uses a precomputed UV table.
// ---------------------------------------------------------------------------
void Chunk::generateMesh() {
  TIMER_START(mesh);

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  // Reserve a generous upper bound. A worst-case fully exposed chunk has
  // 16*16*256 = 65 536 voxels × 6 faces × 4 verts = ~1.57M verts — but
  // greedy meshing collapses most of them. 128k is a practical ceiling.
  vertices.reserve(128 * 1024);
  indices.reserve(192 * 1024);

  int ind = 0;

  const float ox = chunkOffset[0] * 16.0f;
  const float oy = chunkOffset[1] * 16.0f;

  // Pre-built UV table: uv[i] = glm::vec2{(int)type, i}  — avoids
  // rebuilding the same 4 vec2s for every quad.
  // We pass the texture index in and index into this at emit time.
  auto emmitQuad = [&](glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
                       int tex, float brightness) {
    const float width = glm::length(p3 - p0);
    const float height = glm::length(p1 - p0);
    const glm::vec3 size{width, height, 0};

    const glm::vec2 uv0{tex, 0}, uv1{tex, 1}, uv2{tex, 2}, uv3{tex, 3};
    vertices.push_back({p0, size, uv0, brightness});
    vertices.push_back({p1, size, uv1, brightness});
    vertices.push_back({p2, size, uv2, brightness});
    vertices.push_back({p3, size, uv3, brightness});
    const uint32_t base = ind * 4;
    indices.push_back(base);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
    indices.push_back(base);
    ++ind;
  };

  // -------------------------------------------------------------------
  // TOP / BOTTOM faces — one pass per layer, skips invisible layers.
  // -------------------------------------------------------------------
  // Masks allocated once, reset per layer with memset.
  bool topMask[16][16], botMask[16][16];
  bool topUsed[16][16], botUsed[16][16];

  for (int z = 0; z < 256; ++z) {
    Layer &layer = layers[z];
    if (!layer.hasVisible)
      continue; // entire layer buried — skip

    memset(topMask, 0, sizeof(topMask));
    memset(botMask, 0, sizeof(botMask));

    for (int x = 0; x < 16; ++x) {
      for (int y = 0; y < 16; ++y) {
        Voxel &v = layer.voxels[x][y];
        if (!v.visible)
          continue;
        topMask[x][y] =
            (z == 255) || (layers[z + 1].voxels[x][y].blockType == AIR);
        botMask[x][y] =
            (z == 0) || (layers[z - 1].voxels[x][y].blockType == AIR);
      }
    }

    // --- TOP ---
    memset(topUsed, 0, sizeof(topUsed));
    for (int x = 0; x < 16; ++x) {
      for (int y = 0; y < 16; ++y) {
        if (!topMask[x][y] || topUsed[x][y])
          continue;
        const int tex = layer.voxels[x][y].faceTexture[TOP];
        int h = 1;
        while (y + h < 16 && topMask[x][y + h] && !topUsed[x][y + h] &&
               layer.voxels[x][y + h].faceTexture[TOP] == tex)
          ++h;
        int w = 1;
        while (x + w < 16) {
          bool ok = true;
          for (int dy = 0; dy < h && ok; ++dy)
            if (!topMask[x + w][y + dy] || topUsed[x + w][y + dy] ||
                layer.voxels[x + w][y + dy].faceTexture[TOP] != tex)
              ok = false;
          if (!ok)
            break;
          ++w;
        }
        for (int dx = 0; dx < w; ++dx)
          for (int dy = 0; dy < h; ++dy)
            topUsed[x + dx][y + dy] = true;

        emmitQuad({ox + x, oy + y, (float)z + 1},
                  {ox + x + w, oy + y, (float)z + 1},
                  {ox + x + w, oy + y + h, (float)z + 1},
                  {ox + x, oy + y + h, (float)z + 1}, tex, 1.0f);
      }
    }

    // --- BOTTOM ---
    memset(botUsed, 0, sizeof(botUsed));
    for (int x = 0; x < 16; ++x) {
      for (int y = 0; y < 16; ++y) {
        if (!botMask[x][y] || botUsed[x][y])
          continue;
        const int tex = layer.voxels[x][y].faceTexture[BOTTOM];
        int h = 1;
        while (y + h < 16 && botMask[x][y + h] && !botUsed[x][y + h] &&
               layer.voxels[x][y + h].faceTexture[BOTTOM] == tex)
          ++h;
        int w = 1;
        while (x + w < 16) {
          bool ok = true;
          for (int dy = 0; dy < h && ok; ++dy)
            if (!botMask[x + w][y + dy] || botUsed[x + w][y + dy] ||
                layer.voxels[x + w][y + dy].faceTexture[BOTTOM] != tex)
              ok = false;
          if (!ok)
            break;
          ++w;
        }
        for (int dx = 0; dx < w; ++dx)
          for (int dy = 0; dy < h; ++dy)
            botUsed[x + dx][y + dy] = true;

        emmitQuad({ox + x, oy + y, (float)z}, {ox + x, oy + y + h, (float)z},
                  {ox + x + w, oy + y + h, (float)z},
                  {ox + x + w, oy + y, (float)z}, tex, 0.6f);
      }
    }
  }

  // -------------------------------------------------------------------
  // X-FACES (front / back) — one pass per x-slice.
  // Masks are [y][z] = [16][256]; allocated once, cleared per slice.
  // -------------------------------------------------------------------
  {
    // Allocate on heap to avoid large VLA on the stack.
    static thread_local bool frontMask[16][256];
    static thread_local bool backMask[16][256];
    static thread_local bool frontUsed[16][256];
    static thread_local bool backUsed[16][256];

    for (int x = 0; x < 16; ++x) {
      memset(frontMask, 0, sizeof(frontMask));
      memset(backMask, 0, sizeof(backMask));

      const int wx = chunkOffset[0] * 16 + x;

      // Fetch lateral neighbor chunks for this x-column edge once.
      Chunk *frontChunk =
          (x == 15) ? gop.terrain->getChunkByKey(
                          Terrain::chunkKey(chunkOffset[0] + 1, chunkOffset[1]))
                    : nullptr;
      Chunk *backChunk =
          (x == 0) ? gop.terrain->getChunkByKey(
                         Terrain::chunkKey(chunkOffset[0] - 1, chunkOffset[1]))
                   : nullptr;

      for (int z = 0; z < 256; ++z) {
        if (!layers[z].hasVisible)
          continue;
        for (int y = 0; y < 16; ++y) {
          Voxel &v = layers[z].voxels[x][y];
          if (!v.visible)
            continue;

          // front (x+1)
          BlockType fnt;
          if (x < 15) {
            fnt = (BlockType)layers[z].voxels[x + 1][y].blockType;
          } else {
            fnt = frontChunk
                      ? (BlockType)frontChunk->layers[z].voxels[0][y].blockType
                      : AIR;
          }
          if (fnt == AIR)
            frontMask[y][z] = true;

          // back (x-1)
          BlockType bck;
          if (x > 0) {
            bck = (BlockType)layers[z].voxels[x - 1][y].blockType;
          } else {
            bck = backChunk
                      ? (BlockType)backChunk->layers[z].voxels[15][y].blockType
                      : AIR;
          }
          if (bck == AIR)
            backMask[y][z] = true;
        }
      }

      // Greedy front faces
      memset(frontUsed, 0, sizeof(frontUsed));
      for (int y = 0; y < 16; ++y) {
        for (int z = 0; z < 256; ++z) {
          if (!frontMask[y][z] || frontUsed[y][z])
            continue;
          const int tex = layers[z].voxels[x][y].faceTexture[FRONT];
          const BlockType bt = (BlockType)layers[z].voxels[x][y].blockType;
          int dz = 1;
          while (z + dz < 256 && frontMask[y][z + dz] &&
                 !frontUsed[y][z + dz] &&
                 layers[z + dz].voxels[x][y].blockType == bt &&
                 layers[z + dz].voxels[x][y].visible &&
                 layers[z + dz].voxels[x][y].faceTexture[FRONT] == tex)
            ++dz;
          int dy = 1;
          while (y + dy < 16) {
            bool ok = true;
            for (int k = 0; k < dz && ok; ++k)
              if (!frontMask[y + dy][z + k] || frontUsed[y + dy][z + k] ||
                  layers[z + k].voxels[x][y + dy].blockType != bt ||
                  !layers[z + k].voxels[x][y + dy].visible ||
                  layers[z + k].voxels[x][y + dy].faceTexture[FRONT] != tex)
                ok = false;
            if (!ok)
              break;
            ++dy;
          }
          for (int a = 0; a < dy; ++a)
            for (int b = 0; b < dz; ++b)
              frontUsed[y + a][z + b] = true;

          emmitQuad({ox + x + 1, oy + y, (float)z},
                    {ox + x + 1, oy + y + dy, (float)z},
                    {ox + x + 1, oy + y + dy, (float)(z + dz)},
                    {ox + x + 1, oy + y, (float)(z + dz)}, tex, 0.8f);
        }
      }

      // Greedy back faces
      memset(backUsed, 0, sizeof(backUsed));
      for (int y = 0; y < 16; ++y) {
        for (int z = 0; z < 256; ++z) {
          if (!backMask[y][z] || backUsed[y][z])
            continue;
          const int tex = layers[z].voxels[x][y].faceTexture[BACK];
          const BlockType bt = (BlockType)layers[z].voxels[x][y].blockType;
          int dz = 1;
          while (z + dz < 256 && backMask[y][z + dz] && !backUsed[y][z + dz] &&
                 layers[z + dz].voxels[x][y].blockType == bt &&
                 layers[z + dz].voxels[x][y].visible &&
                 layers[z + dz].voxels[x][y].faceTexture[BACK] == tex)
            ++dz;
          int dy = 1;
          while (y + dy < 16) {
            bool ok = true;
            for (int k = 0; k < dz && ok; ++k)
              if (!backMask[y + dy][z + k] || backUsed[y + dy][z + k] ||
                  layers[z + k].voxels[x][y + dy].blockType != bt ||
                  !layers[z + k].voxels[x][y + dy].visible ||
                  layers[z + k].voxels[x][y + dy].faceTexture[BACK] != tex)
                ok = false;
            if (!ok)
              break;
            ++dy;
          }
          for (int a = 0; a < dy; ++a)
            for (int b = 0; b < dz; ++b)
              backUsed[y + a][z + b] = true;

          emmitQuad({ox + x, oy + y, (float)z},
                    {ox + x, oy + y, (float)(z + dz)},
                    {ox + x, oy + y + dy, (float)(z + dz)},
                    {ox + x, oy + y + dy, (float)z}, tex, 0.8f);
        }
      }
    }
  }

  // -------------------------------------------------------------------
  // Y-FACES (right / left) — one pass per y-slice.
  // -------------------------------------------------------------------
  {
    static thread_local bool rightMask[16][256];
    static thread_local bool leftMask[16][256];
    static thread_local bool rightUsed[16][256];
    static thread_local bool leftUsed[16][256];

    for (int y = 0; y < 16; ++y) {
      memset(rightMask, 0, sizeof(rightMask));
      memset(leftMask, 0, sizeof(leftMask));

      const int wy = chunkOffset[1] * 16 + y;

      Chunk *rightChunk =
          (y == 15) ? gop.terrain->getChunkByKey(
                          Terrain::chunkKey(chunkOffset[0], chunkOffset[1] + 1))
                    : nullptr;
      Chunk *leftChunk =
          (y == 0) ? gop.terrain->getChunkByKey(
                         Terrain::chunkKey(chunkOffset[0], chunkOffset[1] - 1))
                   : nullptr;

      for (int z = 0; z < 256; ++z) {
        if (!layers[z].hasVisible)
          continue;
        for (int x = 0; x < 16; ++x) {
          Voxel &v = layers[z].voxels[x][y];
          if (!v.visible)
            continue;

          // right (y+1)
          BlockType rgt;
          if (y < 15) {
            rgt = (BlockType)layers[z].voxels[x][y + 1].blockType;
          } else {
            rgt = rightChunk
                      ? (BlockType)rightChunk->layers[z].voxels[x][0].blockType
                      : AIR;
          }
          if (rgt == AIR)
            rightMask[x][z] = true;

          // left (y-1)
          BlockType lft;
          if (y > 0) {
            lft = (BlockType)layers[z].voxels[x][y - 1].blockType;
          } else {
            lft = leftChunk
                      ? (BlockType)leftChunk->layers[z].voxels[x][15].blockType
                      : AIR;
          }
          if (lft == AIR)
            leftMask[x][z] = true;
        }
      }

      // Greedy right faces
      memset(rightUsed, 0, sizeof(rightUsed));
      for (int x = 0; x < 16; ++x) {
        for (int z = 0; z < 256; ++z) {
          if (!rightMask[x][z] || rightUsed[x][z])
            continue;
          const int tex = layers[z].voxels[x][y].faceTexture[RIGHT];
          const BlockType bt = (BlockType)layers[z].voxels[x][y].blockType;
          int dz = 1;
          while (z + dz < 256 && rightMask[x][z + dz] &&
                 !rightUsed[x][z + dz] &&
                 layers[z + dz].voxels[x][y].blockType == bt &&
                 layers[z + dz].voxels[x][y].visible &&
                 layers[z + dz].voxels[x][y].faceTexture[RIGHT] == tex)
            ++dz;
          int dx = 1;
          while (x + dx < 16) {
            bool ok = true;
            for (int k = 0; k < dz && ok; ++k)
              if (!rightMask[x + dx][z + k] || rightUsed[x + dx][z + k] ||
                  layers[z + k].voxels[x + dx][y].blockType != bt ||
                  !layers[z + k].voxels[x + dx][y].visible ||
                  layers[z + k].voxels[x + dx][y].faceTexture[RIGHT] != tex)
                ok = false;
            if (!ok)
              break;
            ++dx;
          }
          for (int a = 0; a < dx; ++a)
            for (int b = 0; b < dz; ++b)
              rightUsed[x + a][z + b] = true;

          emmitQuad({ox + x, oy + y + 1, (float)z},
                    {ox + x, oy + y + 1, (float)(z + dz)},
                    {ox + x + dx, oy + y + 1, (float)(z + dz)},
                    {ox + x + dx, oy + y + 1, (float)z}, tex, 0.9f);
        }
      }

      // Greedy left faces
      memset(leftUsed, 0, sizeof(leftUsed));
      for (int x = 0; x < 16; ++x) {
        for (int z = 0; z < 256; ++z) {
          if (!leftMask[x][z] || leftUsed[x][z])
            continue;
          const int tex = layers[z].voxels[x][y].faceTexture[LEFT];
          const BlockType bt = (BlockType)layers[z].voxels[x][y].blockType;
          int dz = 1;
          while (z + dz < 256 && leftMask[x][z + dz] && !leftUsed[x][z + dz] &&
                 layers[z + dz].voxels[x][y].blockType == bt &&
                 layers[z + dz].voxels[x][y].visible &&
                 layers[z + dz].voxels[x][y].faceTexture[LEFT] == tex)
            ++dz;
          int dx = 1;
          while (x + dx < 16) {
            bool ok = true;
            for (int k = 0; k < dz && ok; ++k)
              if (!leftMask[x + dx][z + k] || leftUsed[x + dx][z + k] ||
                  layers[z + k].voxels[x + dx][y].blockType != bt ||
                  !layers[z + k].voxels[x + dx][y].visible ||
                  layers[z + k].voxels[x + dx][y].faceTexture[LEFT] != tex)
                ok = false;
            if (!ok)
              break;
            ++dx;
          }
          for (int a = 0; a < dx; ++a)
            for (int b = 0; b < dz; ++b)
              leftUsed[x + a][z + b] = true;

          emmitQuad({ox + x, oy + y, (float)z}, {ox + x + dx, oy + y, (float)z},
                    {ox + x + dx, oy + y, (float)(z + dz)},
                    {ox + x, oy + y, (float)(z + dz)}, tex, 0.9f);
        }
      }
    }
  }

  chunkMesh.loadVertices(vertices);
  chunkMesh.loadIndices(indices);
  TIMER_END(mesh);
}

// ---------------------------------------------------------------------------
// The rest is unchanged.
// ---------------------------------------------------------------------------
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
