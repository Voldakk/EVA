//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform restrict writeonly image2D u_Output;
layout(binding = 1, rgba32f) uniform restrict readonly image2D u_ExtentsMap;

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

	vec4 extents = imageLoad(u_ExtentsMap, pixelCoords);
	float value = 0;
	if(extents.w > 0) { value = mix(u_MinValue, u_MaxValue, rand(vec3(extents.xy, u_Seed))); }

	imageStore(u_Output, pixelCoords, vec4(value, 0.0, 0.0, 1.0));
}
