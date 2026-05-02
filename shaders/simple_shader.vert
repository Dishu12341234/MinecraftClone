#version 450
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inSize;
layout(location = 2) in vec2 textureInfoAndIdx; // x = array layer, y = vertex in6dex 0-3
layout(location = 3) in float brightness;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) flat out int blockID;
layout(location = 3) out float brightnessO;

layout(push_constant) uniform PushConstants {
    mat4 data;
} pc;

void main() {

const vec2 uvTable[4] = vec2[4](
        vec2(0.0, 0.0),
        vec2(0.0, inSize.y),
        vec2(inSize.x, inSize.y),
        vec2(inSize.x, 0)
    );



    vec4 viewPos = ubo.view * pc.data * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * viewPos;

    fragColor = vec3(1,1,1);
    blockID = int(textureInfoAndIdx.x);
    fragTexCoord = uvTable[int(textureInfoAndIdx.y)];
    brightnessO = brightness;
}
