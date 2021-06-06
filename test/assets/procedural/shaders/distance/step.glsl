//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rg32ui) uniform readonly uimage2D u_Input;
layout(binding = 1, rg32ui) uniform writeonly uimage2D u_Output;

uniform int u_StepSize;
uniform float u_MaxDistance;

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    const vec2 pos = vec2(pixelCoords) / vec2(dims);
	float bestDistance = 1000;
    uvec2 bestCoord = uvec2(0);

	for (int y = -1; y <= 1; ++y) 
    {
        for (int x = -1; x <= 1; ++x) 
        {
            ivec2 sampleCoord = pixelCoords + ivec2(x,y) * u_StepSize;
            if(sampleCoord.x < 0 || sampleCoord.y < 0 || sampleCoord.x >= dims.x || sampleCoord.y >= dims.y) {continue;}
            uvec2 seedCoord = imageLoad(u_Input, sampleCoord).rg;

            float dist = distance(vec2(seedCoord) / dims, pos);
            if ((seedCoord.x != 0 || seedCoord.y != 0) && dist < bestDistance && dist <= u_MaxDistance)
            {
                bestDistance = dist;
                bestCoord = seedCoord;
            }
        }
    }

	imageStore(u_Output, pixelCoords, uvec4(bestCoord, 0, 0));
}
