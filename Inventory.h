#pragma once

#include "PassInfo.hpp"
#include "UIComponents.h"
#include <memory>

class Inventory {
private:
  VulkanContext &vkContext;
  UIComponents inventoryComponent;

public:
  Inventory(VulkanContext &vkContext);

  UIComponents *getComponentPointer();

  ~Inventory();
};
