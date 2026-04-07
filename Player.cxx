#include "Player.h"
#include "Camera.h"
#include "Structs.h"
#include <cmath>
#include <iostream>

Player::Player(VulkanContext &vkContext, GameObjectPool &gop)
    : gameObjectPool{gop} {
  this->camera = std::move(Camera(vkContext, gop));
}

Player::~Player() {}

void Player::drawUIIfPossible(VkCommandBuffer commandBuffer,
                              VkPipelineLayout pipelineLayout,
                              VkPipeline graphicsPipeline,
                              std::vector<VkDescriptorSet> &descriptorSets,
                              uint32_t currentFrame,
                              VkExtent2D &swapChainExtent, UI &ui) {
  if (playerState.inInventory)
    camera->drawUIAt(commandBuffer, pipelineLayout, graphicsPipeline,
                     descriptorSets, currentFrame, swapChainExtent, ui, 0);

  camera->drawUIAt(commandBuffer, pipelineLayout, graphicsPipeline,
                   descriptorSets, currentFrame, swapChainExtent, ui, 1);
  camera->drawUIAt(commandBuffer, pipelineLayout, graphicsPipeline,
                   descriptorSets, currentFrame, swapChainExtent, ui, 2);
}

void Player::handlePlayerMovement(UniformBufferObject &UBO,
                                  VkExtent2D &swapChainExtent, Event &event) {
  static float deltaJZ = 0;

  double dx = 0, dy = 0, dz = 0;

  selfTransform.position   = camera->cameraPos;
  selfTransform.position.z += .2f;

  aabb.max = selfTransform.position + glm::vec3(.25f, .25f,  .0f);
  aabb.min = selfTransform.position - glm::vec3(.25f, .25f, 1.8f);

  AxisAlignedBoundingBox groundAABB{};
  groundAABB.max = selfTransform.position + glm::vec3(.1f, .1f,  .0f);
  groundAABB.min = selfTransform.position - glm::vec3(.1f, .1f, 1.8f);

  // ── ground check ────────────────────────────────────────────────────────
  playerState.onGround = false;
  {
    int feet = floor(groundAABB.min.z);
    int head = floor(groundAABB.max.z);
    int minX = floor(groundAABB.min.x);
    int maxX = floor(groundAABB.max.x);
    int minY = floor(groundAABB.min.y);
    int maxY = floor(groundAABB.max.y);

    for (int k = feet; k <= head && !playerState.onGround; k++)
      for (int i = minX; i <= maxX && !playerState.onGround; i++)
        for (int j = minY; j <= maxY; j++) {
          auto v = gameObjectPool.getVoxelGlobal({i, j, k});
          if (v && v->blockType != AIR) { playerState.onGround = true; break; }
        }
  }

  // ── gravity / jump ───────────────────────────────────────────────────────
  if (!playerState.onGround && !playerState.inJump)
    dz -= .01f * event.dt;
  else if (playerState.inJump) {
    deltaJZ -= .003f * event.dt;
    dz      += .01f  * event.dt * deltaJZ;
  }

  // ── desired XY movement ──────────────────────────────────────────────────
  float x_fac    = cos(glm::radians(camera->yaw));
  float y_fac    = sin(glm::radians(camera->yaw));
  float x_fac_dx = x_fac * camera->speed * event.dt;
  float y_fac_dy = y_fac * camera->speed * event.dt;

  // Update facing direction for animation / other systems — NOT used for collision
  bool xDominant = std::abs(x_fac) > std::abs(y_fac);
  if      ( x_fac > 0 &&  xDominant) facingDirection = NORTH;
  else if ( x_fac < 0 &&  xDominant) facingDirection = SOUTH;
  else if ( y_fac > 0 && !xDominant) facingDirection = WEST;
  else if ( y_fac < 0 && !xDominant) facingDirection = EAST;

  bool KEY_W = event.getKeyPressed(GLFW_KEY_W);
  bool KEY_S = event.getKeyPressed(GLFW_KEY_S);
  bool KEY_A = event.getKeyPressed(GLFW_KEY_A);
  bool KEY_D = event.getKeyPressed(GLFW_KEY_D);

  if (KEY_W) { dx += x_fac_dx; dy += y_fac_dy; }
  if (KEY_S) { dx -= x_fac_dx; dy -= y_fac_dy; }
  if (KEY_A) { dx += -y_fac_dy; dy +=  x_fac_dx; }
  if (KEY_D) { dx -= -y_fac_dy; dy -=  x_fac_dx; }

  // ── axis-separated collision ─────────────────────────────────────────────
  // Helper: does the AABB at `testPos` overlap any solid voxel?
  auto overlaps = [&](glm::vec3 testPos) -> bool {
    glm::vec3 tMax = testPos + glm::vec3(.25f, .25f,  .0f);
    glm::vec3 tMin = testPos - glm::vec3(.25f, .25f, 1.8f);

    int x0 = floor(tMin.x), x1 = floor(tMax.x);
    int y0 = floor(tMin.y), y1 = floor(tMax.y);
    int z0 = floor(tMin.z) + 1, z1 = floor(tMax.z);

    for (int k = z0; k <= z1; k++)
      for (int i = x0; i <= x1; i++)
        for (int j = y0; j <= y1; j++) {
          auto v = gameObjectPool.getVoxelGlobal({i, j, k});
          if (v && v->blockType != AIR) return true;
        }
    return false;
  };

  glm::vec3 base = selfTransform.position;

  if (overlaps(base + glm::vec3(dx, 0, 0)))
    dx = 0;

  if (overlaps(base + glm::vec3(0, dy, 0)))
    dy = 0;

  if (overlaps(base + glm::vec3(0, 0, dz)))
    dz = 0;

  if (playerState.onGround) [[likely]]
    playerState.inJump = false;

  if (event.getKeyPressed(GLFW_KEY_SPACE) && playerState.onGround) {
    deltaJZ = 1.f;
    playerState.inJump = true;
  }

  camera->cameraPos.x += dx;
  camera->cameraPos.y += dy;
  camera->cameraPos.z += dz;
  camera->updateUBO(UBO, swapChainExtent, event);
}

const int Player::getHealthPoints() { return healtPoints; }
