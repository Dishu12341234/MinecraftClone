#ifndef H_GPIPE
#define H_GPIPE
#include "PassInfo.hpp"
#include "Structs.h"
#include <array>

#include <fstream>
#include <functional>
#include <vector>

struct Vertex {
  glm::vec3 pos;
  glm::vec3 size;
  glm::vec2 texCoord;
  float brightness;

  // Telling vulkan how to setup binding
  // Struct 1/2
  static VkVertexInputBindingDescription getBindingDescription();
  // Struct 2/2
  static std::array<VkVertexInputAttributeDescription, 4>
  getAttributeDescriptions();

  bool operator==(const Vertex &other) const {
    return pos == other.pos && size == other.size && texCoord == other.texCoord;
  }
};

namespace std {
template <> struct hash<Vertex> {
  size_t operator()(Vertex const &vertex) const {
    size_t seed = 0;

    auto hashCombine = [&seed](auto const &v) {
      seed ^= std::hash<float>()(v) + 0x9e3779b97f4a7c15 + (seed << 6) +
              (seed >> 2);
    };

    hashCombine(vertex.pos.x);
    hashCombine(vertex.pos.y);
    hashCombine(vertex.pos.z);

    hashCombine(vertex.size.r);
    hashCombine(vertex.size.g);
    hashCombine(vertex.size.b);

    hashCombine(vertex.texCoord.x);
    hashCombine(vertex.texCoord.y);

    return seed;
  }
};
} // namespace std

class u_GraphicsPipeline {
private:
  VkDevice device;
  int height, width;
  VkExtent2D swapChainExtent;
  VkRenderPass renderPass;
  VkDescriptorSetLayout *descriptorSetLayout;
  VkSampleCountFlagBits msaaSamples;

  friend class RayGraphicsPipeline;
  friend class UIRenderPipeline;

public:
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline; // shared

  u_GraphicsPipeline();
  ~u_GraphicsPipeline();
  void destroyPipelineLayout();

  void
  u_PassGraphicsPipelineCreateInfo(u_GraphicsPipelineCreateInfo pCreateInfo);

  static std::vector<char> readFile(const std::string &filename);

  virtual void createGraphicsPipeline();
  VkShaderModule createShaderModule(const std::vector<char> &code);
};
#endif
