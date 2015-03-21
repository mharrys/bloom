#version 130

uniform vec2 resolution;
uniform sampler2D color_map;
uniform float thresh = 0.23;

out vec4 frag_color;

float luminace(vec3 color) {
    return 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec4 texel = texture(color_map, uv);
    if (luminace(texel.rgb) > thresh) {
        // mix some color into the bloom instead of just having a 100% white bloom
        frag_color = vec4(mix(texel, vec4(1.0), 0.8).rgb, 1.0);
    } else {
        frag_color = vec4(vec3(0.0), 1.0);
    }
}
