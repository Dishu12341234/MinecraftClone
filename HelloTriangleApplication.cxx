#include "HelloTriangleApplication.hpp"
#include "Camera.h"
#include "Ray.h"

void HelloTriangleApplication::mainLoop()
{
    while (!glfwWindowShouldClose(_window))
    {
        event->eventLoop();
        drawFrame();
    }
}

void HelloTriangleApplication::drawFrame()
{
    // 1️⃣ Wait for previous frame to finish
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    // 2️⃣ Acquire an image from the swapchain
    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(
        device,
        swapChain,
        UINT64_MAX,
        imageAvailableSemaphores[currentFrame], // still per-frame
        VK_NULL_HANDLE,
        &imageIndex);

    if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // 3️⃣ Reset the command buffer for this frame
    vkResetCommandBuffer(commandBuffers[currentFrame], 0); // or VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT

    terrain.value().handelDirtyChunks();
    // 4️⃣ Record drawing commands into the command buffer
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    updateUniformBuffer(currentFrame);


    // 5️⃣ Submit the command buffer to the graphics queue
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    // ✅ Use renderFinishedSemaphore per swapchain image
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[imageIndex]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    // 6️⃣ Present the image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(presentQueue, &presentInfo);

    // 7️⃣ Advance frame
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangleApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.graphicsPipeline);
    terrain.value().draw(commandBuffer, graphicsPipeline.pipelineLayout, graphicsPipeline.graphicsPipeline, descriptorSets, currentFrame, swapChainExtent);
    
    
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rayGraphicsPipeline.graphicsPipeline);
    camera->draw(commandBuffer, rayGraphicsPipeline.pipelineLayout, rayGraphicsPipeline.graphicsPipeline, descriptorSets, currentFrame, swapChainExtent);
    // 
    // gameObjectPool.drawIndexed(commandBuffer, descriptorSets, graphicsPipeline, swapChainExtent, 3, currentFrame);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void HelloTriangleApplication::cleanup()
{
    vkDeviceWaitIdle(device);

    gameObjectPool.cleanUpResources();
    terrain.value().cleanup();
    camera->cleanup();

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    // Destroy per-frame semaphores and fences
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    // Destroy per-swapchain-image render-finished semaphores
    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++)
    {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    for (auto framebuffer : swapChainFramebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);

    graphicsPipeline.destroyPipelineLayout();
    rayGraphicsPipeline.destroyPipelineLayout();

    vkDestroyRenderPass(device, renderPass, nullptr);

    for (auto imageView : swapChainImageViews)
        vkDestroyImageView(device, imageView, nullptr);
    vkDestroyImageView(device, colorImageView, nullptr);
    vkDestroyImage(device, colorImage, nullptr);
    vkFreeMemory(device, colorImageMemory, nullptr);
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

    vkDestroySwapchainKHR(device, swapChain, nullptr);

    texture.destroy();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(_window);
    glfwTerminate();
}

HelloTriangleApplication::HelloTriangleApplication()
{
}

HelloTriangleApplication::HelloTriangleApplication(std::string processName)
{
    this->PROCESS_NAME = processName;
}

void HelloTriangleApplication::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

HelloTriangleApplication::~HelloTriangleApplication()
{
}
