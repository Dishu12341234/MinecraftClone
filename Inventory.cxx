#include "Inventory.h"
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>

Inventory::Inventory(VulkanContext &vkContext)
    : vkContext(vkContext), inventoryComponent(vkContext) {
  glm::vec2 size;
  float screenAspect = 3.0f / 2.0f;
  float texAspect = 270.0f / 162.0f;

  if (texAspect > screenAspect) {
    // fit width
    size.x = 3.0f;
    size.y = 3.0f / texAspect;
  } else {
    // fit height
    size.y = 2.0f;
    size.x = 2.0f * texAspect;
  }

  inventoryComponent.initUIComponent(glm::vec2(0),
                                     glm::vec2(1.5f * 1.5, 1.f * 1.5));
}

UIComponents *Inventory::getComponentPointer() { return &inventoryComponent; }

Inventory::~Inventory() {}
