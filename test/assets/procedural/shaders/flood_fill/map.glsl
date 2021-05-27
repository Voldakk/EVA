//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform writeonly image2D u_Output;
layout(binding = 1, rgba32f) uniform readonly image2D u_FloodFillData;

uniform sampler2D u_MapSampler;

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	vec3 data = imageLoad(u_FloodFillData, pixelCoords).rgb;
	float value = 0;
	if(data.z > 0) { value = texture(u_MapSampler, data.xy).r; }

	imageStore(u_Output, pixelCoords, vec4(value, 0.0, 0.0, 1.0));
}
