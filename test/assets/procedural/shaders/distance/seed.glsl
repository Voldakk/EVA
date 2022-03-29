//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform readonly image2D u_Input;
layout(binding = 1, rg32ui) uniform writeonly uimage2D u_Output;

uniform float u_Threshold;

void main()
{
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    float inPixel  = imageLoad(u_Input, pixelCoords).r;
    uvec4 outPixel = uvec4(0);

    if (inPixel > u_Threshold) { outPixel.rg = uvec2(pixelCoords); }

    imageStore(u_Output, pixelCoords, outPixel);
}
