#version 130

uniform sampler2D tex;
uniform float height;
uniform float weight[31];

in vec2 tex_coord;

out vec4 frag_color;

void main()
{
    frag_color = texture2D(tex, tex_coord) * weight[0];
    for (int i = 1; i < 31; i++) {
        vec2 tex_offset = vec2(0.0, i / height);
        frag_color += texture2D(tex, tex_coord - tex_offset) * weight[i];
        frag_color += texture2D(tex, tex_coord + tex_offset) * weight[i];
    }
}
