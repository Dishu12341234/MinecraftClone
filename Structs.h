#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace BlockFaces {
inline int grassFaceTexture[6] = {0, 1, 1, 1, 1, 1};
inline int dirtFaceTexture[6] = {1, 1, 1, 1, 1, 1};
inline int woodFaceTexture[6] = {3, 3, 3, 3, 3, 3};
inline int leavesFaceTexture[6] = {4, 4, 4, 4, 4, 4};
inline int stoneFaceTexture[6] = {5, 5, 5, 5, 5, 5};
inline int bedrockFaceTexture[6] = {6, 6, 6, 6, 6, 6};
inline int corruptFaceTexture[6] = {19, 19, 19, 19, 19, 19};
}; // namespace BlockFaces

enum FaceDirection { TOP = 0, BOTTOM, LEFT, RIGHT, FRONT, BACK };

enum CardinalFacing { EAST = 0, WEST, NORTH, SOUTH };

// UBO
struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct DrawInfo {
  VkCommandBuffer commandBuffer;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;
  std::vector<VkDescriptorSet> &descriptorSets;
  uint32_t currentFrame;
  VkExtent2D &swapChainExtent;
};

// PCS
struct PushConstantC1 {
  glm::mat4 data;
};

struct PushConstantC2 {
  glm::mat4 model;
  glm::mat4 proj;
};

struct Transform {
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
  glm::quat rotationQuat; // Added for ray orientation
};

enum BlockType {
  AIR=0,
  GRASS=1,
  DIRT=2,
  WOOD=3,
  LEAF=4,
  STONE=5,
  BEDROCK=6,
  END_BLOCK_TYPE
};

struct PlayerState {
  bool onGround{false};
  bool frontBlock{false};
  bool backBlock{false};
  bool leftBlock{false};
  bool rightBlock{false};
  bool topBlock{false};
  bool inGUI{false};
  bool inInventory{false};
  bool inJump{false};
};

struct FaceDef {
  glm::vec3 corners[4]; // vertex positions relative to block origin
  int tileOffset;
  float light;
  int dx, dy, dz; // direction toward the neighbour
};

struct BlockCoordinates {
  int x{0}, y{0}, z{0};
  bool operator==(const BlockCoordinates &other) const {
    return x == other.x && y == other.y && z == other.z;
  }
};

struct Faces {
  bool top;
  bool bottom;
  bool right;
  bool left;
  bool front;
  bool back;
};

struct AxisAlignedBoundingBox {
  glm::vec3 min;
  glm::vec3 max;
};

struct UnloadedVoxelPayload {
  BlockCoordinates blockCoordinates_Global;
  BlockType blockType;
  uint64_t chunkKey;
};

struct ApplicationDimensions {
  int fbw = 0; // frame buffer width
  int fbh = 0; // frame buffer height

  int ww = 0; // window width
  int wh = 0; // window height

  int cw = 0; // Canonical width
  int ch = 0; // Canonical Height
};
