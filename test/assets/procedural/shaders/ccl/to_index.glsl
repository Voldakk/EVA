//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform restrict writeonly image2D u_Output;
layout(binding = 1, r32ui) uniform restrict readonly uimage2D u_LabelMap;

uniform float u_Step;

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    uint label = imageLoad(u_LabelMap, pixelCoords).r;

    float value = float(label) * u_Step;

    imageStore(u_Output, pixelCoords, vec4(value, 0.0, 0.0, 1.0));
}
