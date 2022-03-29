//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform writeonly image2D u_Output;
layout(binding = 1, rgba32f) uniform readonly image2D u_Input;

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    vec4 inPixel  = imageLoad(u_Input, pixelCoords);
    vec4 outPixel = vec4(0);

    // Do work

    imageStore(u_Output, pixelCoords, outPixel);
}
