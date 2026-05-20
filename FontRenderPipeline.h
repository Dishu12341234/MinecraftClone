#ifndef FONTRENDER_PIPELINE_H
#define FONTRENDER_PIPELINE_H

#include "GraphicsPipeline.h"

struct FontVertex {
  glm::vec3 pos;
  glm::vec2 uv;

  // Telling vulkan how to setup binding
  // Struct 1/2
  static VkVertexInputBindingDescription getBindingDescription();
  // Struct 2/2
  static std::array<VkVertexInputAttributeDescription, 2>
  getAttributeDescriptions();

  bool operator==(const FontVertex &other) const {
    return pos == other.pos && uv == other.uv;
  }
};

namespace std {
template <> struct hash<FontVertex> {
  size_t operator()(FontVertex const &vertex) const {
    size_t seed = 0;

    auto hashCombine = [&seed](auto const &v) {
      seed ^= std::hash<float>()(v) + 0x9e3779b97f4a7c15 + (seed << 6) +
              (seed >> 2);
    };

    hashCombine(vertex.pos.x);
    hashCombine(vertex.pos.y);
    hashCombine(vertex.pos.z);

    hashCombine(vertex.uv.r);
    hashCombine(vertex.uv.g);

    return seed;
  }
};
} // namespace std

class FontRenderPipeline : public u_GraphicsPipeline {
public:
  FontRenderPipeline() = default;
  void createGraphicsPipeline() override;
  ~FontRenderPipeline() = default;
}; // FontRenderPipeline

#endif
