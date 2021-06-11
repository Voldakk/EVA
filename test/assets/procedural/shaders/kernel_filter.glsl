//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform writeonly image2D u_Output;
layout(binding = 1, r32f) uniform readonly image2D u_Input;

uniform ivec2 u_KernelSize;
uniform int[9*9] u_Kernel;

uniform int u_Divisor;
uniform int u_StepSize;

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	ivec2 extents = u_KernelSize / 2;

	float value = 0;
	for (int y = -extents.y; y <= extents.y; ++y) 
    {
        for (int x = -extents.x; x <= extents.x; ++x) 
        {
			ivec2 sampleCoord = pixelCoords + ivec2(x,y) * u_StepSize;
			sampleCoord %= dims;
            float sampleValue = imageLoad(u_Input, sampleCoord).r;
			value += sampleValue * u_Kernel[(x + extents.x) + (y + extents.y) * u_KernelSize.x];
		}
	}

	value /= u_Divisor;

	imageStore(u_Output, pixelCoords, vec4(value, 0.0, 0.0, 1.0));
}
