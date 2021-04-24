//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

#define INF 1e20
#define PI 3.1415926535

#define MAX_OBJECTS 100

#define ENABLE_SHADOWS 0
#define ENABLE_GROUND_PLANE 0
#define ENABLE_SPHERES 0

layout(local_size_variable) in;
layout(binding = 0, rgba8) uniform writeonly image2D imgOutput;

uniform int objectBufferCount;
layout(std430, binding = 1) buffer sphereBuffer 
{ 
    vec4 spheresSSBO[]; 
};

uniform sampler2D u_FbColor;
uniform sampler2D u_FbDepth;
uniform mat4 u_ViewProjection;

uniform float u_Time;
uniform sampler2D u_EnvMap;

uniform float u_CameraNear;
uniform float u_CameraFar;

uniform float u_SurfaceDist;
uniform float u_MaxDist;
uniform int u_MaxSteps;
uniform float u_NormalEps;

uniform float u_AoIntensity;
uniform float u_AoStepSize;
uniform int u_AoSteps;


uniform bool u_MandelbulbEnable;
uniform int u_MandelbulbIterations;
uniform float u_MandelbulbPower;
uniform float u_MandelbulbScale;

uniform bool u_TetrahedronEnable;
uniform int u_TetrahedronIterations;
uniform float u_TetrahedronScale;
uniform float u_TetrahedronOffset;

shared vec4 sphereData[MAX_OBJECTS];

void bufferShared()
{
    #if ENABLE_SPHERES
    uint id  = gl_LocalInvocationID.x;
    if(id < objectBufferCount) 
    {
        sphereData[id] = spheresSSBO[id]; 
    }
    memoryBarrierShared();
    #endif
}

float linearizeDepth(float depth, float near, float far) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}


const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 sampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float opSubtraction( float d1, float d2 ) 
{ 
    return max(-d1, d2); 
}

