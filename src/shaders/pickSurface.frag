#version 430 core
out vec4 f_color;

in vec2 o_texture_coordinate;

void main()
{   	
	f_color = vec4(o_texture_coordinate,0,1.0);
}