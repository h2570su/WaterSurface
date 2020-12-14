#version 430 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texture_coordinate;

uniform mat4 u_model;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};
out vec2 o_texture_coordinate;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0f);	 
    o_texture_coordinate = vec2(texture_coordinate.y, 1.0f - texture_coordinate.x);	
}