//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform writeonly image2D u_Output;
layout(binding = 1, r32f) readonly uniform image2D u_InputA;
layout(binding = 2, r32f) readonly uniform image2D u_InputB;

uniform float u_Frac = 0.5;

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	vec4 outPixel = vec4(1);
	float a = imageLoad(u_InputA, pixelCoords).r;
	float b = imageLoad(u_InputB, pixelCoords).r;

	// Do work
	outPixel.r = mix(a, b, u_Frac);

	imageStore(u_Output, pixelCoords, outPixel);
}
