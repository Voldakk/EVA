//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform writeonly image2D u_Output;
layout(binding = 1, rgba32f) readonly uniform image2D u_Input;

uniform ivec2 u_NumBricks = ivec2(4, 8);
uniform float u_Offset = 0.5;
uniform vec2 u_Gap = vec2(0.1, 0.1);
uniform vec2 u_Bevel = vec2(0.1, 0.1);
uniform vec2 u_Height = vec2(0.7, 1.0);

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}
float noise(vec2 p){
    p *= 1413.2371;
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}

float brick(vec2 uv)
{
    uv *= u_NumBricks;
    uv += vec2(floor(uv.y) * u_Offset, 0.0);
    
    vec2 index = floor(uv);
    uv = fract(uv);
    
    vec2 gap = u_Gap * u_NumBricks;
    vec2 bevel = u_Bevel * u_NumBricks;
    
    vec2 tl = gap;
    vec2 br = 1.0 - gap;
    
    vec2 d = -max(tl-uv, uv-br);
    
    d = min(d, bevel) * (1.0/bevel);
    
    float dd = min(d.x, d.y);
    
    float h = mix(u_Height.x, u_Height.y, noise(index));
    
    return dd * h;
}

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
	const vec2 uv = vec2(pixelCoords) / vec2(dims);

    float d = brick(uv);
    d = clamp(d, 0.0, 1.0);
    vec4 outPixel = vec4(d, 0, 0, 1);

	imageStore(u_Output, pixelCoords, outPixel);
}
