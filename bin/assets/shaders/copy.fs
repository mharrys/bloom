#version 130

uniform vec2 resolution;
uniform sampler2D color_map;

out vec4 frag_color;

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    frag_color = texture(color_map, uv);
}
