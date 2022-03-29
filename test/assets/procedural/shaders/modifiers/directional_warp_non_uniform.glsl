//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0) uniform writeonly image2D u_Output;

uniform sampler2D u_InputMapIn;
uniform sampler2D u_InputMapIntensity;
uniform sampler2D u_InputMapAngle;

uniform float u_Angle;
uniform float u_AngleMultiplier;
uniform float u_Intensity;

const ivec3 eps = ivec3(-1, 0, 1);

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv                 = vec2(pixelCoords) / vec2(dims);

    float intensity = texture(u_InputMapIntensity, uv).r * u_Intensity;
    float angle     = radians(u_Angle + mix(0, 360, (texture(u_InputMapAngle, uv).r * u_AngleMultiplier)));

    float dist = intensity;
    vec2 dir   = normalize(vec2(cos(angle), sin(angle)));

    uv += dir * dist;

    float value = texture(u_InputMapIn, uv).r;

    imageStore(u_Output, pixelCoords, vec4(value, 0.0, 0.0, 1.0));
}
