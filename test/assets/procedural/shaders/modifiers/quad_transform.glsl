//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform writeonly image2D u_Output;

uniform sampler2D u_Input;

uniform vec4 a;
uniform vec4 b;

void main()
{
    const ivec2 dims        = imageSize(u_Output);
    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv                 = vec2(pixelCoords) / vec2(dims);

    float x = uv.x;
    float y = uv.y;

    float aa = a[3] * b[2] - a[2] * b[3];
    float bb = a[3] * b[0] - a[0] * b[3] + a[1] * b[2] - a[2] * b[1] + x * b[3] - y * a[3];
    float cc = a[1] * b[0] - a[0] * b[1] + x * b[1] - y * a[1];

    float det = sqrt(bb * bb - 4 * aa * cc);
    float m   = (-bb - det) / (2 * aa);

    float l = (x - a[0] - a[2] * m) / (a[1] + a[3] * m);

    vec3 value = texture(u_Input, vec2(l, m)).rgb;

    if (l < 0 || l > 1 || m < 0 || m > 1) { value = vec3(0); }

    imageStore(u_Output, pixelCoords, vec4(value, 1));
}
