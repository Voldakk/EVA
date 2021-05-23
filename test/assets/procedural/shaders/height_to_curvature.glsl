//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform writeonly image2D u_Output;
layout(binding = 1, r32f) uniform readonly image2D u_Input;

uniform int u_Steps;
uniform int u_StepSize;

void main()
{
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	int samples = 0;
	float value = 0;

	for(int x = -u_Steps; x <= u_Steps; x++)
	{
		int yV = int(sin(acos(float(x) / float(u_Steps))) * u_Steps);
		for(int y = -yV; y <= yV; y++)
		{
			value -= imageLoad(u_Input, pixelCoords + ivec2(x, y) * u_StepSize).r;
			samples++;
		}
	}
	value = value / samples + imageLoad(u_Input, pixelCoords).r;

	value = value * 0.5 + 0.5;
	imageStore(u_Output, pixelCoords, vec4(value, 0, 0, 1));
}
