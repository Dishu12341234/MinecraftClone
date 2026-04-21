#include "UI.h"
#include <iostream>

UI::UI(VulkanContext &vkContext) : vkContext{vkContext} {}

void UI::attachComponent(UIComponents *component) {
  this->components.emplace_back(component);
}

[[deprecated(
    "use the ...At function to render each element indivisually")]] void
UI::render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
           VkPipeline graphicsPipeline,
           std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame,
           VkExtent2D &swapChainExtent, PushConstantC2 c2) {
  // for (auto &&component : components) {

  //   component->draw(commandBuffer, pipelineLayout, graphicsPipeline,
  //                   descriptorSets, currentFrame, swapChainExtent, c2);
  // }
}

void UI::renderAt(DrawInfo &drawInfo, PushConstantC2 c2, uint32_t idx) {
  if (idx >= components.size())
    return;

  UIComponents *component = components[idx];

  component->draw(drawInfo, c2);
}

void UI::cleanup() {
  for (auto &component : components) {
    component->cleanup();
  }
}

UI::~UI() {}
