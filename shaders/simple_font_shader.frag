#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 3) uniform sampler2D tex;

void main()
{
    float alpha = texture(tex, fragUV).r;

    if (alpha < 0.01)
        discard;

    float D = sqrt(fragUV.x * fragUV.x + fragUV.y * fragUV.y + 1);
    outColor = vec4(fragUV.x / D, fragUV.y / D, 1.0 / D, alpha);
}
