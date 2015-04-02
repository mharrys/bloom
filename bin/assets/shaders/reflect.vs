#version 130

uniform mat4 model_view;
uniform mat4 projection;

uniform vec4 eye_position;

in vec4 vertex_position;
in vec3 vertex_normal;

out vec3 reflect_dir;

void main()
{
    vec4 position = vertex_position;
    vec3 normal = normalize(vertex_normal);

    vec3 incident_dir = normalize(position.xyz - eye_position.xyz);
    reflect_dir = reflect(incident_dir, normal);

    gl_Position = projection * model_view * vertex_position;
}
