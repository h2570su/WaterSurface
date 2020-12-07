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

float getTessLevel(float distance_0, float distance_1);

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	 e_in_position[gl_InvocationID] = c_in_position[gl_InvocationID];
	 e_in_normal[gl_InvocationID] = c_in_normal[gl_InvocationID];
	e_in_texture_coordinate[gl_InvocationID] = c_in_texture_coordinate[gl_InvocationID];
    e_in_color[gl_InvocationID] = c_in_color[gl_InvocationID];
   
	if(gl_InvocationID == 0)
	{
		float distance_0 = distance(u_viewer_pos, vec3(u_model * vec4(e_in_position[0], 1.0)));
		float distance_1 = distance(u_viewer_pos, vec3(u_model * vec4(e_in_position[1], 1.0)));
		float distance_2 = distance(u_viewer_pos, vec3(u_model * vec4(e_in_position[2], 1.0)));

	
		gl_TessLevelOuter[0] = getTessLevel(distance_1 , distance_2);
		gl_TessLevelOuter[1] = getTessLevel(distance_2 , distance_0);
		gl_TessLevelOuter[2] = getTessLevel(distance_0 , distance_1);
		gl_TessLevelInner[0] = gl_TessLevelOuter[2];
	}
}

float getTessLevel(float distance_0, float distance_1)
{
    float AvgDistance = (distance_0 + distance_1) / 2.0;
	float nearDistance = 100.0;
	float farDistance = 1000.0;
	float maxTessLevel = 64.0;
	float minTessLevel = 4.0;
	float noTessLevel = 1.0;
    if (AvgDistance <= nearDistance) 
	{
        return maxTessLevel;
    }
    else if (AvgDistance <= farDistance) 
	{
		float a = (minTessLevel-maxTessLevel) / (farDistance-nearDistance);
		float b = maxTessLevel - (nearDistance*a);
		return a*AvgDistance+b; 
    }
    else 
	{
        return minTessLevel;
    }
} 