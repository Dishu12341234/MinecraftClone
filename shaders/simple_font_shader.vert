#version 450

// matches FontVertex
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;

// matches PushConstantC1 { mat4 data; }
layout(push_constant) uniform Push {
    mat4 mvp;
} pc;

void main() {
    gl_Position = pc.mvp * vec4(inPos, 1.0);
    fragUV = inUV;
}
