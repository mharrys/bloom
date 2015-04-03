#version 130

uniform vec2 resolution;
uniform sampler2D read;

uniform float weights[5];

out vec4 frag_color;

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution.xy;

    frag_color = texture(read, uv) * weights[0];
    for (int i = 1; i < 5; i++) {
        vec2 y_offset = vec2(0.0, i / resolution.y);
        frag_color += texture(read, uv - y_offset) * weights[i];
        frag_color += texture(read, uv + y_offset) * weights[i];
    }
}
