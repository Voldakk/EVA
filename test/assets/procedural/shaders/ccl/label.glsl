//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
//layout(binding = 0, r32f) uniform restrict readonly image2D u_Input;
layout(binding = 1, r32ui) uniform restrict writeonly uimage2D u_Output;

uniform sampler2D u_Input;

layout(std430, binding = 2) restrict buffer indexBuffer
{
    uint u_NextIndex;
};

uniform float u_Threshold;

vec2 uv(int x, int y, ivec2 dims)
{
    return vec2(x, y) / vec2(dims);
}

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 index = ivec2(gl_GlobalInvocationID.xy);

    const uint pixelsPerThread = 32;
    const uint size = pixelsPerThread * pixelsPerThread;
    const uint maxLabel = size / 2;
    uint nextLabel = 1;

    uint links[maxLabel + 1]; 
    for(int i = 0; i < maxLabel + 1; i++) { links[i] = i; }
    uint localLabels[pixelsPerThread * pixelsPerThread];

    for (int localY = 0; localY < pixelsPerThread; localY++)
    {
        for (int localX = 0; localX < pixelsPerThread; localX++)
        {
            int x = localX + index.x * int(pixelsPerThread);
            int y = localY + index.y * int(pixelsPerThread);

            float value = texture(u_Input, uv(x, y, dims)).r;

            if (value > u_Threshold)
            {
                float valueX = (localX - 1) < 0 ? 0.0 : texture(u_Input, uv(x-1, y, dims)).r;
                uint labelX  = maxLabel;
                if (valueX > u_Threshold) { labelX = localLabels[(localX - 1) + localY * pixelsPerThread]; }

                float valueY = (localY - 1) < 0 ? 0.0 : texture(u_Input, uv(x, y-1, dims)).r;
                uint labelY  = maxLabel;
                if (valueY > u_Threshold) { labelY = localLabels[localX + (localY - 1) * pixelsPerThread]; }

                if (labelX == maxLabel && labelY == maxLabel)
                { 
                    localLabels[localX + localY * pixelsPerThread] = nextLabel++;
                }
                else
                {
                    uint low  = min(labelX, labelY);
                    uint high = max(labelX, labelY);

                    localLabels[localX + localY * pixelsPerThread] = low;

                    links[high] = min(low, links[high]);
                }
            }
            else
            {
                localLabels[localX + localY * pixelsPerThread] = 0;
            }
        }
    }

    for (uint i = 1; i < nextLabel; i++) 
    {
        uint l = links[i];
        if(l == i)
        {
            links[i] = atomicAdd(u_NextIndex, 1);
        }
        else
        {
            links[i] = links[l];
        }
    }
    links[0] = 0;

    for (int localY = 0; localY < pixelsPerThread; localY++)
    {
        for (int localX = 0; localX < pixelsPerThread; localX++) 
        {
            int x = localX + index.x * int(pixelsPerThread);
            int y = localY + index.y * int(pixelsPerThread);
            uint value = links[localLabels[localX + localY * pixelsPerThread]];
            imageStore(u_Output, ivec2(x, y), uvec4(value));
        }
    }
}
