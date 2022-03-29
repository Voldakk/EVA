//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0) uniform writeonly image2D u_Output;

uniform sampler2D u_InputMapIn;

uniform int u_Steps;
uniform int u_StepSize;

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    const vec2 uv           = vec2(pixelCoords) / vec2(dims);

    int samples = 0;
    float value = 0;

    for (int x = -u_Steps; x <= u_Steps; x++)
    {
        int yV = int(sin(acos(float(x) / float(u_Steps))) * u_Steps);
        for (int y = -yV; y <= yV; y++)
        {
            value -= imageLoad(u_Input, clamp(pixelCoords + ivec2(x, y) * u_StepSize, ivec2(0), dims - 1)).r;
            samples++;
        }
    }
    value = value / samples + imageLoad(u_Input, pixelCoords).r;

    value = value * 0.5 + 0.5;
    imageStore(u_Output, pixelCoords, vec4(value, 0, 0, 1));
}
