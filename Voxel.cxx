#include "Voxel.h"

Voxel::Voxel(VulkanContext &vkContext, BlockType blockType) : vkContext{vkContext}
{
    this->blockType = blockType;
}

void Voxel::setType(BlockType blockType)
{
    this->blockType = blockType;
    switch (blockType)
    {
    case AIR:
        faceTexture = BlockFaces::corruptFaceTexture;
        break;
    case GRASS:
        faceTexture = BlockFaces::grassFaceTexture;
        break;
    case DIRT:
        faceTexture = BlockFaces::dirtFaceTexture;
        break;
    case STONE:
        faceTexture = BlockFaces::stoneFaceTexture;
        break;
    
    default:
        faceTexture = BlockFaces::corruptFaceTexture;
        break;
    }
}

void Voxel::setPosition(glm::vec3 position)
{
    this->transform.position = position;
}

void Voxel::setRotation(glm::vec3 rotation)
{
    this->transform.rotation = rotation;
}

void Voxel::setScale(glm::vec3 scale)
{
    this->transform.scale = scale;
}

BlockType Voxel::getBlockType()
{
    return BlockType(blockType);
}



Voxel::~Voxel()
{
}
