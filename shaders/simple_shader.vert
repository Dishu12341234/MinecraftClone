#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 textureInfoAndIdx; // x = tile index, y = vertex index (0-3)
layout(location = 3) in float brightness;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out float brightnessO;

layout(push_constant) uniform PushConstants {
    mat4 data;
} pc;

const float TILE_SIZE  = 0.05;
const float ATLAS_COLS = 1.0 / TILE_SIZE; // 20

const vec2 uvTable[4] = vec2[4](
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

void main() {
    gl_PointSize = 5.f;

    const float WORLD_SCALE = 1.f;
    vec4 viewPos = ubo.view * pc.data * vec4(inPosition, 1.0);
    viewPos.xyz *= WORLD_SCALE;
    gl_Position = ubo.proj * viewPos;
    fragColor = inColor;



    int tid = int(textureInfoAndIdx.x);

    int vid = int(textureInfoAndIdx.y);
    vec2 cornerUV = uvTable[vid];

    float tileSize = 0.05;

    int x = tid % 20;
    int y = tid / int(ATLAS_COLS);


    fragTexCoord = (cornerUV + vec2(x,y)) * tileSize;

    brightnessO = brightness;
}