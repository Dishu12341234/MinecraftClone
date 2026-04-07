#include "Camera.h"
#include "HelloTriangleApplication.hpp"
#include "Ray.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include <iostream>

std::ostream &operator<<(std::ostream &os, const glm::vec3 &v)
{
    os << "vec3{x=" << v.x << ", y=" << v.y << ", z=" << v.z << "}";
    return os;
}

void Camera::getHitInfo(HitInfo &hitInfo)

{
    float stepSize = 0.1f;

    glm::vec3 rayOriginCR = gePositionInWorldCoords();
    glm::vec3 rayDirCR = forwardCR;

    float maxDistanceCR = 5.0f; // Max ray distance

    for (float t = 0.0f; t < maxDistanceCR; t += stepSize)
    {
        glm::vec3 point = rayOriginCR + rayDirCR * t;
        BlockCoordinates blockCoords{
            static_cast<int>(floor(point.x)),
            static_cast<int>(floor(point.y)),
            static_cast<int>(floor(point.z))};

        Voxel *voxel = gameObjectPool.getVoxelGlobal(blockCoords);
        if (voxel && voxel->getBlockType() != AIR)
        {
            hitInfo.blockCoords = blockCoords;
            hitInfo.hitVoxel = voxel;

            break;
        }
    }
}

Camera::Camera(VulkanContext &vkContext, GameObjectPool &gop)
    : gameObjectPool{gop},
      vkContext{vkContext},
      cameraRay{vkContext, gop}

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
    if (yaw >= 360)
        yaw -= 360;

    if(yaw < 0)
        yaw += 360;
    // std::cout << "yaw: " << yaw << std::endl;
    pitch += dy * sensitivity;
    pitch = glm::clamp(pitch, -89.9f, 89.9f);

    // Z-up forward vector from yaw/pitch
    forwardCR.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    forwardCR.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    forwardCR.z = sin(glm::radians(pitch));
    forwardCR = glm::normalize(forwardCR);

    glm::vec3 worldUp(0.0f, 0.0f, 1.0f); // Z-up

    forwardFlat = glm::normalize(glm::vec3(forwardCR.x, forwardCR.y, 0.0f));
    right = glm::normalize(glm::cross(worldUp, forwardFlat));

    speed = .001f;

    v_velocity = 30;

    UBO.view = glm::lookAt(cameraPos, cameraPos + forwardCR, worldUp);

    UBO.proj = glm::perspective(
        glm::radians(90.0f),
        swapChainExtent.width / (float)swapChainExtent.height,
        0.025f,
        1000.0f);

    UBO.proj[1][1] *= -1;
}

glm::vec3 Camera::gePositionInWorldCoords() { return cameraPos; }

void Camera::cleanup()
{
    cameraRay.cleanup();
}

void Camera::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent)
{

    cameraRay.transform.position = gePositionInWorldCoords();
    // cameraRay.transform.position.z -= .1f;
    cameraRay.direction = forwardCR;
    cameraRay.draw(commandBuffer, pipelineLayout, graphicsPipeline, descriptorSets, currentFrame, swapChainExtent);
}

void Camera::drawUI(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                    VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets,
                    uint32_t currentFrame, VkExtent2D &swapChainExtent, UI &ui)
{
    PushConstantC1 c1;

    float aspect = (float)swapChainExtent.width / (float)swapChainExtent.height;
    c1.data = glm::ortho(-aspect, aspect, -1.0f, 1.0f, 0.0f, 1.0f);

    ui.render(commandBuffer, pipelineLayout, graphicsPipeline,
              descriptorSets, currentFrame, swapChainExtent, c1);
}

void Camera::drawUIAt(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, UI &ui, uint32_t idx)
{
    PushConstantC1 c1;

    float aspect = (float)swapChainExtent.width / (float)swapChainExtent.height;
    c1.data = glm::ortho(-aspect, aspect, -1.0f, 1.0f, 0.0f, 1.0f);

    ui.renderAt(commandBuffer, pipelineLayout, graphicsPipeline,
                descriptorSets, currentFrame, swapChainExtent, c1, idx);
}

Camera::~Camera()
{
}
