#version 420 core

layout(triangles, equal_spacing, ccw) in;

in vec3 e_in_position[];
in vec3 e_in_normal[];
in vec2 e_in_texture_coordinate[];
in vec3 e_in_color[];

out vec3 f_in_position;
out vec3 f_in_normal;
out vec2 f_in_texture_coordinate;
out vec3 f_in_color;

uniform vec2 u_direction;
uniform float u_time;
uniform float u_wavelength;
uniform float u_amplitude;
uniform int u_waveSelect;
uniform highp sampler2D u_heightmap;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};

//from http://ogldev.atspace.co.uk/www/tutorial30/tutorial30.html
vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main()
{
	f_in_texture_coordinate = interpolate2D(e_in_texture_coordinate[0], e_in_texture_coordinate[1], e_in_texture_coordinate[2]);
	
	f_in_position = interpolate3D(e_in_position[0], e_in_position[1],e_in_position[2]);
	if(u_waveSelect==0)
	{
		f_in_position += vec3(0,u_amplitude,0)*sin((2*3.14)*(dot(u_direction ,f_in_texture_coordinate)+u_time)/u_wavelength);
	}
	else
	{
		vec2 heightmapCoord = f_in_texture_coordinate;
		heightmapCoord+=u_direction*(u_time/20);
		heightmapCoord/=(u_wavelength*8);		
		heightmapCoord = heightmapCoord - (vec2(1,1) * floor(heightmapCoord.xy));
		f_in_position += vec3(0,2*u_amplitude,0) *( vec3(texture(u_heightmap, heightmapCoord))-vec3(0,0.5,0));
	}

    f_in_color = interpolate3D(e_in_color[0], e_in_color[1],e_in_color[2]);
	gl_Position =u_projection*u_view*vec4(f_in_position,1.0);
}