#include "Fonts.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
Fonts::Fonts(VulkanContext &vkContext, FT_Library &ft)
    : vkContext(vkContext), mesh(vkContext), ft(ft) {}

// ─────────────────────────────────────────────────────────────────────────────
// createTextureImage
// ─────────────────────────────────────────────────────────────────────────────
void Fonts::createTextureImage() {

  const std::string fontPath = "/home/divyansh/MinecraftClone/fonts/arial.ttf";

  // ── 1. Load face ─────────────────────────────────────────────────────────
  FT_Face face;
  if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    throw std::runtime_error("FreeType: failed to load font: " + fontPath);

  FT_Set_Pixel_Sizes(face, 0, FONT_PIXEL_HEIGHT);

  // ── 2. First pass: measure atlas ─────────────────────────────────────────
  uint32_t totalWidth = 0;
  uint32_t maxHeight = 0;

  for (uint32_t c = FIRST_CHAR; c <= LAST_CHAR; ++c) {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      std::cerr << "FreeType: failed to load glyph '" << (char)c << "'\n";
      continue;
    }
    totalWidth += face->glyph->bitmap.width + 1;
    maxHeight = std::max(maxHeight, face->glyph->bitmap.rows);
  }

  atlasW = 1;
  while (atlasW < totalWidth)
    atlasW <<= 1;
  atlasH = 1;
  while (atlasH < maxHeight)
    atlasH <<= 1;

  // ── 3. CPU atlas (R8, zero-init) ─────────────────────────────────────────
  std::vector<uint8_t> atlasPixels(atlasW * atlasH, 0);

  // ── 4. Second pass: blit + record metrics ─────────────────────────────────
  uint32_t penX = 0;

  for (uint32_t c = FIRST_CHAR; c <= LAST_CHAR; ++c) {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
      continue;

    FT_GlyphSlot g = face->glyph;
    uint32_t bw = g->bitmap.width;
    uint32_t bh = g->bitmap.rows;
    int pitch = g->bitmap.pitch;

    for (uint32_t row = 0; row < bh; ++row) {
      uint8_t *src = g->bitmap.buffer + row * pitch;
      uint8_t *dest = atlasPixels.data() + row * atlasW + penX;
      memcpy(dest, src, bw);
    }

    GlyphInfo &info = glyphs[c - FIRST_CHAR];
    info.bitmapW = bw;
    info.bitmapH = bh;
    info.bearingX = g->bitmap_left;
    info.bearingY = g->bitmap_top;
    info.advanceX = static_cast<int32_t>(g->advance.x);

    info.u0 = static_cast<float>(penX) / static_cast<float>(atlasW);
    info.v0 = 0.0f;
    info.u1 = static_cast<float>(penX + bw) / static_cast<float>(atlasW);
    info.v1 = static_cast<float>(bh) / static_cast<float>(atlasH);

    penX += bw + 1;
  }

  FT_Done_Face(face);

  // ── 5. Upload atlas ───────────────────────────────────────────────────────
  VkDeviceSize imageSize = static_cast<VkDeviceSize>(atlasW) * atlasH;

  texture.mipLevels =
      static_cast<uint32_t>(std::floor(std::log2(std::max(atlasW, atlasH)))) +
      1;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingMemory;
  texture.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       stagingBuffer, stagingMemory);

  void *mapped;
  vkMapMemory(texture.device, stagingMemory, 0, imageSize, 0, &mapped);
  memcpy(mapped, atlasPixels.data(), static_cast<size_t>(imageSize));
  vkUnmapMemory(texture.device, stagingMemory);

  texture.createImage(atlasW, atlasH, VK_SAMPLE_COUNT_1_BIT, texture.mipLevels,
                      VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                          VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                          VK_IMAGE_USAGE_SAMPLED_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.textureImage,
                      texture.textureImageMemory);

  texture.transitionImageLayout(
      texture.textureImage, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mipLevels);

  texture.copyBufferToImage(stagingBuffer, texture.textureImage, atlasW,
                            atlasH);
  texture.generateMipmaps(texture.textureImage, VK_FORMAT_R8_UNORM, atlasW,
                          atlasH, texture.mipLevels);

  vkDestroyBuffer(texture.device, stagingBuffer, nullptr);
  vkFreeMemory(texture.device, stagingMemory, nullptr);

  // ── 6. Image view — R8_UNORM with R swizzled into all channels ────────────
  //
  //  CRITICAL FIX: u_Texture::createTextureView() hard-codes
  //  VK_FORMAT_R8G8B8A8_SRGB. That format does NOT match this R8_UNORM image,
  //  causing a Vulkan validation error and a black/garbage texture.  We build
  //  the view manually here.
  //
  //  The component swizzle maps the single R channel into all four output
  //  channels so sampling .r in the shader always returns the greyscale
  //  coverage.
  //
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = texture.textureImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8_UNORM;

  viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_R;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_R;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_R;

  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = texture.mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(texture.device, &viewInfo, nullptr,
                        &texture.textureImageView) != VK_SUCCESS)
    throw std::runtime_error("Fonts: failed to create atlas image view!");
}

