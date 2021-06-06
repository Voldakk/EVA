//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform writeonly image2D u_Output;

uniform ivec2 u_NumBricks = ivec2(4, 8);
uniform float u_Offset = 0.5;
uniform vec2 u_Gap = vec2(0.1, 0.1);
uniform vec2 u_Bevel = vec2(0.1, 0.1);
uniform vec2 u_Height = vec2(0.7, 1.0);

float rand(vec2 n) 
{ 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float brick(vec2 uv)
{
    uv *= u_NumBricks;
    uv += vec2(floor(uv.y) * u_Offset, 0.0);
    
    vec2 index = vec2(mod(ivec2(floor(uv)), u_NumBricks));
    uv = fract(uv);
    
    vec2 gap = u_Gap * u_NumBricks;
    vec2 bevel = u_Bevel * u_NumBricks;
    
    vec2 tl = gap;
    vec2 br = 1.0 - gap;
    
    vec2 d = -max(tl-uv, uv-br);
    
    d = min(d, bevel) * (1.0/bevel);
    
    float dd = min(d.x, d.y);
    
    float h = mix(u_Height.x, u_Height.y, rand(index));
    
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
