#version 130

uniform vec2 resolution;
uniform sampler2D read;

uniform float weights[5];

out vec4 frag_color;

void main(void)
{
    vec2 uv = gl_FragCoord.xy / resolution.xy;

    frag_color = texture(read, uv) * weights[0];
    for (int i = 1; i < 5; i++) {
        vec2 x_offset = vec2(i / resolution.x, 0.0);
        frag_color += texture(read, uv - x_offset) * weights[i];
        frag_color += texture(read, uv + x_offset) * weights[i];
    }
}
