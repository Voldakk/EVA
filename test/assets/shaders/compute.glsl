//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

#define MAX_OBJECTS 1000

layout(local_size_variable) in;
layout(rgba8) uniform image2D imgOutput;
uniform int objectBufferCount;
layout(std430, binding = 1) buffer objectBuffer 
{ 
    vec2 data_SSBO[]; 
};

shared vec2 pointsData[MAX_OBJECTS];

void main()
{
    uint id  = gl_LocalInvocationID.x;
    if(id < objectBufferCount) 
    {
        pointsData[id] = data_SSBO[id]; 
    }

    memoryBarrierShared();

    // get index in global work group i.e x,y position
    ivec2 dims         = imageSize(imgOutput);
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.x % dims.x, gl_GlobalInvocationID.x / dims.x);
    ivec2 pos          = pixelCoords - dims / 2;
    vec2 uv            = vec2(pixelCoords.xy / dims);
    uv                 = uv * 2.0 - 1.0;

    // base pixel colour for image
    vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);

    for (int i = 0; i < objectBufferCount; i++)
    {
        if (length(pos - pointsData[i]) < 20.0) { pixel.r += 0.2; }
    }

    imageStore(imgOutput, pixelCoords, pixel);
}
