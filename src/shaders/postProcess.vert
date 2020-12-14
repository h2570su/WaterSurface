#version 430 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texture_coordinate;

out vec2 o_texture_coordinate;

void main()
{
    gl_Position = vec4(position, 1.0f);			
    o_texture_coordinate = texture_coordinate;
}