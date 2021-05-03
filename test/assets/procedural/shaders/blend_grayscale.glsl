//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform writeonly image2D u_Output;
layout(binding = 1, r32f) uniform readonly image2D u_InputA;
layout(binding = 2, r32f) uniform readonly image2D u_InputB;

uniform float u_Opacity = 0.5;
uniform int u_BlendMode = 0;

#define MODE_COPY 0
#define MODE_ADD 1
#define MODE_SUB 2
#define MODE_MUL 3
#define MODE_DIV 4
#define MODE_MIN 5
#define MODE_MAX 6

#define EPS 0.000001

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	vec4 outPixel = vec4(1);
	float a = imageLoad(u_InputA, pixelCoords).r;
	float b = imageLoad(u_InputB, pixelCoords).r;

	// Do work
	float value = 0;
	switch(u_BlendMode)
	{
		case MODE_COPY: value = mix(a, b, u_Opacity); break;
		case MODE_ADD: value = mix(a, a + b, u_Opacity); break;
		case MODE_SUB: value = mix(a, a - b, u_Opacity); break;
		case MODE_MUL: value = mix(a, a * b, u_Opacity); break;
		case MODE_DIV: value = mix(a, a / (b + EPS), u_Opacity); break;
		case MODE_MIN: value = mix(a, min(a, b), u_Opacity); break;
		case MODE_MAX: value = mix(a, max(a, b), u_Opacity); break;
	}

	value = clamp(value, 0.0, 1.0);
	outPixel.r = value;
	imageStore(u_Output, pixelCoords, outPixel);
}
