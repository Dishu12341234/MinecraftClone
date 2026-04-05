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

    int xOffset = gl_InstanceIndex % 10;
    int yOffset = gl_InstanceIndex / 10;

    gl_PointSize = 4.f;
    mat4 model = mat4(1.0);
    model[3] = vec4(xOffset / 10.f, - yOffset / 10.f, 0.0, 1.0);
    //            proj
    gl_Position = pc.data * model * vec4(inPosition, 1.0);
    uvOut = uvIn;
    outTextureIndex = inTextureIndex;
}