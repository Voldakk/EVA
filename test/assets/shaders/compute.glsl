//#type compute
#version 430 compatibility
#extension GL_ARB_compute_variable_group_size : enable

#define INF 1e20
#define PI 3.14159265359

#define MAX_LIGHTS 10
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

uniform vec3 u_CameraPosition;

// IBL
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BrdfLUT;

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

uniform vec3 u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;

// Lights
uniform int u_NumLights;
uniform struct Light
{
   vec4 position;
   vec3 color;
   float attenuation;
} u_AllLights[MAX_LIGHTS];

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
    const int Iterations = u_MandelbulbIterations;
    const float Power = u_MandelbulbPower;
    const float Scale = u_MandelbulbScale;
    const float Bailout = u_MandelbulbScale;
    const vec3 pos = point;

    vec3 z = pos;
	float dr = 1.0;
	float r = 0.0;
	for (int i = 0; i < Iterations ; i++) {
		r = length(z);
		if (r>Bailout) break;
		
		// convert to polar coordinates
		float theta = acos(z.z/r);
		float phi = atan(z.y,z.x);
		dr =  pow( r, Power-1.0)*Power*dr + 1.0;
		
		// scale and rotate the point
		float zr = pow( r,Power);
		theta = theta*Power;
		phi = phi*Power;
		
		// convert back to cartesian coordinates
		z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
		z+=pos;
	}
	return 0.5*log(r)*r/dr;
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
       z = z * u_TetrahedronOffset - u_TetrahedronScale * (u_TetrahedronOffset - 1.0);
       n++;
    }
    return length(z) * pow(u_TetrahedronOffset, -float(n));
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

/*float getLight(vec3 point, vec3 normal)
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
}*/
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 Ambient(vec3 N, vec3 V, vec3 R, vec3 F0, float roughness, float metallic, float ao, vec3 albedo)
{
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(u_BrdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    return (kD * diffuse + specular) * ao;
}

vec3 CalcLight(vec3 point, vec3 N, vec3 V, vec3 F0, float roughness, float metallic, vec3 albedo)
{
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < u_NumLights; ++i) 
    {
        vec3 L;
        vec3 radiance;

        // Point light
        if(u_AllLights[i].position.w == 1.0)
        {
            L = normalize(u_AllLights[i].position.xyz - point);
            float distance = length(u_AllLights[i].position.xyz - point);
            float attenuation = u_AllLights[i].attenuation / (distance * distance);
            radiance = u_AllLights[i].color * attenuation;
        }
        else
        {
            L = normalize(u_AllLights[i].position.xyz);
            radiance = u_AllLights[i].color;
        }

        vec3 H = normalize(V + L);

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G = GeometrySmith(N, V, L, roughness);    
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
        vec3 nominator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	                
            
        float NdotL = max(dot(N, L), 0.0);        

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    return Lo;
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
    vec3 pixel = vec3(0.0);

    // Copy from frame buffer
    vec3 fbColor = texture(u_FbColor, uv).rgb;
    pixel = fbColor;

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

        //float diffuse = getLight(point, normal);
        float ao = getAO(point, normal);

        vec3 albedo = u_Albedo;
        float metallic = u_Metallic;
        float roughness = u_Roughness;
        vec3 N = normal;

        vec3 V = normalize(u_CameraPosition - point);
        vec3 R = reflect(-V, N); 

        vec3 F0 = mix(vec3(0.04), albedo, metallic);

        vec3 ambient = Ambient(N, V, R, F0, roughness, metallic, ao, albedo);
        vec3 Lo = CalcLight(point, N, V, F0, roughness, metallic, albedo);
        
        pixel = vec3(ambient + Lo);
    }
    else if(fbDepth >= u_CameraFar)
    {
        vec2 uvEnv = sampleSphericalMap(rd);
        pixel = texture(u_EnvMap, uvEnv).rgb; 

        // Vissibe sun
        /*for(int i = 0; i < u_NumLights; ++i) 
        {
            if (u_AllLights[i].position.w != 1.0)
            {
                float sunSize = 1 - 1 / pow(length(u_AllLights[i].color + 1), 0.3);
                sunSize *= 0.5;
                float deg = acos(dot(normalize(u_AllLights[i].position.xyz), rd));
                float a = 1 - clamp(deg, 0, sunSize) / (sunSize);
                pixel = mix(pixel, u_AllLights[i].color, a*a);
            }
        }*/
    }

    // HDR tonemapping
    pixel = pixel / (pixel + vec3(1.0));
    // gamma correct
    pixel = pow(pixel, vec3(1.0/2.2)); 

    imageStore(imgOutput, pixelCoords, vec4(pixel, 1.0));
}
