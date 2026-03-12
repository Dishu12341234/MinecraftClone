#include "Chunk.h"
#include <chrono>
#include <iostream>

#define TIMER_START(name) \
    auto name##_start = std::chrono::high_resolution_clock::now();

#define TIMER_END(name)                                                                                             \
    {                                                                                                               \
        auto name##_end = std::chrono::high_resolution_clock::now();                                                \
        auto name##_dur = std::chrono::duration_cast<std::chrono::milliseconds>(name##_end - name##_start).count(); \
        std::cout << #name << " took " << name##_dur << " ms\n";                                                    \
    }

Chunk::Chunk(VulkanContext &vkContext) : vkContext{vkContext}, chunkMesh{vkContext}
{
}

void Chunk::setOffset(int x, int y)
{
    this->chunkOffsetX = x;
    this->chunkOffsetY = y;
}

void Chunk::populateBlocks()
{
    TIMER_START(POPULATING_BLOCKS)

    voxels.reserve(16 * 16 * 256);

    for (int z = 0; z < 256; z++)
    {
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                int blockType;

                if (y <= 50)
                    blockType = STONE; // bottom layers
                else if (y <= 64)
                    blockType = GRASS; // top layers
                else
                    blockType = AIR; // above

                voxels.emplace_back(
                    vkContext,
                    BlockType(blockType));
            }
        }
    }

    TIMER_END(POPULATING_BLOCKS)
}

void Chunk::buildChunkMesh()
{
    TIMER_START(BUILD_CHUNK_MESH)

    std::vector<Vertex> vertices = {
        // Front face
        {{-0.5f, -0.5f, 0.5f}, {1, 0, 0}, {0, 0}},
        {{0.5f, -0.5f, 0.5f}, {0, 1, 0}, {1, 0}},
        {{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 1}},
        {{-0.5f, 0.5f, 0.5f}, {1, 1, 0}, {0, 1}},

        // Back face
        {{0.5f, -0.5f, -0.5f}, {1, 0, 1}, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, {0, 1, 1}, {1, 0}},
        {{-0.5f, 0.5f, -0.5f}, {1, 1, 1}, {1, 1}},
        {{0.5f, 0.5f, -0.5f}, {0, 0, 0}, {0, 1}},

        // Right face
        {{0.5f, -0.5f, 0.5f}, {1, 0, 0}, {0, 0}},
        {{0.5f, -0.5f, -0.5f}, {0, 1, 0}, {1, 0}},
        {{0.5f, 0.5f, -0.5f}, {0, 0, 1}, {1, 1}},
        {{0.5f, 0.5f, 0.5f}, {1, 1, 0}, {0, 1}},

        // Left face
        {{-0.5f, -0.5f, -0.5f}, {1, 0, 0}, {0, 0}},
        {{-0.5f, -0.5f, 0.5f}, {0, 1, 0}, {1, 0}},
        {{-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 1}},
        {{-0.5f, 0.5f, -0.5f}, {1, 1, 0}, {0, 1}},

        // Top face
        {{-0.5f, 0.5f, 0.5f}, {1, 0, 0}, {0, 0}},
        {{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0}},
        {{0.5f, 0.5f, -0.5f}, {0, 0, 1}, {1, 1}},
        {{-0.5f, 0.5f, -0.5f}, {1, 1, 0}, {0, 1}},

        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, {1, 0, 0}, {0, 0}},
        {{0.5f, -0.5f, -0.5f}, {0, 1, 0}, {1, 0}},
        {{0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 1}},
        {{-0.5f, -0.5f, 0.5f}, {1, 1, 0}, {0, 1}},
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0,       // Front
        4, 5, 6, 6, 7, 4,       // Back
        8, 9, 10, 10, 11, 8,    // Right
        12, 13, 14, 14, 15, 12, // Left
        16, 17, 18, 18, 19, 16, // Top
        20, 21, 22, 22, 23, 20  // Bottom
    };

    chunkMesh.vertices = vertices;
    chunkMesh.indices = indices;

    chunkMesh.createVertexBuffer();
    chunkMesh.createIndexBuffer();

    TIMER_END(BUILD_CHUNK_MESH)
}

void Chunk::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent)
{
    VkBuffer vertexBuffers[] = {chunkMesh.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, chunkMesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    PushConstantC1 c1;
    c1.data = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
    c1.data[1][1] *= -1;

    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantC1), &c1);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &(descriptorSets[currentFrame]), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(chunkMesh.indices.size()), 1, 0, 0, 0);
}

void Chunk::cleanup()
{
    chunkMesh.cleanup();
}

Voxel *Chunk::getVoxelChunkLocal(BlockCoordinates coords)
{
    // Check bounds first
    if (coords.x < 0 || coords.x >= 16 ||
        coords.y < 0 || coords.y >= 16 ||
        coords.z < 0 || coords.z >= 256)
        return nullptr;

    int idx = coords.x + coords.y * 16 + coords.z * 16 * 16;
    return &voxels[idx];
}

Chunk::~Chunk()
{
}
