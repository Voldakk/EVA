//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0) uniform writeonly image2D u_Output;

uniform sampler2D u_InputMapIn;

uniform vec2 u_InputRange  = vec2(0.0, 1.0);
uniform vec2 u_OutputRange = vec2(0.0, 1.0);
uniform float u_Midtone    = 0.5;

uniform vec4 u_InputRangeMin  = vec4(0.0);
uniform vec4 u_InputRangeMax  = vec4(1.0);
uniform vec4 u_OutputRangeMin = vec4(0.0);
uniform vec4 u_OutputRangeMax = vec4(1.0);
uniform vec4 u_Midtones       = vec4(0.5);

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv                 = vec2(pixelCoords) / vec2(dims);

    vec4 value = texture(u_InputMapIn, uv);

    float gamma = (1 - u_Midtone) * 2;
    value.rgb   = (value.rgb - u_InputRange.x) / (u_InputRange.y - u_InputRange.x);
    value.rgb   = pow(value.rgb, vec3(gamma));
    value.rgb   = value.rgb * (u_OutputRange.y - u_OutputRange.x) + u_OutputRange.x;

    vec4 gammas = (1.0 - u_Midtones) * 2.0;
    value       = (value - u_InputRangeMin) / (u_InputRangeMax - u_InputRangeMin);
    value       = pow(value, gammas);
    value       = value * (u_OutputRangeMax - u_OutputRangeMin) + u_OutputRangeMin;

    value = clamp(value, 0.0, 1.0);
    imageStore(u_Output, pixelCoords, value);
}