// ─────────────────────────────────────────────────────────────────────────────
// buildTextMesh
// ─────────────────────────────────────────────────────────────────────────────
float Fonts::buildTextMesh(const std::string &text, float screenScale) {

  const float px2ws = screenScale / static_cast<float>(FONT_PIXEL_HEIGHT);

  std::vector<FontVertex> vertices;
  std::vector<uint32_t> indices;
  vertices.reserve(text.size() * 4);
  indices.reserve(text.size() * 6);

  float penX = 0.0f;

  for (char ch : text) {
    uint32_t c = static_cast<unsigned char>(ch);

    if (c < FIRST_CHAR || c > LAST_CHAR) {
      if (c == ' ')
        penX += screenScale * 0.3f;
      continue;
    }

    const GlyphInfo &g = glyphs[c - FIRST_CHAR];

    if (g.bitmapW == 0 || g.bitmapH == 0) {
      penX += static_cast<float>(g.advanceX) / 64.0f * px2ws;
      continue;
    }

    float x0 = penX + static_cast<float>(g.bearingX) * px2ws;
    float x1 = x0 + static_cast<float>(g.bitmapW) * px2ws;

    // Vulkan NDC has Y pointing DOWN.
    // bearingY is pixels above the baseline → negate to push the glyph
    // downward.
    float y0 = -static_cast<float>(g.bearingY) * px2ws; // top edge
    float y1 =
        static_cast<float>(g.bitmapH - g.bearingY) * px2ws; // bottom edge

    uint32_t base = static_cast<uint32_t>(vertices.size());

    // UV v0 = top of glyph in atlas, v1 = bottom — unchanged.
    vertices.push_back(FontVertex{{x0, y0, 0.f}, {g.u0, g.v0}}); // TL
    vertices.push_back(FontVertex{{x1, y0, 0.f}, {g.u1, g.v0}}); // TR
    vertices.push_back(FontVertex{{x1, y1, 0.f}, {g.u1, g.v1}}); // BR
    vertices.push_back(FontVertex{{x0, y1, 0.f}, {g.u0, g.v1}}); // BL

    indices.insert(indices.end(), {base + 0, base + 1, base + 2, base + 2,
                                   base + 3, base + 0});

    penX += static_cast<float>(g.advanceX) / 64.0f * px2ws;
  }

  mesh.loadVertices(vertices);
  mesh.loadIndices(indices);
  mesh.createVertexBuffer();
  mesh.createIndexBuffer();

  return penX;
}

// ─────────────────────────────────────────────────────────────────────────────
void Fonts::cleanup() {
  mesh.cleanup();
  // texture.destroy() is called in HelloTriangleApplication::cleanup()
}
