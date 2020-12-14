#version 430 core
out vec4 f_color;

in vec2 o_texture_coordinate;

uniform sampler2D u_frame;
uniform int u_effect;

void main()
{   
	vec2 texture_coordinate = o_texture_coordinate;
	if((u_effect&0x01)==1)
	{
		float pixel = 100.0;
		texture_coordinate=vec2(floor(o_texture_coordinate.x*pixel)/pixel, floor(o_texture_coordinate.y*pixel)/pixel);
	}
	f_color = vec4(texture(u_frame, texture_coordinate));
}