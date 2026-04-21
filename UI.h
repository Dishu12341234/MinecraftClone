#pragma once
#include "Structs.h"
#include "PassInfo.hpp"
#include "UIComponents.h"
#include <vector>
#include <deque>


class UI
{
private:
    VulkanContext &vkContext;
    std::deque<UIComponents *> components;
    public:

    UI(const UI &) = delete;
    UI &operator=(const UI &other)
    {
        return *this;
    }

    UI(UI &&) = default;
    UI &operator=(UI &&) = default;

    UI(VulkanContext &vkContext);

    void attachComponent(UIComponents *component);

    void render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
              std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, PushConstantC2 c2);
    void renderAt(DrawInfo &drawInfo, PushConstantC2 c2, uint32_t idx);
    void cleanup();
    ~UI();
};
