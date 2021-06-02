//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform restrict writeonly image2D u_Output;
layout(binding = 1, r32ui) uniform restrict readonly uimage2D u_LabelMap;

uniform float u_MinValue;
uniform float u_MaxValue;
uniform float u_Seed;

float rand(vec3 n) { 
	return fract(sin(dot(n, vec3(12.9898, 4.1414, 3.1415))) * 43758.5453);
}

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	uint label = imageLoad(u_LabelMap, pixelCoords).r;
	vec3 value = vec3(0);
	if(label > 0) 
	{ 
		value.r = mix(u_MinValue, u_MaxValue, rand(vec3(label, u_Seed, 0.123)));
		value.g = mix(u_MinValue, u_MaxValue, rand(vec3(label, u_Seed, 5.193)));
		value.b = mix(u_MinValue, u_MaxValue, rand(vec3(label, u_Seed, 7.315)));
	}

	imageStore(u_Output, pixelCoords, vec4(value, 1.0));
}
