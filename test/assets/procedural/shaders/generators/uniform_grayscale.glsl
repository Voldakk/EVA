//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform writeonly image2D u_Output;

uniform float u_Value;

void main()
{
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    imageStore(u_Output, pixelCoords, vec4(u_Value, 0, 0, 1));
}
