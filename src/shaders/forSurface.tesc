#version 420 core

layout (vertices = 3) out;

in vec3 c_in_position[];
in vec3 c_in_normal[];
in vec2 c_in_texture_coordinate[];
in vec3 c_in_color[];

out vec3 e_in_position[];
out vec3 e_in_normal[];
out vec2 e_in_texture_coordinate[];
out vec3 e_in_color[];

uniform mat4 u_model;
uniform vec3 u_viewer_pos;



void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	 e_in_position[gl_InvocationID] = c_in_position[gl_InvocationID];
	 e_in_normal[gl_InvocationID] = c_in_normal[gl_InvocationID];
	e_in_texture_coordinate[gl_InvocationID] = c_in_texture_coordinate[gl_InvocationID];
    e_in_color[gl_InvocationID] = c_in_color[gl_InvocationID];
   

	
    gl_TessLevelOuter[0] = 500000.0 ;
    gl_TessLevelOuter[1] = 500000.0 ;
    gl_TessLevelOuter[2] = 500000.0 ;
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}