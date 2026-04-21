#pragma once
#include "Camera.h"
#include "Event.h"
#include "PassInfo.hpp"
#include "Structs.h"
#include "UIComponents.h"
#include <memory>
#include <array>

struct BoundingBox {
  glm::vec2 min;
  glm::vec2 max;
};

struct Slot {
  int itemID{0};
  BoundingBox bb{};
};

struct InnerSlots {
  int leftMargin = 0;   // 4
  int bottomMargin = 0; // 4
  glm::vec2 box_boxGap{0};   // 2

  int boxWidth = 0;  // 20
  int boxHeight = 0; // 20
  std::array<Slot, 36> slots;
};

struct Hotbar {

  int leftMargin = 0;   // 4
  int bottomMargin = 0; // 4
  int box_boxGap = 0;   // 2

  int boxWidth = 0;  // 20
  int boxHeight = 0; // 20

  std::array<Slot, 12> slots;
};

struct InventoryLayout {
  int originalTextureWidth = 0;
  int originalTextureHeight = 0;

  glm::vec2 textureScaling{1};

  InnerSlots innerSlots;
  Hotbar hotbar;
};

class Inventory {
private:
  VulkanContext &vkContext;

  UIComponents inventoryComponent;
  UIComponents filterComponent;

  float scale = .75f;

  float width_px = 0;
  float height_px = 0;

  glm::vec2 texture_start{0};
  glm::vec2 texture_end{0};

  glm::vec2 voffset{};


  InventoryLayout inventoryLayout;

  PlayerState *playerState = nullptr;

public:
  static ApplicationDimensions dimensions;

  Inventory(VulkanContext &vkContext, ApplicationDimensions &dimensions);

  UIComponents *getInventoryComponentPointer();
  UIComponents *getFilterComponentPointer();

  void attachPlayerState(PlayerState *playerState) {
    this->playerState = playerState;
  };

  void inventoryUpdates(Event &event);
  void drawUI(DrawInfo &drawInfo, std::shared_ptr<Camera> playerCamera, UI &ui);

  ~Inventory();
};
