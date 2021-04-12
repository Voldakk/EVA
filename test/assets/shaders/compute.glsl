//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

#define INF 1e20
#define EPS 1e-2

#define MAX_OBJECTS 1000
#define MAX_STEPS 100
#define MAX_DIST 100.0
#define SURF_DIST 0.01
#define ESCAPE_SURF_DIST 0.02

#define ENABLE_SHADOWS 1
#define ENABLE_GROUND_PLANE 0

layout(local_size_variable) in;
layout(rgba8) uniform image2D imgOutput;

uniform int objectBufferCount;
layout(std430, binding = 1) buffer sphereBuffer 
{ 
    vec4 spheresSSBO[]; 
};

uniform float time;

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

float getDist(vec3 point)
{
    float minDist = INF;
    for (int i = 0; i < objectBufferCount; i++)
    {
        float dist = length(point - sphereData[i].xyz) - sphereData[i].w;
        minDist = min(minDist, dist);
    }

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
    vec3 lightPos = vec3(0, 5, 6);
    lightPos.xz += vec2(sin(time), cos(time)) * 15;
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
    if(pixelCoords.x > dims.x || pixelCoords.y > dims.y) 
        return;

    vec2 uv = vec2(pixelCoords.xy) / vec2(dims) - 0.5;
    uv.x *= aspect;

    // Base pixel colour
    vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);

    // Camera
    vec3 ro = vec3(0, 2, -15);
    vec3 rd = normalize(vec3(uv.xy, 1));

    // Ray march
    float dist = rayMarch(ro, rd);
    vec3 point = ro + rd * dist;

    float diffuse = getLight(point);
    pixel.rgb = vec3(diffuse);

    imageStore(imgOutput, pixelCoords, pixel);
}
