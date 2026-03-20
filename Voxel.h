#pragma once
#include "PassInfo.hpp"
#include "Structs.h"



class Voxel
{
private:
    VulkanContext &vkContext;
    Transform transform;
    int blockType = AIR;
    const int* faceTexture = BlockFaces::corruptFaceTexture; // default to a bright pink texture to easily spot untextured blocks

    AxisAlignedBoundingBox aabb{};

    friend class Chunk;
    friend class Player;
    friend class Camera;
public:
    Voxel(VulkanContext &vkContext, BlockType blockType = AIR);

    void setType(BlockType blockType);
    void setPosition(glm::vec3 position);
    void setRotation(glm::vec3 rotation);
    void setScale(glm::vec3 scale);
    BlockType getBlockType();

    ~Voxel();
};
