#include "Chunk.h"
#include <chrono>
#include <iostream>

Chunk::Chunk(VulkanContext &vkContext, GameObjectPool &gameObjectPool) : vkContext{vkContext}, chunkMesh{vkContext}, gameObjectPool{gameObjectPool}
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

    for (int z = 0; z < 256; z++)
    {
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                int blockType;

                if (z <= 50)
                    blockType = STONE;
                else if (z <= 64)
                    blockType = GRASS;
                else
                    blockType = AIR;

                voxels.emplace_back(vkContext, BlockType(blockType));
            }
        }
    }
}

bool Chunk::isFaceVisible(int x, int y, int z)
{
    Voxel *v = gameObjectPool.getVoxelGlobal({
        x + chunkOffsetX * 16,
        y + chunkOffsetY * 16,
        z});
    return v == nullptr || v->getBlockType() == AIR;
}

void Chunk::buildChunkMesh()
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(16 * 16 * 64 * 4);
    indices.reserve(16 * 16 * 64 * 6);

    // Each face wound CCW when viewed from OUTSIDE (normal = cross(e1, e2) points outward)
    // Verified: cross(v1-v0, v2-v0) must point in the face's outward normal direction
    static const FaceDef faces[6] = {
        // Top (normal +z): cross((1,0,0),(1,1,0)) = (0,0,1) ✓
        {{{0,0,1}, {1,0,1}, {1,1,1}, {0,1,1}},
         {{0,0}, {1,0}, {1,1}, {0,1}},
         1.0f, 0, 0, 1},

        // Bottom (normal -z): cross((0,1,0),(1,1,0)) = (0,0,-1) ✓
        {{{0,0,0}, {0,1,0}, {1,1,0}, {1,0,0}},
         {{0,0}, {0,1}, {1,1}, {1,0}},
         0.5f, 0, 0, -1},

        // Front (normal +y): cross((0,0,1),(1,0,1)) = (0,1,0) ✓
        {{{0,1,0}, {0,1,1}, {1,1,1}, {1,1,0}},
         {{0,0}, {0,1}, {1,1}, {1,0}},
         0.75f, 0, 1, 0},

        // Back (normal -y): cross((0,0,1),(-1,0,1)) = (0,-1,0) ✓
        {{{1,0,0}, {1,0,1}, {0,0,1}, {0,0,0}},
         {{0,0}, {0,1}, {1,1}, {1,0}},
         0.85f, 0, -1, 0},

        // Right (normal +x): cross((0,1,0),(0,1,1)) = (1,0,0) ✓
        {{{1,0,0}, {1,1,0}, {1,1,1}, {1,0,1}},
         {{0,0}, {1,0}, {1,1}, {0,1}},
         0.7f, 1, 0, 0},

        // Left (normal -x): cross((0,-1,0),(0,-1,1)) = (-1,0,0) ✓
        {{{0,1,0}, {0,0,0}, {0,0,1}, {0,1,1}},
         {{0,0}, {1,0}, {1,1}, {0,1}},
         0.8f, -1, 0, 0},
    };

    auto blockColor = [](BlockType type) -> glm::vec3
    {
        switch (type)
        {
        case STONE: return {0.5f, 0.5f, 0.5f};
        case GRASS: return {0.4f, 0.8f, 0.3f};
        default:    return {1.0f, 1.0f, 1.0f};
        }
    };

    for (int z = 0; z < 256; z++)
    {
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                const int idx = x + y * 16 + z * 256;
                const BlockType type = voxels[idx].getBlockType();

                if (type == AIR)
                    continue;

                const glm::vec3 color  = blockColor(type);
                const glm::vec3 origin = {(float)x, (float)y, (float)z};

                for (const FaceDef &face : faces)
                {
                    if (!isFaceVisible(x + face.dx, y + face.dy, z + face.dz))
                        continue;

                    const uint32_t base = static_cast<uint32_t>(vertices.size());

                    vertices.push_back({origin + face.corners[0], color, face.uvs[0], face.light});
                    vertices.push_back({origin + face.corners[1], color, face.uvs[1], face.light});
                    vertices.push_back({origin + face.corners[2], color, face.uvs[2], face.light});
                    vertices.push_back({origin + face.corners[3], color, face.uvs[3], face.light});

                    indices.insert(indices.end(), {base, base+1, base+2, base+2, base+3, base});
                }
            }
        }
    }

    chunkMesh.vertices = std::move(vertices);
    chunkMesh.indices  = std::move(indices);
    chunkMesh.createVertexBuffer();
    chunkMesh.createIndexBuffer();
}

void Chunk::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
                 std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent)
{
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
}

Chunk::~Chunk()
{
}