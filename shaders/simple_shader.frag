#version 450
layout(binding = 1) uniform sampler2DArray texSampler;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in int blockID;
layout(location = 3) in float brightnessO;
layout(location = 0) out vec4 outColor;

void main() {
    vec4 texColor = texture(texSampler, vec3(fragTexCoord, float(blockID)));
    if (texColor.a < 0.1) discard;

    vec3 color = texColor.rgb * brightnessO;

    // Fake bloom — luminance above threshold bleeds into extra brightness
    float luma      = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float threshold = 0.3;
    float bloom     = max(luma - threshold, 0.0) * 2.5;
    color += bloom * color;   // tinted glow, not white blowout

    // Depth fog
    float depth     = gl_FragCoord.z / gl_FragCoord.w;
    float fogFactor = clamp((192.0 - depth) / (192.0 - 48.0), 0.0, 1.0);
    vec3  fogColor  = vec3(0.78, 0.88, 1.0);
    color = mix(fogColor, color, fogFactor);

    outColor = vec4(color, texColor.a);
}
