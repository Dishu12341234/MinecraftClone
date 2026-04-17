#pragma once

#include "Textures.hpp"
class UITexture : public u_Texture {
private:
public:
  UITexture();
  void createTextureSampler() override;
  ~UITexture();
};
