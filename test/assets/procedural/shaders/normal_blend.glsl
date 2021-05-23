//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform writeonly image2D u_Output;
layout(binding = 1, rgba32f) uniform readonly image2D u_InputA;
layout(binding = 2, rgba32f) uniform readonly image2D u_InputB;
layout(binding = 3, r32f) uniform readonly image2D u_OpacityMap;

uniform float u_Opacity;

vec3 blend_linear(vec4 n1, vec4 n2)
{
    vec4 r = (n1 + n2)*2 - 2;
    return normalize(r.xyz);
}

vec3 blend_pd(vec4 n1, vec4 n2)
{
    n1 = n1*2 - 1;
    n2 = n2.xyzz*vec4(2, 2, 2, 0) + vec4(-1, -1, -1, 0);
    vec3 r = n1.xyz*n2.z + n2.xyw*n1.z;
    return normalize(r);
}

vec3 blend_whiteout(vec4 n1, vec4 n2)
{
    n1 = n1*2 - 1;
    n2 = n2*2 - 1;
    vec3 r = vec3(n1.xy + n2.xy, n1.z*n2.z);
    return normalize(r);
}

vec3 blend_udn(vec4 n1, vec4 n2)
{
    vec3 c = vec3(2, 1, 0);
    vec3 r;
    r = n2.xyz*c.yyz + n1.xyz;
    r =  r*c.xxx -  c.xxy;
    return normalize(r);
}

vec3 blend_rnm(vec4 n1, vec4 n2)
{
    vec3 t = n1.xyz*vec3( 2,  2, 2) + vec3(-1, -1,  0);
    vec3 u = n2.xyz*vec3(-2, -2, 2) + vec3( 1,  1, -1);
    vec3 r = t*dot(t, u) - u*t.z;
    return normalize(r);
}

vec3 blend_unity(vec4 n1, vec4 n2)
{
    n1 = n1.xyzz*vec4(2, 2, 2, -2) + vec4(-1, -1, -1, 1);
    n2 = n2*2 - 1;
    vec3 r;
    r.x = dot(n1.zxx,  n2.xyz);
    r.y = dot(n1.yzy,  n2.xyz);
    r.z = dot(n1.xyw, -n2.xyz);
    return normalize(r);
}

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	vec4 a = imageLoad(u_InputA, pixelCoords);
	vec4 b = imageLoad(u_InputB, pixelCoords);
	float opacity = u_Opacity * imageLoad(u_OpacityMap, pixelCoords).r;

	vec3 r = blend_rnm(a, b);
	r = normalize(r) * 0.5 + 0.5;
    r = mix(a.xyz, r, opacity);
	imageStore(u_Output, pixelCoords, vec4(r, 1));
}
