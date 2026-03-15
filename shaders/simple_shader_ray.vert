#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform PushConstants {
    mat4 data;
} pc;


void main() {
    gl_PointSize = 4.f;
    gl_LineWidth = 4.f;
    gl_Position = ubo.proj * ubo.view * pc.data * vec4(inPosition, 1.0);
    fragColor = inColor;
}