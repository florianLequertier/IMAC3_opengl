
#version 410 core

in block
{
    vec2 Texcoord;
} In;

out vec4 Color;
 

// Uniforms : 


uniform sampler2D ColorBuffer;
uniform sampler2D NormalBuffer;
uniform sampler2D DepthBuffer;
uniform mat4 ScreenToWorld;
uniform vec3 CameraPosition;
 
 //shadow : 
 uniform sampler2D Shadow;
 uniform mat4 WorldToLightScreen;

//lights struct : 

struct DirectionalLight{
	vec3 direction;
	vec3 color;
	float intensity; 
};

//light uniforms : 
uniform DirectionalLight directionalLight;

vec3 computeDirectionalLight(DirectionalLight light, vec3 p, vec3 n,  vec3 diffuse, vec3 specular, float specularPower)
{
	vec3 l = normalize(-light.direction);
	float ndotl = clamp(dot(n,l), 0.0, 1.0);
	vec3 v = normalize(CameraPosition - p);
	vec3 h = normalize(l+v);
	float ndoth = clamp(dot(n,h),0.0,1.0);

	diffuse /= 3.1415f;

	specular *= pow(ndoth, specularPower);
	specular /= ( (specularPower + 8.0) / (8.0*3.1415) );

	float intensity = light.intensity;

	return intensity * light.color * 3.1415 * (diffuse + specular) * (ndotl) ; 
}

float computeShadow(vec3 p)
{
	//shadow
	float shadowBias = 0.01f;
	vec4 wlP = WorldToLightScreen * vec4(p.xyz, 1.0);
	vec3 lP = vec3(wlP/wlP.w) * 0.5 + 0.5;
	//float lDepth =  texture(Shadow, lP.xy).r; //textureProj(Shadow, vec4(lP.xy, lP.z - shadowBias, 1.0), 0.0);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(Shadow, 0);

	//test if we are outside the shadow map. 
	if(lP.x < 0 || lP.x > 1 || lP.y < 0 || lP.y > 1 || lP.z > 1 || lP.z < 0)
		return 0;

	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(Shadow, lP.xy + vec2(x, y)*texelSize).r;
			shadow += (pcfDepth + shadowBias > lP.z) ? 0.0 : 1.0;        
		}    
	}
	shadow /= 9.0;

	return shadow;
}


void main(void)
{

	// Read gbuffer values
	vec4 colorBuffer = texture(ColorBuffer, In.Texcoord).rgba;
	vec4 normalBuffer = texture(NormalBuffer, In.Texcoord).rgba;
	float depth = texture(DepthBuffer, In.Texcoord).r;

	// Convert texture coordinates into screen space coordinates
	vec2 xy = In.Texcoord * 2.0 -1.0;
	// Convert depth to -1,1 range and multiply the point by ScreenToWorld matrix
	vec4 wP = vec4(xy, depth * 2.0 -1.0, 1.0) * ScreenToWorld;
	// Divide by w
	vec3 p = vec3(wP.xyz / wP.w);

	vec3 diffuse = colorBuffer.rgb;
	vec3 specular = colorBuffer.aaa;
	float specularPower = normalBuffer.a;
	vec3 n = normalBuffer.rgb*2.0 -1.0;


	vec3 color = computeDirectionalLight(directionalLight, p, n, diffuse, specular, specularPower * 100);
	color *= (1-computeShadow(p));

	//if(lDepth + shadowBias > lP.z)
		Color = vec4(color, 1.0);
	//else
	//	Color = vec4(0.0, 0.0, 0.0, 1.0);

}