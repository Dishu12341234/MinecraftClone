#version 450
layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 3) uniform sampler2D tex;

vec3 hsl2rgb(float h, float s, float l) {
    float c = (1.0 - abs(2.0 * l - 1.0)) * s;
    float x = c * (1.0 - abs(mod(h * 6.0, 2.0) - 1.0));
    float m = l - c * 0.5;
    vec3 rgb;
    if      (h < 1.0/6.0) rgb = vec3(c, x, 0);
    else if (h < 2.0/6.0) rgb = vec3(x, c, 0);
    else if (h < 3.0/6.0) rgb = vec3(0, c, x);
    else if (h < 4.0/6.0) rgb = vec3(0, x, c);
    else if (h < 5.0/6.0) rgb = vec3(x, 0, c);
    else                  rgb = vec3(c, 0, x);
    return rgb + m;
}

void main()
{
    float alpha = texture(tex, fragUV).r;
    if (alpha < 0.01)
        discard;

    // --- Spherical normal (same math, kept) ---
    float D    = sqrt(fragUV.x * fragUV.x + fragUV.y * fragUV.y + 1.0);
    vec3  norm = vec3(fragUV.x / D, fragUV.y / D, 1.0 / D);

    // --- Iridescent hue shift across the surface ---
    float r    = length(fragUV);                        // 0 → centre, grows outward
    float hue  = fract(r * 1.4 + norm.z * 0.6);        // rings of colour
    vec3  iris = hsl2rgb(hue, 0.85, 0.60);

    // --- Fake specular highlight (fixed "light" at top-left) ---
    vec3  lightDir  = normalize(vec3(-0.4, 0.8, 1.2));
    float specPow   = 64.0;
    float spec      = pow(max(dot(norm, lightDir), 0.0), specPow);

    // --- Fresnel-like rim glow ---
    float rim = 1.0 - clamp(norm.z, 0.0, 1.0);   // bright at edges
    rim       = pow(rim, 2.5);

    // --- Compose ---
    vec3 colour = iris                          // iridescent base
                + vec3(1.0) * spec * 1.2        // hot white specular
                + vec3(0.55, 0.85, 1.0) * rim;  // icy blue rim

    outColor = vec4(colour, alpha);
}
