//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rg32f) uniform restrict writeonly image2D u_Output;
layout(binding = 1, rgba32f) uniform restrict readonly image2D u_ExtentsMap;

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    vec2 position = vec2(0);

    vec4 extents = imageLoad(u_ExtentsMap, pixelCoords).rgba;
    if (extents.z > 0)
    {
        vec2 minE = extents.xy;
        vec2 maxE = extents.zw;

        vec2 center = minE + (maxE - minE) * 0.5;
        position    = center;
    }

    imageStore(u_Output, pixelCoords, vec4(position, 0.0, 1.0));
}
