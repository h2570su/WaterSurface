#version 430 core
out vec4 f_color;

in vec3 o_position;


uniform vec3 u_viewer_pos;

uniform samplerCube u_skybox;


void main()
{   
	vec3 sourceColor = vec3(texture(u_skybox, o_position));		
		f_color = vec4(sourceColor, 1.0f);
}