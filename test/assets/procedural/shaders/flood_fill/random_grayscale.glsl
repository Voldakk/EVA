//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform restrict writeonly image2D u_Output;
layout(binding = 1, rgba32f) uniform restrict readonly image2D u_FloodFillData;

uniform float u_MinValue;
uniform float u_MaxValue;
uniform float u_Seed;

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}
float noise(vec2 p){
    p *= 1413.2371;
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	vec3 data = imageLoad(u_FloodFillData, pixelCoords).rgb;
	float value = 0;
	if(data.z > 0) { value = mix(u_MinValue, u_MaxValue, noise(vec2(data.z, u_Seed))); }

	imageStore(u_Output, pixelCoords, vec4(value, 0.0, 0.0, 1.0));
}
