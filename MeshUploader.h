#ifndef MESH_UPLOADER
#define MESH_UPLOADER

#include <vulkan/vulkan.h>
#include "GameMeshObject.h"

class MeshUploader {
public:
    static void upload(
        VulkanContext vkContext,
        GameMeshObject& mesh
    );
};

#endif