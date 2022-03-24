//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;

layout(binding = 0, r32ui) uniform restrict readonly uimage2D u_Labels;

uniform bool u_Wrap;
uniform int u_NumLinkParts;

layout(std430, binding = 1) restrict buffer linksBuffer 
{ 
     uint links[]; 
};

uint GetPartOffset(uint label) 
{ 
	return label / 32; 
}

uint GetPart(uint label) 
{ 
    return 1 << (label % 32);
}

void main()
{
	const ivec2 dims = imageSize(u_Labels);
	const ivec2 index = ivec2(gl_GlobalInvocationID.xy);
    const int pixelsPerThread = 32;

	ivec2 minPos = index * pixelsPerThread;
	ivec2 maxPos = minPos + pixelsPerThread - 1;
	ivec2 negPos = (minPos - 1);

	if(u_Wrap) { negPos %= dims; }

	for (int x = minPos.x; x <= maxPos.x; x++)
    {
		uint lMin = imageLoad(u_Labels, ivec2(x, minPos.y)).r;
		uint lNeg = imageLoad(u_Labels, ivec2(x, negPos.y)).r;

		if (lMin != 0 && lNeg != 0)
		{
            atomicOr(links[u_NumLinkParts * lMin + GetPartOffset(lNeg)], GetPart(lNeg));
            atomicOr(links[u_NumLinkParts * lNeg + GetPartOffset(lMin)], GetPart(lMin));
		}
	}

	for (int y = minPos.y; y <= maxPos.y; y++)
    {
		uint lMin = imageLoad(u_Labels, ivec2(minPos.x, y)).r;
		uint lNeg = imageLoad(u_Labels, ivec2(negPos.x, y)).r;

        if (lMin != 0 && lNeg != 0)
		{
            atomicOr(links[u_NumLinkParts * lMin + GetPartOffset(lNeg)], GetPart(lNeg));
            atomicOr(links[u_NumLinkParts * lNeg + GetPartOffset(lMin)], GetPart(lMin));
		}
	}
}
