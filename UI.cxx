#include "UI.h"
#include <iostream>

UI::UI(VulkanContext &vkContext) : vkContext{vkContext}
{
}

void UI::attachComponent(UIComponents &component)
{
    this->components.push_back(component);
}

void UI::render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, PushConstantC1 c1)
{
    for (auto &&component : components)
    {
        component.draw(commandBuffer, pipelineLayout, graphicsPipeline, descriptorSets, currentFrame, swapChainExtent, c1);
    }
}

void UI::cleanup()
{
    for (auto &component : components)
    {
        component.cleanup();
    }
}

UI::~UI()
{
}
