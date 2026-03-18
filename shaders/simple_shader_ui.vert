#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 uvIn;
layout(location = 2) in uint inTextureIndex;

layout(location = 0) out vec2 uvOut;
layout(location = 1) out flat uint outTextureIndex;


layout(push_constant) uniform PushConstants {
    mat4 data;
} pc;


void main() {
    gl_PointSize = 4.f;
    gl_Position = ubo.proj * ubo.view * pc.data * vec4(inPosition, 1.0);
    uvOut = uvIn;
    outTextureIndex = inTextureIndex;
}