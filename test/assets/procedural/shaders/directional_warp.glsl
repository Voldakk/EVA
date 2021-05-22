//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform writeonly image2D u_Output;
layout(binding = 1, r32f) uniform readonly image2D u_Input;
layout(binding = 2, r32f) uniform readonly image2D u_IntensityMap;

uniform float u_Angle;
uniform float u_Intensity;
uniform sampler2D u_InputMapSampler;
uniform sampler2D u_IntensityMapSampler;

const ivec3 eps = ivec3(-1, 0, 1);

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = vec2(pixelCoords) / vec2(dims);
	
	float angle = radians(u_Angle);
	float intensity = imageLoad(u_IntensityMap, pixelCoords).r * u_Intensity;

	float dist = intensity;
	vec2 dir = normalize(vec2(cos(angle), sin(angle)));

	uv += dir * dist;

	float value = texture(u_InputMapSampler, uv).r;
	//float value = 0;

	imageStore(u_Output, pixelCoords, vec4(value, 0.0, 0.0, 1.0));
}
