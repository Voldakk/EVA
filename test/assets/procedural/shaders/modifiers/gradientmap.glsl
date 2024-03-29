//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0) uniform writeonly image2D u_Output;

uniform sampler2D u_InputMapIn;

uniform struct Mark
{
    float position;
    vec4 color;
} u_Marks[10];
uniform int u_Count;

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv                 = vec2(pixelCoords) / vec2(dims);

    vec4 inPixel  = texture(u_InputMapIn, uv);
    vec4 outPixel = vec4(0, 0, 0, 1);
    float value   = inPixel.r;

    for (int i = 0; i < u_Count; ++i)
    {
        if (value <= u_Marks[i].position)
        {
            int prev   = max(0, i - 1);
            float diff = u_Marks[i].position - u_Marks[prev].position;
            float k    = (diff == 0.0) ? 0.0 : (value - u_Marks[prev].position) / diff;
            outPixel   = mix(u_Marks[prev].color, u_Marks[i].color, k);
            break;
        }
    }

    if (value > u_Marks[u_Count - 1].position) { outPixel = u_Marks[u_Count - 1].color; }

    imageStore(u_Output, pixelCoords, outPixel);
}
