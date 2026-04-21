#include "Inventory.h"
#include "Event.h"
#include "Structs.h"
#include <fmt/base.h>
#include <fmt/core.h>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <numeric>
#include <ranges>

ApplicationDimensions Inventory::dimensions;

long double operator""_px_to_u(unsigned long long int px) {
  return px *
         (float(Inventory::dimensions.cw) / float(Inventory::dimensions.ww));
}

auto getHotbarCell = [](InventoryLayout &inventoryLayout,
                        glm::vec2 texture_start, glm::vec2 texture_end,
                        Event &event) {
  float mouseX = event.mouseX;
  float mouseY = event.mouseY;
  int px =
      int(mouseX - inventoryLayout.hotbar.leftMargin - texture_start.x) %
      int(inventoryLayout.hotbar.boxWidth + inventoryLayout.hotbar.box_boxGap);
  bool hoveringBox =
      (px < inventoryLayout.hotbar.boxWidth) &&
      (mouseY < (texture_end.y - inventoryLayout.hotbar.bottomMargin) &&
       (mouseY > (texture_end.y - inventoryLayout.hotbar.bottomMargin -
                  inventoryLayout.hotbar.boxHeight)));

  if (mouseX >= texture_start.x && mouseX <= texture_end.x &&
      mouseY >= texture_start.y && mouseY <= texture_end.y) {

    if (hoveringBox) {
      int sx = mouseX - inventoryLayout.hotbar.leftMargin - texture_start.x;
      int cx = floor(int(sx / (inventoryLayout.hotbar.boxWidth +
                               inventoryLayout.hotbar.box_boxGap)));
      return cx;
    }
  }
  return -1;
};

auto getInnerInventoryCell = [](InventoryLayout &inventoryLayout,
                                glm::vec2 texture_start, glm::vec2 texture_end,
                                Event &event) {
  float mouseX = event.mouseX;
  float mouseY = event.mouseY;

  int numCols = inventoryLayout.hotbar.slots.size();
  int numRows = inventoryLayout.innerSlots.slots.size() / numCols;

  int mx =
      int(mouseX - inventoryLayout.innerSlots.leftMargin - texture_start.x);

  int px = mx % int(inventoryLayout.innerSlots.boxWidth +
                    inventoryLayout.innerSlots.box_boxGap.x);

  int my =
      int(mouseY - texture_end.y + inventoryLayout.innerSlots.bottomMargin +
          (inventoryLayout.innerSlots.boxHeight +
           inventoryLayout.innerSlots.box_boxGap.y) *
              numRows -
          inventoryLayout.innerSlots.box_boxGap.y);
  int py = my % int(inventoryLayout.innerSlots.boxHeight +
                    inventoryLayout.innerSlots.box_boxGap.y);

  int row = std::floor(my / (inventoryLayout.innerSlots.boxHeight +
                             inventoryLayout.innerSlots.box_boxGap.y));
  int col = std::floor(mx / (inventoryLayout.innerSlots.boxWidth +
                             inventoryLayout.innerSlots.box_boxGap.x));

  int idx = col + numCols * row;

  if (py >= inventoryLayout.innerSlots.boxHeight)
    return -1;

  if (px >= inventoryLayout.innerSlots.boxWidth)
    return -1;

  if (idx < 0)
    return -1;

  if (idx > inventoryLayout.innerSlots.slots.size() - 1)
    return -1;

  return idx;
};

