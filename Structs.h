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
    int tileOffset;
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

struct Faces
{
    bool top;
    bool bottom;
    bool right;
    bool left;
    bool front;
    bool back;
};

struct BlockFaces {
    static constexpr int grassFaceTexture[6]  = {0, 1, 1, 1, 1, 1};
    static constexpr int dirtFaceTexture[6]   = {1, 1, 1, 1, 1, 1};
    static constexpr int woodFaceTexture[6]   = {3, 3, 3, 3, 3, 3};
    static constexpr int leavesFaceTexture[6] = {4, 4, 4, 4, 4, 4};
    static constexpr int stoneFaceTexture[6]  = {5, 5, 5, 5, 5, 5};
    static constexpr int corruptFaceTexture[6]  = {19, 19, 19, 19, 19, 19};
};