#include "Chunk.h"
#include <chrono>
#include <iostream>
#include "PerlinNoise.hpp"

Chunk::Chunk(VulkanContext &vkContext, GameObjectPool &gameObjectPool) : vkContext{vkContext}, chunkMesh{vkContext}, backMesh{vkContext}, gameObjectPool{gameObjectPool}
{
}

void Chunk::setOffset(int x, int y)
{
    this->chunkOffsetX = x;
    this->chunkOffsetY = y;
}

void Chunk::populateBlocks()
{
    voxels.reserve(16 * 16 * 256);
    int cmx = chunkOffsetX * 16;
    int cmy = chunkOffsetY * 16;

    const siv::PerlinNoise::seed_type seed = 87844057u;

    const siv::PerlinNoise perlin{seed};

    for (int z = 0; z < 256; z++)
    {
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                const double surfaceNoise = 10.f - floor(perlin.octave2D_01((x + cmx) * .02f, (y + cmy) * .02f, 2) * 10);
                const double stoneNoise = 8.f - floor(perlin.octave2D_01((x + cmx) * .025f, (y + cmy) * .025f, 6) * 8);
                int blockType;

                int zP = z + surfaceNoise;
                int height = 60 + (int)surfaceNoise; // grass/dirt surface
                int stoneHeight = 50 + (int)stoneNoise;

                if (z > height)
                    blockType = AIR;
                else if (z == height)
                    blockType = GRASS;
                else if (z > stoneHeight)
                    blockType = DIRT;
                else if (z == 0)
                    blockType = BEDROCK;
                else
                    blockType = STONE;
                voxels.emplace_back(vkContext, BlockType(blockType));
                voxels.back().setType(BlockType(blockType));

                // storing global coords for GC detection
                voxels.back().setPosition({x + 0.5f + cmx,
                                           y + 0.5f + cmy,
                                           z + 0.5f});
            }
        }
    }
}

bool Chunk::isFaceVisible(int x, int y, int z)
{
    Voxel *v = gameObjectPool.getVoxelGlobal({x + chunkOffsetX * 16,
                                              y + chunkOffsetY * 16,
                                              z});
    return v == nullptr || v->getBlockType() == AIR;
}

void Chunk::genMesh(bool useChunkMesh)
{
    if (!populated)
        return;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(16 * 16 * 64 * 4);
    indices.reserve(16 * 16 * 64 * 6);

    // Each face wound CCW when viewed from OUTSIDE (normal = cross(e1, e2) points outward)
    // Verified: cross(v1-v0, v2-v0) must point in the face's outward normal direction
    static const FaceDef faces[6] = {
        // Top (+Z)
        {{{-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}},
         0,
         1.0f,
         0,
         0,
         1},

        // Bottom (-Z)
        {{{-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}},
         1,
         0.5f,
         0,
         0,
         -1},

        // Front (+Y)
        {{{-0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f}},
         2,
         0.75f,
         0,
         1,
         0},

        // Back (-Y)
        {{{0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f}},
         3,
         0.85f,
         0,
         -1,
         0},

        // Right (+X)
        {{{0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}},
         4,
         0.7f,
         1,
         0,
         0},

        // Left (-X)
        {{{-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}},
         5,
         0.8f,
         -1,
         0,
         0},
    };

    static auto blockColor = [](BlockType type) -> glm::vec3
    {
        switch (type)
        {
        case STONE:
            return {0.5f, 0.5f, 0.5f};
        case GRASS:
            return {0.4f, 0.8f, 0.3f};
        default:
            return {1.0f, 1.0f, 1.0f};
        }
    };

    for (int z = 0; z < 256; z++)
    {
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                const int idx = z * 256 + x * 16 + y;
                const BlockType type = voxels[idx].getBlockType();
                const int *faceTexture = voxels[idx].faceTexture;

                if (type == AIR)
                    continue;

                const glm::vec3 color = blockColor(type);
                const glm::vec3 origin = voxels[idx].transform.position - glm::vec3(16 * chunkOffsetX, 16 * chunkOffsetY, 0);

                for (const FaceDef &face : faces)
                {
                    if (!isFaceVisible(x + face.dx, y + face.dy, z + face.dz))
                        continue;

                    const uint32_t base = static_cast<uint32_t>(vertices.size());

                    vertices.push_back({origin + face.corners[0], color, glm::vec2(faceTexture[face.tileOffset], 0), face.light});
                    vertices.push_back({origin + face.corners[1], color, glm::vec2(faceTexture[face.tileOffset], 1), face.light});
                    vertices.push_back({origin + face.corners[2], color, glm::vec2(faceTexture[face.tileOffset], 2), face.light});
                    vertices.push_back({origin + face.corners[3], color, glm::vec2(faceTexture[face.tileOffset], 3), face.light});

                    indices.insert(indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
                }
            }
        }
    }

    if (useChunkMesh)
    {
        chunkMesh.vertices = std::move(vertices);
        chunkMesh.indices = std::move(indices);
    }
    else
    {
        backMesh.vertices = std::move(vertices);
        backMesh.indices = std::move(indices);
    }
}

void Chunk::buildChunkMesh()
{

    genMesh(true);
    backMesh.vertices = chunkMesh.vertices;
    backMesh.indices = chunkMesh.indices;
    // genMesh();
    chunkMesh.createVertexBuffer();
    chunkMesh.createIndexBuffer();
    backMesh.createVertexBuffer();
    backMesh.createIndexBuffer();

    renderable = populated;
}

void Chunk::updateChunkMesh()
{
    std::cerr << "Generating new chunk mesh: "
              << chunkOffsetX << ", " << chunkOffsetY << std::endl;
    genMesh();
    backMesh.updateVertexBuffer();
    backMesh.updateIndexBuffer();
}

void Chunk::swapMesh()
{
    chunkMesh.vertices = std::move(backMesh.vertices);
    chunkMesh.indices = std::move(backMesh.indices);

    backMesh.vertices.clear();
    backMesh.indices.clear();

    chunkMesh.updateVertexBuffer();
    chunkMesh.updateIndexBuffer();
}

void Chunk::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
                 std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent)
{
    if (!renderable)
        return;

    VkBuffer vertexBuffers[] = {chunkMesh.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, chunkMesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    PushConstantC1 c1;
    c1.data = glm::translate(glm::mat4(1.f), glm::vec3(chunkOffsetX * 16, chunkOffsetY * 16, 0));
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantC1), &c1);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &(descriptorSets[currentFrame]), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(chunkMesh.indices.size()), 1, 0, 0, 0);
}

void Chunk::cleanup()
{
    chunkMesh.cleanup();
    backMesh.cleanup();
}

Chunk::~Chunk()
{
}