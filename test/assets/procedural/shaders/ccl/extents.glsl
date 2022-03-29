//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;

layout(binding = 0, r32ui) uniform restrict readonly uimage2D u_Labels;

struct Extents
{
    uint minX;
    uint minY;
    uint maxX;
    uint maxY;
};

layout(std430, binding = 1) restrict buffer extentsBuffer { Extents extents[]; };

void main()
{
    const ivec2 pos = ivec2(gl_GlobalInvocationID.xy);

    uint l = imageLoad(u_Labels, pos).r;

    if (l != 0)
    {
        atomicMin(extents[l - 1].minX, pos.x);
        atomicMax(extents[l - 1].maxX, pos.x);

        atomicMin(extents[l - 1].minY, pos.y);
        atomicMax(extents[l - 1].maxY, pos.y);
    }
}
