//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform restrict writeonly image2D u_Output;
layout(binding = 1, rgba32f) uniform restrict readonly image2D u_ExtentsMap;

uniform sampler2D u_MapSampler;

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

        vec2 uv = vec2(pixelCoords) / vec2(dims);
        if (uv.x < minE.x) { uv.x += 1; }
        if (uv.y < minE.y) { uv.y += 1; }

        uv    = (uv - minE) / (maxE - minE);
        value = texture(u_MapSampler, uv).r;
    }

    imageStore(u_Output, pixelCoords, vec4(value, 0.0, 0.0, 1.0));
}
