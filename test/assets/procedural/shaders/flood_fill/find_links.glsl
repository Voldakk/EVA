//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;

layout(binding = 0, r32ui) uniform restrict readonly uimage2D u_Labels;

layout(std430, binding = 1) restrict buffer linksBuffer 
{ 
     uint links[]; 
};

void main()
{
	const ivec2 dims = imageSize(u_Labels);
	const ivec2 index = ivec2(gl_GlobalInvocationID.xy);
    const int pixelsPerThread = 32;

	ivec2 minPos = index * pixelsPerThread;
	ivec2 maxPos = minPos + pixelsPerThread - 1;
	ivec2 negPos = (minPos - 1) % dims;

	for (int x = minPos.x; x <= maxPos.x; x++)
    {
		uint lMin = imageLoad(u_Labels, ivec2(x, minPos.y)).r;
		uint lNeg = imageLoad(u_Labels, ivec2(x, negPos.y)).r;

		uint minl = min(lMin, lNeg);
		if(minl != 0)
		{
			atomicMin(links[lMin], minl);
			atomicMin(links[lNeg], minl);
		}
	}

	for (int y = minPos.y; y <= maxPos.y; y++)
    {
		uint lMin = imageLoad(u_Labels, ivec2(minPos.x, y)).r;
		uint lNeg = imageLoad(u_Labels, ivec2(negPos.x, y)).r;

		uint minl = min(lMin, lNeg);
		if(minl != 0)
		{
			atomicMin(links[lMin], minl);
			atomicMin(links[lNeg], minl);
		}
	}
}
