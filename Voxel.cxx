#include "Voxel.h"
#include <glm/ext/vector_float3.hpp>

Voxel::Voxel(BlockType blockType) { this->blockType = blockType; }

void Voxel::setType(BlockType blockType) {
  this->blockType = blockType;
  switch (blockType) {
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

BlockType Voxel::getBlockType() { return BlockType(blockType); }

Voxel::~Voxel() {}
