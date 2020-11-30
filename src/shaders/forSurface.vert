#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coordinate;
layout (location = 3) in vec3 color;

uniform mat4 u_model;
uniform vec3 u_viewer_pos;
uniform int u_shadingSelect;
struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
struct PointLight {    
    vec3 position;   
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant;
    float linear;
    float quadratic;  
};  
struct SpotLight {    
    vec3 position;
        
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant;
    float linear;
    float quadratic;  

	 vec3 direction;
	 float cutoff;
	 float outer_cutoff;
};    
#define NR_DIRECTIONAL_LIGHTS 4  
uniform DirLight dirLights[NR_DIRECTIONAL_LIGHTS];
#define NR_POINT_LIGHTS 4  
uniform PointLight pointLights[NR_POINT_LIGHTS];
#define NR_SPOT_LIGHTS 4
uniform  SpotLight spotLights[NR_SPOT_LIGHTS];

uniform sampler2D u_texture;
uniform bool u_useTexture;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};


out vec3 c_in_position;
out vec3 c_in_normal;
out vec2 c_in_texture_coordinate;
out vec3 c_in_color;



vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 materialColor)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // combine results
    vec3 ambient = light.ambient * materialColor;
    vec3 diffuse = light.diffuse * diff * materialColor;
    vec3 specular = light.specular * spec * materialColor;
    return (ambient + diffuse + specular);
}  

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 materialColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
    float distance    = length(light.position - fragPos);
	light.constant=(light.constant==0.0f)?0.0000001f:light.constant;
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient  * materialColor;
    vec3 diffuse  = light.diffuse  * diff * materialColor;
    vec3 specular = light.specular * spec * materialColor;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 materialColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
	light.constant=(light.constant==0.0f)?0.0000001f:light.constant;
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);
    // combine results

    vec3 ambient = light.ambient * materialColor;
    vec3 diffuse = light.diffuse * diff * materialColor;
    vec3 specular = light.specular * spec * materialColor;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}


void main()
{
    //gl_Position = u_projection * u_view * u_model * vec4(position, 1.0f);	
    c_in_position = vec3(u_model * vec4(position, 1.0f));
    c_in_normal = mat3(transpose(inverse(u_model))) * normal;
    c_in_texture_coordinate = vec2(texture_coordinate.x, 1.0f - texture_coordinate.y);
	if(u_shadingSelect==0||u_shadingSelect==1)
	{
		c_in_color = color;
	}
	else
	{
		vec3 sourceColor;
	
		if(u_useTexture)
		{    
			sourceColor = vec3(texture(u_texture, c_in_texture_coordinate));		
		}
		else
		{
			sourceColor = color;		
		}

		vec3 result = vec3(0,0,0);
		vec3 _normal = normalize( c_in_normal);
		vec3 viewDir = normalize(u_viewer_pos - c_in_position);
		for(int i=0;i<NR_DIRECTIONAL_LIGHTS;i++)
		{
			result += CalcDirLight(dirLights[i], _normal, viewDir, sourceColor);
		}
		for(int i=0;i<NR_POINT_LIGHTS;i++)
		{
			result += CalcPointLight(pointLights[i], _normal, c_in_position, viewDir, sourceColor);
		}
		for(int i=0;i<NR_SPOT_LIGHTS;i++)
		{
			result += CalcSpotLight(spotLights[i], _normal, c_in_position, viewDir, sourceColor);
		}
		c_in_color = result;
	}
}