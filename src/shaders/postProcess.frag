#version 430 core
out vec4 f_color;

in vec2 o_texture_coordinate;

uniform sampler2D u_frame;
uniform int u_effect;
uniform float u_time;

void main()
{   
	vec2 texture_coordinate = o_texture_coordinate;

	f_color = vec4(texture(u_frame, texture_coordinate));
	

	if((u_effect&0x01)!=0)
	{
		if(o_texture_coordinate.x>0.5)
		{
			float pixel = 100.0;
			texture_coordinate=vec2((floor(texture_coordinate.x*pixel)+0.5)/pixel, floor((texture_coordinate.y*pixel)+0.5)/pixel);
			f_color = vec4(texture(u_frame, texture_coordinate));
		}
		else
		{
			f_color = vec4(texture(u_frame, texture_coordinate));
		}
		
	}
	if((u_effect&0x02)!=0)
	{
		if(o_texture_coordinate.x>0.5)
		{
			f_color = vec4(texture(u_frame, texture_coordinate + 0.005*vec2( sin(u_time+1024.0*texture_coordinate.x),cos(u_time+768.0*texture_coordinate.y)) ).xyz, 1.0);
		}
		else
		{
			f_color = vec4(texture(u_frame, texture_coordinate));
		}
		
	}
	if((u_effect&0x04)!=0)
	{
		if(o_texture_coordinate.x>0.5)
		{
			 float dist = distance(vec2(0.5,0.5),texture_coordinate)*5+u_time;
			 mat2 rot = mat2(cos(dist),-sin(dist),sin(dist),cos(dist));
			 texture_coordinate = texture_coordinate*rot;
			f_color = vec4(texture(u_frame, texture_coordinate));
		}
		else
		{
			f_color = vec4(texture(u_frame, texture_coordinate));
		}
		
	}
	
	if(u_effect!=0 && o_texture_coordinate.x>=0.495&&o_texture_coordinate.x<=0.505)
	{
			f_color = vec4(1.0,0,0,1.0);
	}
}