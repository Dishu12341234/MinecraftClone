#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in float brightnessO;
layout(location = 3) in vec2 tileMin;
layout(location = 4) in vec2 tileMax;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 clampedUV = clamp(fragTexCoord, tileMin, tileMax);
    vec4 texColor = texture(texSampler, clampedUV);
    outColor = texColor * brightnessO;
}