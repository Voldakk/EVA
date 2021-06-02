//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform restrict writeonly image2D u_Output;
layout(binding = 1, r32ui) uniform restrict readonly uimage2D u_Labels;

layout(std430, binding = 2) restrict readonly buffer extentsBuffer 
{ 
    ivec4 extents[]; 
};

void main()
{
	ivec2 dims = imageSize(u_Labels);
	const ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
	vec4 values = vec4(0);

	uint l = imageLoad(u_Labels, pos).r;

	if(l > 0)
	{
		ivec4 e = extents[l-1];
		values = vec4(e) / vec4(dims, dims);
	}

	imageStore(u_Output, pos, values);
}
