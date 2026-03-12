#pragma once
#include "PassInfo.hpp"
#include "Structs.h"

class Voxel
{
private:
    VulkanContext &vkContext;
    Transform transform;
    int blockType = AIR;

public:
    Voxel(VulkanContext &vkContext, BlockType blockType = AIR);

    void setType(BlockType blockType);
    void setPosition(glm::vec3 position);
    void setRotation(glm::vec3 rotation);
    void setScale(glm::vec3 scale);
    BlockType getBlockType();

    ~Voxel();
};
