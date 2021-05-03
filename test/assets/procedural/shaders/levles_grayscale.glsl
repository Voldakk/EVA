//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform writeonly image2D u_Output;
layout(binding = 1, r32f) readonly uniform image2D u_Input;

uniform vec2 u_InputRange = vec2(0.0, 1.0);
uniform vec2 u_OutputRange = vec2(0.0, 1.0);
uniform float u_Midtone = 0.5;

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	float inPixel = imageLoad(u_Input, pixelCoords).r;
	vec4 outPixel = vec4(0);
	
	// Do work
	/*float gamma = 0.0;
	if (u_Midtone < 0.5)
	{
		u_Midtone *= 2;
		gamma = 1 + (9 * (1 - u_Midtone));
		gamma = min(gamma, 9.99);
	} 
	else if(u_Midtone > 0.5)
	{
		u_Midtone = (u_Midtone * 2) - 1;
		gamma = 1 - u_Midtone;
		gamma = max(gamma, 0.01);
	}*/
	
	float value = inPixel = (inPixel - u_InputRange.x) / (u_InputRange.y - u_InputRange.x);
	value = pow(value, u_Midtone * 2);
	value = value * (u_OutputRange.y - u_OutputRange.x) + u_OutputRange.x;

	value = clamp(value, 0.0, 1.0);
	outPixel.r = value;
	imageStore(u_Output, pixelCoords, outPixel);
}
