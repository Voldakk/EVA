//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0) uniform writeonly image2D u_Output;

uniform float u_Scale   = 10.0;
uniform vec2 u_Position = vec2(0);
uniform int u_Octaves   = 4;

// The MIT License
// Copyright © 2013 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE
// IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


vec2 grad(ivec2 z) // replace this anything that returns a random vector
{
    z = ivec2(mod(vec2(z), u_Scale));
    // 2D to 1D  (feel free to replace by some other)
    int n = z.x + z.y * 11111;

    // Hugo Elias hash (feel free to replace by another one)
    n = (n << 13) ^ n;
    n = (n * (n * n * 15731 + 789221) + 1376312589) >> 16;

    // Perlin style vectors
    n &= 7;
    vec2 gr = vec2(n & 1, n >> 1) * 2.0 - 1.0;
    return (n >= 6) ? vec2(0.0, gr.x) : (n >= 4) ? vec2(gr.x, 0.0) : gr;
}

float noise(in vec2 p)
{
    ivec2 i = ivec2(floor(p));
    vec2 f  = fract(p);

    vec2 u = f * f * (3.0 - 2.0 * f); // feel free to replace by a quintic smoothstep instead

    return mix(mix(dot(grad(i + ivec2(0, 0)), f - vec2(0.0, 0.0)), dot(grad(i + ivec2(1, 0)), f - vec2(1.0, 0.0)), u.x),
               mix(dot(grad(i + ivec2(0, 1)), f - vec2(0.0, 1.0)), dot(grad(i + ivec2(1, 1)), f - vec2(1.0, 1.0)), u.x), u.y);
}

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(pixelCoords) / vec2(dims);
    uv *= u_Scale;
    uv += u_Position;

    float value     = 0.0;
    float intensity = 0.5;
    float frequency = 1.0;
    for (int i = 0; i < u_Octaves; i++)
    {
        value += intensity * noise(uv * frequency);
        intensity *= 0.5;
        frequency *= 2;
    }

    value = 0.5 + 0.5 * value;

    imageStore(u_Output, pixelCoords, vec4(vec3(value), 1));
}
