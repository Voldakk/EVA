//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform writeonly image2D u_Output;
layout(binding = 1, r32f) readonly uniform image2D u_Input;

uniform float u_Strength;

const ivec3 eps = ivec3(-1, 0, 1);

void main()
{
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	// z0 -- z1 -- z2
	// |	 |     |
	// z3 -- h  -- z4
	// |     |     |
	// z5 -- z6 -- z7
	
	float z0 = imageLoad(u_Input, pixelCoords + eps.xx).r;
	float z1 = imageLoad(u_Input, pixelCoords + eps.yx).r;
	float z2 = imageLoad(u_Input, pixelCoords + eps.zx).r;

	float z3 = imageLoad(u_Input, pixelCoords + eps.xy).r;
	float z4 = imageLoad(u_Input, pixelCoords + eps.zy).r;

	float z5 = imageLoad(u_Input, pixelCoords + eps.xz).r;
	float z6 = imageLoad(u_Input, pixelCoords + eps.yz).r;
	float z7 = imageLoad(u_Input, pixelCoords + eps.zz).r;
	 
	// Sobel Filter
	vec3 normal;
	normal.z = 1.0 / u_Strength;
	normal.x = z0 + 2*z3 + z5 - z2 - 2*z4 - z7;
	normal.y = z0 + 2*z1 + z2 -z5 - 2*z6 - z7;
	normal = normalize(normal);

	imageStore(u_Output, pixelCoords, vec4(normal * 0.5 + 0.5, 1.0));
}
