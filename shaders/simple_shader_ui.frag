#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 2) uniform sampler2D texSampler[16];

layout(location = 0) in vec2 uv;
layout(location = 1) in flat uint inTextureIndex;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler[nonuniformEXT(inTextureIndex)], uv);
}