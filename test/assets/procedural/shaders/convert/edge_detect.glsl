//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0) uniform writeonly image2D u_Output;

uniform sampler2D u_InputMapIn;

uniform float u_Threshold;

float run(ivec2 centerCoords, ivec2 dims)
{
    float center = imageLoad(u_Input, centerCoords).r;

    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            ivec2 sampleCoord = centerCoords + ivec2(x, y);
            sampleCoord %= dims;
            float other = imageLoad(u_Input, sampleCoord).r;
            if (distance(other, center) > u_Threshold) { return 1; }
        }
    }
    return 0;
}

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv                 = vec2(pixelCoords) / vec2(dims);

    float value = run(pixelCoords, dims);

    imageStore(u_Output, pixelCoords, vec4(value, 0.0, 0.0, 1.0));
}
