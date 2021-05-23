//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

layout(local_size_variable) in;
layout(binding = 0, r32f) uniform writeonly image2D u_Output;
layout(binding = 1, r32f) uniform readonly image2D u_Input;

uniform vec3 u_Samples[64];
uniform vec3 u_Noise[16];

const int kernelSize = 64;
const int noiseWidth = 4;

uniform float u_Radius;
uniform float u_Bias;

uniform float u_HeightScale;
uniform float u_NormalStrength;

vec3 SampleNormal(ivec2 pixelCoords)
{
	const ivec3 eps = ivec3(-1, 0, 1);
	float z0 = imageLoad(u_Input, pixelCoords + eps.xx).r;
	float z1 = imageLoad(u_Input, pixelCoords + eps.yx).r;
	float z2 = imageLoad(u_Input, pixelCoords + eps.zx).r;
	float z3 = imageLoad(u_Input, pixelCoords + eps.xy).r;
	float z4 = imageLoad(u_Input, pixelCoords + eps.zy).r;
	float z5 = imageLoad(u_Input, pixelCoords + eps.xz).r;
	float z6 = imageLoad(u_Input, pixelCoords + eps.yz).r;
	float z7 = imageLoad(u_Input, pixelCoords + eps.zz).r;
	vec3 normal;
	normal.z = 1.0 / u_NormalStrength;
	normal.x = z0 + 2*z3 + z5 - z2 - 2*z4 - z7;
	normal.y = z0 + 2*z1 + z2 -z5 - 2*z6 - z7;
	return normalize(normal);
}

vec3 SamplePos(ivec2 pixelCoords, ivec2 dims)
{
	float z = imageLoad(u_Input, pixelCoords).r * u_HeightScale;
	vec2 xy = vec2(pixelCoords) / vec2(dims);
	return vec3(xy.x, xy.y, z);
}

void main()
{
	const ivec2 dims = imageSize(u_Output);
	const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

	float height = imageLoad(u_Input, pixelCoords).r;
	vec4 outPixel = vec4(0);

	vec3 fragPos = SamplePos(pixelCoords, dims);
	vec3 normal = SampleNormal(pixelCoords);

	ivec2 p = pixelCoords % noiseWidth;
    vec3 randomVec = u_Noise[p.x + p.y * noiseWidth];

    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * u_Samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * u_Radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        //vec4 offset = vec4(samplePos, 1.0);
        //offset = projection * offset; // from view to clip-space
        //offset.xyz /= offset.w; // perspective divide
        //offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
		ivec2 sampleCoords = ivec2(samplePos.xy * dims);
        float sampleDepth = SamplePos(sampleCoords, dims).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, u_Radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + u_Bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);

	imageStore(u_Output, pixelCoords, vec4(occlusion, 0, 0, 1));
}
