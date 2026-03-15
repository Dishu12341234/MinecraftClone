#include "Camera.h"
#include "HelloTriangleApplication.hpp"
#include "Ray.h"
#include <iostream>

std::ostream &operator<<(std::ostream &os, const glm::vec3 &v)
{
    os << "vec3{x=" << v.x << ", y=" << v.y << ", z=" << v.z << "}";
    return os;
}

Camera::Camera(VulkanContext &vkContext, GameObjectPool &gop)
    : gameObjectPool{gop}, vkContext{vkContext}, cameraRay{vkContext, gop}
{
}
void Camera::updateUBO(UniformBufferObject &UBO,
                       VkExtent2D &swapChainExtent,
                       Event &event)
{
    static double lastX = event.mouseX;
    static double lastY = event.mouseY;

    float dt = event.dt * 0.001f;

    double dx = lastX - event.mouseX;
    double dy = lastY - event.mouseY;

    lastX = event.mouseX;
    lastY = event.mouseY;

    float sensitivity = 0.1f;

    yaw += dx * sensitivity;
    pitch += dy * sensitivity;
    pitch = glm::clamp(pitch, -89.9f, 89.9f);

    // Z-up forward vector from yaw/pitch
    forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    forward.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    forward.z = sin(glm::radians(pitch));
    forward = glm::normalize(forward);

    glm::vec3 worldUp(0.0f, 0.0f, 1.0f); // Z-up

    glm::vec3 forwardFlat = glm::normalize(glm::vec3(forward.x, forward.y, 0.0f));
    glm::vec3 right = glm::normalize(glm::cross(worldUp, forwardFlat));

    float speed = 15.81f * dt;

    if (event.getKeyPressed(GLFW_KEY_W))
        cameraPos += forwardFlat * speed;
    if (event.getKeyPressed(GLFW_KEY_S))
        cameraPos -= forwardFlat * speed;
    if (event.getKeyPressed(GLFW_KEY_A))
        cameraPos -= -right * speed;
    if (event.getKeyPressed(GLFW_KEY_D))
        cameraPos += -right * speed;
    if (event.getKeyPressed(GLFW_KEY_SPACE))
        cameraPos.z += speed;
    if (event.getKeyPressed(GLFW_KEY_LEFT_SHIFT))
        cameraPos.z -= speed;

    UBO.view = glm::lookAt(cameraPos, cameraPos + forward, worldUp);

    UBO.proj = glm::perspective(
        glm::radians(45.0f),
        swapChainExtent.width / (float)swapChainExtent.height,
        0.1f,
        1000.0f);

    UBO.proj[1][1] *= -1;
}

// Camera space: 1 unit = 10 voxels
glm::vec3 Camera::gePositionInWorldCoords() { return cameraPos * 10.f; }

void Camera::cleanup()
{
    cameraRay.cleanup();
}

void Camera::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent)
{

    cameraRay.transform.position = gePositionInWorldCoords() / 10.f;
    cameraRay.transform.position.z -= .5f; // lower the ray a bit so it doesn't clip with the camera
    cameraRay.transform.rotation = glm::vec3(0.f, glm::radians(-pitch), glm::radians(yaw));
    cameraRay.draw(commandBuffer, pipelineLayout, graphicsPipeline, descriptorSets, currentFrame, swapChainExtent);
}

Camera::~Camera() {}
