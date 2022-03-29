//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0) uniform writeonly image2D u_Output;

uniform float u_Scale;
uniform vec2 u_Position;
uniform float u_Exponent;

vec2 rand(vec2 p)
{
    p = mod(p, vec2(u_Scale));
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

float GetDistance(vec2 pos)
{
    vec2 index = floor(pos);
    vec2 uv    = fract(pos);

    float minDist = 10.;

    for (int j = -1; j <= 1; j++)
    {
        for (int i = -1; i <= 1; i++)
        {
            vec2 neighbor = vec2(float(i), float(j));
            vec2 point    = rand(index + neighbor);
            vec2 diff     = neighbor + point - uv;
            float dist    = length(diff);

            minDist = min(minDist, dist);
        }
    }

    return minDist;
}

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(pixelCoords) / vec2(dims);
    uv += u_Position;

    float dist = GetDistance(uv * u_Scale);

    float value = 1.0 - pow(dist, u_Exponent);

    imageStore(u_Output, pixelCoords, vec4(vec3(value), 1));
}
