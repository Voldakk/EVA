//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform readonly image2D u_Texture;
layout(binding = 1, rg32ui) uniform readonly uimage2D u_Data;
layout(binding = 2, r32f) uniform writeonly image2D u_Output;

uniform bool u_Blend;
uniform float u_MaxDistance;

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	ivec2 seedCoord = ivec2(imageLoad(u_Data, pixelCoords).rg);
	vec4 outPixel = imageLoad(u_Texture, seedCoord);

	if(u_Blend)
	{
		vec2 pos = vec2(pixelCoords) / vec2(dims);

		vec2 seedPos = vec2(seedCoord) / dims;
           
        float dx = min(distance(pos.x, seedPos.x), distance(pos.x, seedPos.x + (pos.x > seedPos.x ? 1 : -1)));
        float dy = min(distance(pos.y, seedPos.y), distance(pos.y, seedPos.y + (pos.y > seedPos.y ? 1 : -1)));
		float dist = length(vec2(dx, dy));

		float t = dist / (u_MaxDistance + 0.0001);
		t = clamp(t, 0.0, 1.0);
		outPixel = mix(outPixel, vec4(0.0), t);
	}

	imageStore(u_Output, pixelCoords, outPixel);
}
