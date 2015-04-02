#version 130

uniform vec2 resolution;
uniform sampler2D read;

uniform float thresh = 0.5;

out vec4 frag_color;

void main(void) {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec4 texel = texture(read, uv);

    vec3 luma_compare = vec3(0.2125, 0.7154, 0.0721);
    float luma = dot(luma_compare, texel.rgb);
    luma = max(0.0, luma - thresh);

    frag_color = vec4(texel.rgb * sign(luma), 1.0);
}
