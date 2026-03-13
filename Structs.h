#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

// UBO
struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

// PCS
struct PushConstantC1
{
    glm::mat4 data;
};

struct Transform
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

enum BlockType
{
    AIR = 0,
    GRASS,
    DIRT,
    STONE
};

struct FaceDef
{
    glm::vec3 corners[4]; // vertex positions relative to block origin
    glm::vec2 uvs[4];
    float light;
    int dx, dy, dz; // direction toward the neighbour
};

struct BlockCoordinates
{
    int x{0}, y{0}, z{0};
    bool operator==(const BlockCoordinates &other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }
};