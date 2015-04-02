#version 130

uniform samplerCube env;

in vec3 reflect_dir;

out vec4 frag_color;

void main()
{
    frag_color = texture(env, reflect_dir);
}
