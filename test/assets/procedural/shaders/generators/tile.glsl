//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform writeonly image2D u_Output;

uniform ivec2 u_Count;
uniform vec2 u_Height;

uniform float u_Scale;
uniform float u_ScaleVariation;

uniform float u_Offset;
uniform float u_OffsetVariation;

uniform float u_RandomMask;

uniform float u_Seed;

float rand(vec2 n) { return fract(sin(dot(vec3(n, u_Seed), vec3(12.9898, 4.1414, 3.1415))) * 43758.5453); }

float square(vec2 uv, ivec2 index)
{
    float scale = u_Scale * (1 - u_ScaleVariation * rand(index));
    vec2 gap    = vec2((1 - scale) / 2.0);

    vec2 tl = gap;
    vec2 br = 1.0 - gap;

    vec2 d   = -max(tl - uv, uv - br);
    float dd = min(d.x, d.y);
    if (dd > 0.0) { dd = 1.0; }

    float h = mix(u_Height.x, u_Height.y, rand(vec2(index) * 1.12351));

    return clamp(dd * h, 0.0, 1.0);
}

float tile(vec2 uv, ivec2 index) { return square(uv, index); }

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv                 = vec2(pixelCoords) / vec2(dims);


    uv *= u_Count;
    ivec2 index = ivec2(mod(ivec2(floor(uv)), u_Count));

    float offset = u_Offset * (1 - u_OffsetVariation * rand(index.yy));
    uv += vec2(floor(uv.y) * offset, 0.0);

    index = ivec2(mod(ivec2(floor(uv)), u_Count));
    uv    = fract(uv);

    float value = 0;
    if (rand(index) >= u_RandomMask) { value = tile(uv, index); }

    imageStore(u_Output, pixelCoords, vec4(value));
}
