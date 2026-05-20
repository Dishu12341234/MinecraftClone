#ifndef FONT_H
#define FONT_H

#include "FontRenderPipeline.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Mesh.h"
#include "PassInfo.hpp"
#include "Structs.h"
#include "Textures.hpp"

// ── Per-glyph metrics + atlas UV coords
// ───────────────────────────────────────
struct GlyphInfo {
  // Pixel dimensions of the glyph bitmap
  uint32_t bitmapW = 0, bitmapH = 0;

  // Bearing: offset from the pen origin to the top-left of the bitmap (px)
  int32_t bearingX = 0, bearingY = 0;

  // How far to advance the pen after drawing this glyph (px, in 1/64 units
  // → divide by 64)
  int32_t advanceX = 0;

  // UV coordinates inside the atlas texture  [0..1]
  float u0 = 0, v0 = 0; // top-left
  float u1 = 0, v1 = 0; // bottom-right
};
class Fonts {
private:
  VulkanContext &vkContext;
  FT_Library &ft;
  static constexpr uint32_t FIRST_CHAR = 32; // space
  static constexpr uint32_t LAST_CHAR = 126; // '~'
  static constexpr uint32_t NUM_CHARS = LAST_CHAR - FIRST_CHAR + 1;

  // Pixel size used when rasterising glyphs (change to taste)
  static constexpr uint32_t FONT_PIXEL_HEIGHT = 64;
  Mesh<FontVertex> mesh;
  u_Texture texture;
  std::array<GlyphInfo, NUM_CHARS> glyphs{};
  uint32_t atlasW = 0, atlasH = 0;
  friend class HelloTriangleApplication;

public:
  Fonts(VulkanContext &vkContext, FT_Library &ft);
  void createTextureImage();
  void cleanup();
  float buildTextMesh(const std::string &text, float screenScale);
  ~Fonts() = default;
};

#endif // !FONT_H
