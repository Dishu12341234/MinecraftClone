#pragma once
#include "PassInfo.hpp"
#include "Structs.h"
#include <glm/ext/vector_float3.hpp>

class Voxel {
private:
  char blockType = AIR;
  // top , bottom, left, right, front, back
  const char *faceTexture =
      BlockFaces::corruptFaceTexture; // default to a bright pink texture to
                                      // easily spot untextured blocks

  bool visible = false;

  friend class Chunk;
  friend class Player;
  friend class Camera;

public:
  Voxel(BlockType blockType = AIR);

  void setType(BlockType blockType);
  void setScale(glm::vec3 scale);
  BlockType getBlockType();

  ~Voxel();
};
