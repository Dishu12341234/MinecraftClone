#include "Voxel.h"

Voxel::Voxel(BlockType blockType)
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
    case WOOD:
        faceTexture = BlockFaces::woodFaceTexture;
        break;
    case LEAF:
        faceTexture = BlockFaces::leavesFaceTexture;
        break;
    case STONE:
        faceTexture = BlockFaces::stoneFaceTexture;
        break;
    case BEDROCK:
        faceTexture = BlockFaces::bedrockFaceTexture;
        break;
    
    default:
        faceTexture = BlockFaces::corruptFaceTexture;
        break;
    }
}

void Voxel::setPosition(glm::vec3 position)
{
    this->transform.position = position;
    aabb.max = position + glm::vec3(.5f);
    aabb.min = position - glm::vec3(.5f);

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
