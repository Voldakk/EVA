//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;

layout(binding = 0, r32ui) uniform restrict uimage2D u_Labels;

layout(std430, binding = 1) restrict readonly buffer linksBuffer 
{ 
     uint links[]; 
};

void main()
{
	const ivec2 pos = ivec2(gl_GlobalInvocationID.xy);

	uint l = imageLoad(u_Labels, pos).r;
	imageStore(u_Labels, pos, uvec4(links[l]));
}
