//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

#define PI 3.14159265359

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform restrict writeonly image2D u_Output;
layout(binding = 1, rgba32f) uniform restrict readonly image2D u_ExtentsMap;

uniform float u_Angle;
uniform float u_AngleVariation;
uniform float u_Seed;

float rand(vec3 n) { return fract(sin(dot(n, vec3(12.9898, 4.1414, 3.1415))) * 43758.5453); }

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    float value = 0;


    vec4 extents = imageLoad(u_ExtentsMap, pixelCoords).rgba;
    if (extents.z > 0)
    {
        vec2 minE = extents.xy;
        vec2 maxE = extents.zw;

        float r     = (rand(vec3(minE, u_Seed)) - 0.5) * u_AngleVariation;
        float angle = u_Angle + mix(-PI, PI, 0.5 + r);

        vec2 uv = vec2(pixelCoords) / vec2(dims);
        if (uv.x < minE.x) { uv.x += 1; }
        if (uv.y < minE.y) { uv.y += 1; }

        uv = (uv - minE) / (maxE - minE);
        uv -= 0.5;

        vec2 dir = vec2(cos(angle), sin(angle));
        vec2 p1  = dir * (1.0 / max(abs(dir.x), abs(dir.y)));
        dir      = -dir;
        vec2 p0  = dir * (1.0 / max(abs(dir.x), abs(dir.y)));

        vec2 v    = uv - p0;
        float d   = dot(v, dir);
        vec2 near = p0 + dir * d;

        float t = distance(p0, near) / distance(p0, p1);
        value   = t;
    }


    imageStore(u_Output, pixelCoords, vec4(value, 0.0, 0.0, 1.0));
}
