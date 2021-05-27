//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform writeonly image2D u_Output;

uniform float u_Step;
uniform int u_LabelCount;
layout(std430, binding = 1) buffer labelBuffer 
{ 
    int labels[]; 
};

uniform int u_ExtentsCount;
layout(std430, binding = 2) buffer extentsBuffer 
{ 
    ivec4 extents[]; 
};

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
	vec4 values = vec4(0, 0, 0, 1);

	int l = labels[pos.x + pos.y * dims.x];
	if(l > 0)
	{
		values.z = l * u_Step;

		ivec4 e = extents[l-1];
		int minX = e.x; 
		int maxX = e.y; 
		int minY = e.z; 
		int maxY = e.w; 

		values.x = float(pos.x - minX) / float(maxX - minX);
		values.y = float(pos.y - minY) / float(maxY - minY);
	}

	imageStore(u_Output, pos, values);
}