Inventory::Inventory(VulkanContext &vkContext,
                     ApplicationDimensions &dimensions)
    : vkContext(vkContext), inventoryComponent(vkContext),
      filterComponent(vkContext) {
  // the image is 270*180
  Inventory::dimensions = dimensions;
  inventoryLayout.originalTextureWidth = 270;
  inventoryLayout.originalTextureHeight = 180;

  float aspect = float(dimensions.fbw) / float(dimensions.fbh);

  // the way orthographic projection is set up
  //  the canonical view port extends from - aspect to + aspect

  inventoryComponent.setTextureIDX(0);
  inventoryComponent.initUIComponent(
      glm::vec2(0), glm::vec2(dimensions.cw * scale, dimensions.ch * scale));

  width_px = dimensions.ww * scale;
  height_px = dimensions.wh * scale;

  inventoryLayout.textureScaling.x =
      width_px / inventoryLayout.originalTextureWidth;
  inventoryLayout.textureScaling.y =
      height_px / inventoryLayout.originalTextureHeight;

  texture_start.x = abs(dimensions.ww - width_px) / 2.f;
  texture_end.x = abs(dimensions.ww + width_px) / 2.f;

  texture_start.y = abs(dimensions.wh - height_px) / 2.f;
  texture_end.y = abs(dimensions.wh + height_px) / 2.f;

  fmt::println("texture_start.x = {}", texture_start.x);
  fmt::println("texture_end.x   = {}", texture_end.x);

  fmt::println("texture_start.y = {}", texture_start.y);
  fmt::println("texture_end.y   = {}", texture_end.y);

  fmt::println("textureScaling: ({},{})", inventoryLayout.textureScaling.x,
               inventoryLayout.textureScaling.y);

  inventoryLayout.hotbar.leftMargin = 4 * inventoryLayout.textureScaling.x;
  inventoryLayout.hotbar.bottomMargin = 4 * inventoryLayout.textureScaling.y;
  inventoryLayout.hotbar.box_boxGap = 2 * inventoryLayout.textureScaling.x;
  inventoryLayout.hotbar.boxWidth = 20 * inventoryLayout.textureScaling.x;
  inventoryLayout.hotbar.boxHeight = 20 * inventoryLayout.textureScaling.y;

  inventoryLayout.innerSlots.leftMargin = 4 * inventoryLayout.textureScaling.x;
  inventoryLayout.innerSlots.bottomMargin =
      30 * inventoryLayout.textureScaling.y;
  inventoryLayout.innerSlots.box_boxGap = glm::vec2(2.f);
  inventoryLayout.innerSlots.box_boxGap.x *= inventoryLayout.textureScaling.x;
  inventoryLayout.innerSlots.box_boxGap.y *= inventoryLayout.textureScaling.y;
  inventoryLayout.innerSlots.boxWidth = 20 * inventoryLayout.textureScaling.x;
  inventoryLayout.innerSlots.boxHeight = 20 * inventoryLayout.textureScaling.y;

  filterComponent.setTextureIDX(3);
  filterComponent.setEpsilonFactor_dz(1);
  filterComponent.initUIComponent(
      glm::vec2(0), glm::vec2(inventoryLayout.hotbar.boxWidth * 1_px_to_u,
                              inventoryLayout.hotbar.boxHeight * 1_px_to_u));

  // hotbar
  for (auto [i, slot] : std::views::enumerate(inventoryLayout.hotbar.slots)) {
    slot.bb.min.x = texture_start.x + inventoryLayout.hotbar.leftMargin +
                    i * (inventoryLayout.hotbar.box_boxGap +
                         inventoryLayout.hotbar.boxWidth);
    slot.bb.max.x = slot.bb.min.x + inventoryLayout.hotbar.boxWidth;

    slot.bb.min.y = texture_end.y - inventoryLayout.hotbar.bottomMargin -
                    inventoryLayout.hotbar.boxHeight;
    slot.bb.max.y = slot.bb.min.y + inventoryLayout.hotbar.boxHeight;

    fmt::println("X : ({},{}) | Y : ({},{})", slot.bb.min.x, slot.bb.max.x,
                 slot.bb.min.y, slot.bb.max.y);
  }

  int numCols = inventoryLayout.hotbar.slots.size();
  int numRows = inventoryLayout.innerSlots.slots.size() / numCols;

  for (int o = 0; o < numRows; o++) {
    for (int i = 0; i < numCols; i++) {
      int idx = i + numCols * o;
      Slot &slot = inventoryLayout.innerSlots.slots[idx];

      slot.bb.min.x = texture_start.x + inventoryLayout.innerSlots.leftMargin +
                      i * (inventoryLayout.innerSlots.box_boxGap.x +
                           inventoryLayout.innerSlots.boxWidth);
      slot.bb.max.x = slot.bb.min.x + inventoryLayout.innerSlots.boxWidth;

      slot.bb.min.y = texture_end.y - inventoryLayout.innerSlots.bottomMargin -
                      (inventoryLayout.innerSlots.boxHeight +
                       inventoryLayout.innerSlots.box_boxGap.y) *
                          numRows +
                      (inventoryLayout.innerSlots.boxHeight) * o +
                      inventoryLayout.innerSlots.box_boxGap.y * (o + 1);
      slot.bb.max.y = slot.bb.min.y + inventoryLayout.innerSlots.boxHeight;
    }
  }
}

UIComponents *Inventory::getInventoryComponentPointer() {
  return &inventoryComponent;
}
UIComponents *Inventory::getFilterComponentPointer() {
  return &filterComponent;
}


void Inventory::inventoryUpdates(Event &event) {
  float mouseX = event.mouseX;
  float mouseY = event.mouseY;
  voffset.x = dimensions.cw;
  voffset.y = dimensions.ch;

  fmt::println("voff_x: {}", voffset.x);

  int hotbarCell =
      getHotbarCell(inventoryLayout, texture_start, texture_end, event);
  int innverInventoryCell =
      getInnerInventoryCell(inventoryLayout, texture_start, texture_end, event);

  if (hotbarCell < 0 && innverInventoryCell < 0)
    return;

  BoundingBox bb =
      hotbarCell < 0 ? inventoryLayout.innerSlots.slots[innverInventoryCell].bb
                     : inventoryLayout.hotbar.slots[hotbarCell].bb;

  voffset = bb.max + bb.min;
  voffset /= 2.f;
  voffset.x -= dimensions.fbw / 2.f;
  voffset.y -= dimensions.fbh / 2.f;

  voffset.x /= dimensions.fbw;
  voffset.y /= dimensions.fbh;

  voffset.x *= dimensions.cw;
  voffset.y *= dimensions.ch;

  fmt::println("vx: {} ", voffset.x);
}

void Inventory::drawUI(DrawInfo &drawInfo, std::shared_ptr<Camera> playerCamera,
                       UI &ui) {
  if (playerState->inInventory) {
    playerCamera->drawUIAt(drawInfo, ui, 0);
    playerCamera->drawUIAt(drawInfo, ui, 1, glm::vec3(voffset, 0));
  }
}

Inventory::~Inventory() {}