float opSmoothSubtraction( float d1, float d2, float k ) 
{
    float h = clamp( 0.5 - 0.5 * (d2 + d1) / k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k * h * (1.0 - h); 
}

float sdMandelbulb(vec3 point) 
{
    vec4 p = vec4(point, 0) / u_MandelbulbScale;
	vec4 z = p;
	float dr = 1.0;
	float r = 0.0;
	for (int i = 0; i < u_MandelbulbIterations; i++) 
    {
		r = length(z);
		if (r > 10) break;

		float theta = acos(z.z / r);
		float phi = atan(z.y, z.x);
		dr = pow(r, u_MandelbulbPower - 1.0) * u_MandelbulbPower * dr + 1.0;

		float zr = pow(r, u_MandelbulbPower);
		theta = theta * u_MandelbulbPower;
		phi = phi * u_MandelbulbPower;

		z = zr * vec4(
			sin(theta) * cos(phi),
			sin(phi) * sin(theta),
			cos(theta),
			1.0);
		z += p;
	}
	return u_MandelbulbScale * 0.5 * log(r) * r / dr;
}

float sdTetrahedron(vec3 z)
{
	float r;
    int n = 0;
    while (n < u_TetrahedronIterations) 
    {
       if(z.x + z.y < 0) z.xy = -z.yx;
       if(z.x + z.z < 0) z.xz = -z.zx;
       if(z.y + z.z < 0) z.zy = -z.yz;
       z = z * u_TetrahedronScale - u_TetrahedronOffset * (u_TetrahedronScale - 1.0);
       n++;
    }
    return length(z) * pow(u_TetrahedronScale, -float(n));
}

float getDist(vec3 point)
{
    float minDist = INF;

    if(u_MandelbulbEnable) minDist = min(minDist, sdMandelbulb(point));
    if(u_TetrahedronEnable) minDist = min(minDist, sdTetrahedron(point));

    #if ENABLE_SPHERES
    float sphereDist = INF;
    for (int i = 0; i < objectBufferCount; i++)
    {
        float dist = length(point - sphereData[i].xyz) - sphereData[i].w;
        sphereDist = min(sphereDist, dist);
    }
    minDist = opSmoothSubtraction(sphereDist, minDist, 0.5);
    #endif

    #if ENABLE_GROUND_PLANE
    minDist = min(minDist, point.y);
    #endif


    return minDist;
}

float rayMarch(vec3 ro, vec3 rd)
{
    float dist = 0;
    for (int i = 0; i < u_MaxSteps; i++)
    {
        vec3 point = ro + rd * dist;

        float distToSurface = getDist(point);
        dist += distToSurface;

        if(distToSurface < u_SurfaceDist || dist > u_MaxDist) 
            break;
    }
    return dist;
}

const vec2 e = vec2(u_NormalEps, 0);
vec3 getNormal(vec3 point)
{
    float dist = getDist(point);
    vec3 normal = dist - vec3(getDist(point - e.xyy),
                              getDist(point - e.yxy),
                              getDist(point - e.yyx));
    return normalize(normal);
}

float getAO(vec3 point, vec3 normal)
{
    float ao = 0;

    for(int i = 1; i <= u_AoSteps; ++i)
    {
        float dist = i * u_AoStepSize;
        float aoDist = getDist(point + normal * dist);
        ao += (1/pow(2, i)) * (dist - aoDist);
    }

    return 1 - ao * u_AoIntensity;
}

float getLight(vec3 point, vec3 normal)
{
    vec3 lightPos = vec3(2, 5, -6);
    //lightPos.xz += vec2(sin(time), cos(time)) * 15;
    vec3 pointToLight = normalize(lightPos - point);

    float diffuse = max(0, dot(normal, pointToLight));

    // Shadow
    #if ENABLE_SHADOWS
    if(diffuse > 0) {
        float d = rayMarch(point + normal * u_SurfaceDist * 2, pointToLight);
        if(d < length(pointToLight)) diffuse *= 0.1;
    }
    #endif

    return diffuse;
}

void main()
{
    bufferShared();

    // Get the current pixel and uv coordinates
    const ivec2 dims   = imageSize(imgOutput);
    const float aspect = float(dims.x) / float(dims.y);

    const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.x % dims.x, gl_GlobalInvocationID.x / dims.x);
    if(pixelCoords.x >= dims.x || pixelCoords.y >= dims.y) 
        return;

    vec2 uv = vec2(pixelCoords.xy) / vec2(dims);

    // Base pixel colour
    vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);

    // Copy from frame buffer
    vec3 fbColor = texture(u_FbColor, uv).rgb;
    pixel.rgb = fbColor;

    float fbDepth = texture(u_FbDepth, uv).r;
    fbDepth = linearizeDepth(fbDepth, u_CameraNear, u_CameraFar);

    // Camera
    uv = uv * 2 - 1;
    mat4 m = inverse(u_ViewProjection);
    vec4 rayStart = vec4(uv, -1, 1);
    vec4 rayEnd = vec4(uv, 0, 1);
    rayStart = m * rayStart;
    rayEnd = m * rayEnd;
    rayStart /= rayStart.w;
    rayEnd /= rayEnd.w;

    vec3 ro = rayStart.xyz;
    vec3 rd = normalize(rayEnd - rayStart).xyz;
    
    // Ray march
    float dist = rayMarch(ro, rd);

    if(dist <= u_MaxDist && dist < fbDepth)
    {
        vec3 point = ro + rd * dist;
        vec3 normal = getNormal(point);

        float diffuse = getLight(point, normal);
        float ao = getAO(point, normal);
        pixel.rgb = vec3(diffuse * ao);
    }
    else if(fbDepth >= u_CameraFar)
    {
        vec2 uvEnv = sampleSphericalMap(rd);
        pixel.rgb = texture(u_EnvMap, uvEnv).rgb;
    }

    imageStore(imgOutput, pixelCoords, pixel);
}
