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
uniform bool u_testNormal;

#define MAX_DROPS 100

uniform vec2 u_drop[MAX_DROPS];
uniform float u_dropTime[MAX_DROPS];
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

vec3 getHeightMapCoord(in vec2 heightmapCoord, in vec3 XandZ)
{
	
		heightmapCoord+=u_direction*(u_time/20);
		heightmapCoord/=(u_wavelength*8);		
		heightmapCoord = heightmapCoord - (vec2(1,1) * floor(heightmapCoord.xy));
		XandZ.y= 2*u_amplitude *(texture(u_heightmap, heightmapCoord).r - 0.5);
		return XandZ;
}

vec3 getSineCoord(in vec2 heightmapCoord, in vec3 XandZ)
{
		XandZ.y = u_amplitude * sin((2*3.14)*(dot(u_direction ,heightmapCoord)+u_time)/u_wavelength);
		return XandZ;
}
float delta = 0.0001;
vec3 getHeightMapNormal(in vec2 heightmapCoord, in vec3 XandZ)
{	
	
	vec3 dx = vec3(
        delta*400,
        getHeightMapCoord(vec2(heightmapCoord.x+delta,heightmapCoord.y), XandZ).y - getHeightMapCoord(vec2(heightmapCoord.x-delta,heightmapCoord.y), XandZ).y,
        0.0);
	vec3 dy = vec3(
        0.0,
        getHeightMapCoord(vec2(heightmapCoord.x,heightmapCoord.y+delta), XandZ).y - getHeightMapCoord(vec2(heightmapCoord.x,heightmapCoord.y-delta), XandZ).y,
        -delta*400);

	return normalize( cross(-dy,dx));
}

vec3 getSineNormal(in vec2 heightmapCoord, in vec3 XandZ)
{
	
	vec3 dx = vec3(
        delta*400,
        getSineCoord(vec2(heightmapCoord.x+delta,heightmapCoord.y), XandZ).y -  getSineCoord(vec2(heightmapCoord.x-delta,heightmapCoord.y), XandZ).y,
        0.0);
	vec3 dy = vec3(
        0.0,
        getSineCoord(vec2(heightmapCoord.x,heightmapCoord.y+delta), XandZ).y - getSineCoord(vec2(heightmapCoord.x,heightmapCoord.y-delta), XandZ).y,
        -delta*400);

	return normalize( cross(-dy,dx));
}

vec3 getSimCoord(in vec2 heightmapCoord, in vec3 XandZ)
{
	XandZ.y = 0;
	for(int i=0;i<MAX_DROPS;i++)
	{
		if(u_dropTime[i]!=0)
		{
			float dist = distance(heightmapCoord, u_drop[i])/(u_wavelength)*30;
			float t_c = (u_time-u_dropTime[i])*(2*3.1415926)*5.0;
			XandZ.y += u_amplitude * sin((dist-t_c)*clamp(0.0125*t_c,0,1))/(exp(0.1*abs(dist-t_c)+(0.05*t_c)))*1.5;
		}
		else
		{
			break;
		}
	}
	return XandZ;
}

vec3 getSimNormal(in vec2 heightmapCoord, in vec3 XandZ)
{	
	vec3 dx = vec3(
        delta*200,
        getSimCoord(vec2(heightmapCoord.x+delta,heightmapCoord.y), XandZ).y -  XandZ.y,
        0.0);
	vec3 dy = vec3(
        0.0,
        getSimCoord(vec2(heightmapCoord.x,heightmapCoord.y+delta), XandZ).y - XandZ.y,
        -delta*200);

	return normalize( cross(-dy,dx));
}

void main()
{
	f_in_texture_coordinate = interpolate2D(e_in_texture_coordinate[0], e_in_texture_coordinate[1], e_in_texture_coordinate[2]);
	f_in_color = interpolate3D(e_in_color[0], e_in_color[1], e_in_color[2]);
	f_in_position = interpolate3D(e_in_position[0], e_in_position[1],e_in_position[2]);
	if(u_waveSelect==2)
	{
		f_in_position = getSimCoord(f_in_texture_coordinate, f_in_position);
		f_in_normal = getSimNormal(f_in_texture_coordinate, f_in_position);	
	}
	else if(u_waveSelect==1)
	{		
		f_in_position = getHeightMapCoord(f_in_texture_coordinate, f_in_position);
		f_in_normal = getHeightMapNormal(f_in_texture_coordinate, f_in_position);		
	}
	else
	{
		f_in_position = getSineCoord(f_in_texture_coordinate, f_in_position);
		f_in_normal = getSineNormal(f_in_texture_coordinate, f_in_position);				
	}

    //f_in_color = interpolate3D(e_in_color[0], e_in_color[1],e_in_color[2]);
	gl_Position =u_projection*u_view*vec4(f_in_position,1.0);
}