#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

//inside the c++ code for sake of learning I have use a push constant for ubo.proj as pc.data

layout(push_constant) uniform PushConstants {
    mat4 data;
} pc;

void main() {
    //gl_Position = pc.data * ubo.view * ubo.model[gl_InstanceIndex] * vec4(inPosition, 1.0);
    gl_Position = pc.data * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord * .05f;
}