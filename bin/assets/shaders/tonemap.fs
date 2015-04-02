#version 130

uniform vec2 resolution;
uniform sampler2D read;
uniform sampler2D bloom;

uniform float exposure = 1.2;
uniform float luma_max = 0.5;
uniform float bloom_factor = 0.7;

out vec4 frag_color;

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec4 color = texture(read, uv);
    vec4 color_bloom = texture(bloom, uv);

    color += color_bloom * bloom_factor;

    float Y = dot(vec4(0.30, 0.59, 0.11, 0.0), color);
    float YD = exposure * (exposure / luma_max + 1.0) / (exposure + 1.0);
    color *= YD;

    frag_color = color;
}
