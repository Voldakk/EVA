//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

#define INF 1e20
#define EPS 1e-2
#define PI 3.14

#define MAX_OBJECTS 100
#define MAX_STEPS 200
#define MAX_DIST 200.0
#define SURF_DIST 1e-2
#define ESCAPE_SURF_DIST 2e-2

#define ENABLE_SHADOWS 0
#define ENABLE_GROUND_PLANE 0

layout(local_size_variable) in;
layout(binding = 0, rgba8) uniform writeonly image2D imgOutput;

uniform int objectBufferCount;
layout(std430, binding = 1) buffer sphereBuffer 
{ 
    vec4 spheresSSBO[]; 
};

uniform float time;
uniform sampler2D envMap;

uniform vec3 cameraPosition;
uniform vec3 cameraForward;
uniform vec3 cameraUp;
uniform vec3 cameraRight;
uniform float cameraFov;

shared vec4 sphereData[MAX_OBJECTS];

void bufferShared()
{
    uint id  = gl_LocalInvocationID.x;
    if(id < objectBufferCount) 
    {
        sphereData[id] = spheresSSBO[id]; 
    }
    memoryBarrierShared();
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
    const int RECURSE_NUM = 10;
    const float POWER = 8;
    const float SCALE = 3;

    vec4 p = vec4(point, 0) / SCALE;
	vec4 z = p;
	float dr = 1.0;
	float r = 0.0;
	for (int i = 0; i < RECURSE_NUM; i++) 
    {
		r = length(z);
		if (r > 10) break;

		float theta = acos(z.z / r);
		float phi = atan(z.y, z.x);
		dr = pow(r, POWER - 1.0) * POWER * dr + 1.0;

		float zr = pow(r, POWER);
		theta = theta * POWER;
		phi = phi * POWER;

		z = zr * vec4(
			sin(theta) * cos(phi),
			sin(phi) * sin(theta),
			cos(theta),
			1.0);
		z += p;
	}
	return SCALE * 0.5 * log(r) * r / dr;
}

float sdTetrahedron(vec3 z)
{
    const int Iterations = 15;
    const float Scale = 2;
    const float Offset = 2;

	float r;
    int n = 0;
    while (n < Iterations) 
    {
       if(z.x + z.y < 0) z.xy = -z.yx;
       if(z.x + z.z < 0) z.xz = -z.zx;
       if(z.y + z.z < 0) z.zy = -z.yz;
       z = z*Scale - Offset*(Scale-1.0);
       n++;
    }
    return length(z) * pow(Scale, -float(n));
}

float getDist(vec3 point)
{
    float minDist = INF;

    minDist = min(minDist, sdMandelbulb(point));

    float sphereDist = INF;
    for (int i = 0; i < objectBufferCount; i++)
    {
        float dist = length(point - sphereData[i].xyz) - sphereData[i].w;
        sphereDist = min(sphereDist, dist);
    }
    minDist = opSmoothSubtraction(sphereDist, minDist, 0.5);

    #if ENABLE_GROUND_PLANE
    minDist = min(minDist, point.y);
    #endif


    return minDist;
}

float rayMarch(vec3 ro, vec3 rd)
{
    float dist = 0;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        vec3 point = ro + rd * dist;

        float distToSurface = getDist(point);
        dist += distToSurface;

        if(distToSurface < SURF_DIST || dist > MAX_DIST) 
            break;
    }
    return dist;
}

const vec2 e = vec2(EPS, 0);
vec3 getNormal(vec3 point)
{
    float dist = getDist(point);
    vec3 normal = dist - vec3(getDist(point - e.xyy),
                              getDist(point - e.yxy),
                              getDist(point - e.yyx));
    return normalize(normal);
}

float getLight(vec3 point)
{
    vec3 lightPos = vec3(2, 5, -6);
    //lightPos.xz += vec2(sin(time), cos(time)) * 15;
    vec3 pointToLight = normalize(lightPos - point);
    vec3 normal = getNormal(point);

    float diffuse = max(0, dot(normal, pointToLight));

    // Shadow
    #if ENABLE_SHADOWS
    if(diffuse > 0) {
        float d = rayMarch(point + normal * ESCAPE_SURF_DIST, pointToLight);
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

    vec2 uv = vec2(pixelCoords.xy) / vec2(dims) - 0.5;
    uv.x *= aspect;

    // Base pixel colour
    vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);

    // Camera
    vec3 ro = cameraPosition;

    vec2 cpos = uv * tan(radians(cameraFov / 2)); 
    vec3 rd = vec3(cpos, 1);
    rd = normalize(rd);
    mat3 camToWorld = transpose(mat3(cameraRight, cameraUp, cameraForward));
    rd *= camToWorld;

    // Ray march
    float dist = rayMarch(ro, rd);

    if(dist <= MAX_DIST)
    {
        vec3 point = ro + rd * dist;

        float diffuse = getLight(point);
        pixel.rgb = vec3(diffuse);
    }
    else
    {
        vec2 uvEnv = sampleSphericalMap(rd);
        pixel.rgb = texture(envMap, uvEnv).rgb;
    }

    imageStore(imgOutput, pixelCoords, pixel);
}
