//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform writeonly image2D u_Output;

uniform vec4 u_Color;

void main()
{
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	imageStore(u_Output, pixelCoords, u_Color);
}
