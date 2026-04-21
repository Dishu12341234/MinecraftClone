#pragma once
#include "PassInfo.hpp"
#include "Structs.h"
#include "UIRenderPipeline.h"

class UIComponents {
private:
  VulkanContext &vkContext;
  glm::vec2 position{0.f, 0.f};
  glm::vec2 size{1.f, 1.f};

  // Vertices and stuff
  std::vector<UIVertex> vertices;
  std::vector<uint32_t> indices;
  VkBuffer vertexBuffer = VK_NULL_HANDLE;
  VkBuffer indexBuffer = VK_NULL_HANDLE;

  VkBuffer StagingVertexBuffer = VK_NULL_HANDLE;
  VkBuffer StagingIndexBuffer = VK_NULL_HANDLE;

  VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
  VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

  VkDeviceMemory StagingVertexBufferMemory = VK_NULL_HANDLE;
  VkDeviceMemory StagingIndexBufferMemory = VK_NULL_HANDLE;

  VkDeviceSize stagingVertexBufferSize = 0;
  VkDeviceSize stagingIndexBufferSize = 0;

  void *stagingIndexBufferData;
  void *stagingVertexBufferData;

  void createVertexBuffer();
  void createIndexBuffer();

  int textureIDX{15};
  int instanceCount{1};

  float espilon_dz = 0;

public:
  UIComponents(VulkanContext &vkContext);

  void initUIComponent(glm::vec2 position = glm::vec2{0.f, 0.f},
                       glm::vec2 size = glm::vec2{1.f, 1.f});

  void setTextureIDX(int idx);
  void setInstanceCount(int idx);
  void setEpsilonFactor_dz(int edz);

  void draw(DrawInfo &drawInfo, PushConstantC2 &c2);

  void cleanup();
  ~UIComponents();
};
