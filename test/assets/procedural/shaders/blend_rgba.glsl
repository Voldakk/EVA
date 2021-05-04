//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform writeonly image2D u_Output;
layout(binding = 1, rgba32f) uniform readonly image2D u_InputA;
layout(binding = 2, rgba32f) uniform readonly image2D u_InputB;

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

vec3 Select(vec3 v1, vec3 v2, vec3 s, float t)
{
	vec3 res;
	res.r = s.r < t ? v1.r : v2.r;
	res.g = s.g < t ? v1.g : v2.g;
	res.b = s.b < t ? v1.b : v2.b;
	return res;
}

vec3 Multiply(vec3 b, vec3 s)
{
	return b * s;
}

vec3 Screen(vec3 b, vec3 s)
{
	return 1.0 - (1.0-b)*(1.0-s);
}

vec3 HardLight(vec3 b, vec3 s)
{
	vec3 v1 = Multiply(b, 2*s);
	vec3 v2 = Screen(b, 2*s - 1);
	return Select(v1, v2, s, 0.5);
}

vec3 D(vec3 x)
{
	vec3 v1 = (((16 * x - 12) * x + 4) * x);
	vec3 v2 = sqrt(x);
	return Select(v1, v2, x, 0.25);
}

vec3 SoftLight(vec3 b, vec3 s)
{
	vec3 v1 = (b - (1 - 2*s) * b * (1 - b));
	vec3 v2 = (b + (2*s - 1) * (D(b) - b));
	return Select(v1, v2, s, 0.5);
}

vec3 ColorDodge(vec3 b, vec3 s)
{
	vec3 v1 = (min(vec3(1.0), b / (1.0-s)));
	vec3 v2 = vec3(1.0);
	return Select(v1, v2, s, 1.0);
}

vec3 ColorBurn(vec3 b, vec3 s)
{
	vec3 v1 = (1 - min(vec3(1), (1-b) / s));
	vec3 v2 = vec3(0);

	return Select(v1, v2, 1 - s, 1.0);
}

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	vec4 outPixel = vec4(1);
	vec4 a = imageLoad(u_InputA, pixelCoords);
	vec4 b = imageLoad(u_InputB, pixelCoords);

	// Do work
	vec4 value = a;
	float opacity = b.a * u_Opacity;
	switch(u_BlendMode)
	{
		case MODE_COPY: value = mix(a, b, opacity); break;
		case MODE_ADD: value.rgb = mix(a.rgb, a.rgb + b.rgb, opacity); break;
		case MODE_SUB: value.rgb = mix(a.rgb, a.rgb - b.rgb, opacity); break;
		case MODE_MUL: value = mix(a, a * b, opacity); break;
		case MODE_DIV: value = mix(a, a / (b + EPS), opacity); break;
		case MODE_MIN: value = mix(a, min(a, b), opacity); break;
		case MODE_MAX: value = mix(a, max(a, b), opacity); break;
		case MODE_DIFF: value.rgb = mix(a.rgb, abs(a.rgb - b.rgb), opacity); break;
		case MODE_SCREEN: value.rgb = mix(a.rgb, Screen(b.rgb, a.rgb), opacity); break;
		case MODE_OVERLAY: value.rgb = mix(a.rgb, HardLight(a.rgb, b.rgb), opacity); break;
		case MODE_HARD_LIGHT: value.rgb = mix(a.rgb, HardLight(b.rgb, a.rgb), opacity); break;
		case MODE_SOFT_LIGHT: value.rgb = mix(a.rgb, SoftLight(b.rgb, a.rgb), opacity); break;
		case MODE_COLOR_DODGE: value.rgb = mix(a.rgb, ColorDodge(b.rgb, a.rgb), opacity); break;
		case MODE_COLOR_BURN: value.rgb = mix(a.rgb, ColorBurn(b.rgb, a.rgb), opacity); break;
	}

	value = clamp(value, 0.0, 1.0);
	outPixel = value;
	imageStore(u_Output, pixelCoords, outPixel);
}
