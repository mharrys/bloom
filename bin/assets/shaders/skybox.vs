#version 130

uniform mat4 view; // camera orientation only
uniform mat4 projection;

in vec4 vertex_position;

out vec3 reflect_dir;

void main()
{
    reflect_dir = vertex_position.xyz;
    gl_Position = projection * view * vertex_position;
}
