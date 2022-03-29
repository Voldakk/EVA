//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform writeonly image2D u_Output;

uniform int u_Shape;

uniform ivec2 u_Count;
uniform float u_Scale;

uniform vec2 u_Size;
uniform float u_Angle;

uniform float u_Param;

#define SQRT2PI 2.506628

#define SHAPE_SQUARE   0
#define SHAPE_DISC     1
#define SHAPE_GAUSSIAN 2


float Square(vec2 uv) { return (uv.x > u_Size.x || uv.x < -u_Size.x || uv.y > u_Size.y || uv.y < -u_Size.y) ? 0 : 1; }

float Disc(vec2 uv)
{
    float p = (pow(uv.x, 2) / pow(u_Size.x, 2)) + (pow(uv.y, 2) / pow(u_Size.y, 2));
    return p <= 1 ? 1 : 0;
}

float Gaussian(vec2 uv)
{
    float x  = (pow(uv.x, 2) / pow(u_Size.x, 2)) + (pow(uv.y, 2) / pow(u_Size.y, 2));
    float fx = (1 / (u_Param * SQRT2PI)) * exp(-0.5 * pow(x / u_Param, 2));
    float f0 = (1 / (u_Param * SQRT2PI)) * exp(-0.5 * pow(0 / u_Param, 2));
    return fx / f0;
}

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv                 = vec2(pixelCoords) / vec2(dims);

    uv *= u_Count;
    uv = fract(uv);
    uv = uv * 2 - 1;
    uv /= u_Scale;

    uv *= mat2(
        cos(u_Angle), sin(u_Angle), 
        -sin(u_Angle), cos(u_Angle)
    );

    float value = 0;

    switch (u_Shape)
    {
        case SHAPE_SQUARE: value = Square(uv); break;
        case SHAPE_DISC: value = Disc(uv); break;
        case SHAPE_GAUSSIAN: value = Gaussian(uv); break;
    }

    imageStore(u_Output, pixelCoords, vec4(value));
}