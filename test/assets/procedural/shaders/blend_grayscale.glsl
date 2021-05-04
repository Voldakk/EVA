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
#define MODE_SCREEN 7
#define MODE_OVERLAY 8
#define MODE_HARD_LIGHT 9
#define MODE_SOFT_LIGHT 10
#define MODE_DIFF 11
#define MODE_COLOR_DODGE 12
#define MODE_COLOR_BURN 13

#define EPS 0.000001

float Multiply(float b, float s)
{
	return b * s;
}

float Screen(float b, float s)
{
	return 1.0 - (1.0-b)*(1.0-s);
}

float HardLight(float b, float s)
{
	return s <= 0.5 ? Multiply(b, 2*s) : Screen(b, 2*s - 1);
}

float D(float x)
{
	return x <= 0.25 ? (((16 * x - 12) * x + 4) * x) : sqrt(x);
}

float SoftLight(float b, float s)
{
	return s <= 0.5 ? (b - (1 - 2*s) * b * (1 - b)) : (b + (2*s - 1) * (D(b) - b));
}

float ColorDodge(float b, float s)
{
	return s < 1.0 ? (min(1.0, b / (1-s))) : 1;
}

float ColorBurn(float b, float s)
{
	return s > 0.0 ? (1 - min(1, (1-b) / s)) : 0;
}

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
		case MODE_SCREEN: value = mix(a, Screen(b, a), u_Opacity); break;
		case MODE_OVERLAY: value = mix(a, HardLight(a, b), u_Opacity); break;
		case MODE_HARD_LIGHT: value = mix(a, HardLight(b, a), u_Opacity); break;
		case MODE_SOFT_LIGHT: value = mix(a, SoftLight(b, a), u_Opacity); break;
		case MODE_DIFF: value = mix(a, abs(a - b), u_Opacity); break;
		case MODE_COLOR_DODGE: value = mix(a, ColorDodge(b, a), u_Opacity); break;
		case MODE_COLOR_BURN: value = mix(a, ColorBurn(b, a), u_Opacity); break;
	}

	value = clamp(value, 0.0, 1.0);
	outPixel.r = value;
	imageStore(u_Output, pixelCoords, outPixel);
}
