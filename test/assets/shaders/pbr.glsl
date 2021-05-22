//#type vertex
#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;  

out VTOF {
    vec3 fragPos;
    vec2 UV;
    mat3 TBN;
    vec3 tangentFragPos;
    vec3 tangentViewPos;
} vOut;

uniform vec3 u_CameraPosition;
uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
    vOut.fragPos = vec3(u_Model * vec4(a_Position, 1.0));

    vOut.UV = a_TexCoords;


    // Calculate TBN matrix
	//vec3 T = normalize(mat3(u_Model) * a_Tangent);
	//vec3 B = normalize(mat3(u_Model) * a_Bitangent);
	//vec3 N = normalize(mat3(u_Model) * a_Normal);

    vec3 T = normalize(mat3(u_Model) * a_Tangent);
	vec3 N = normalize(mat3(u_Model) * a_Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vOut.TBN = mat3(T, B, N);

    vOut.tangentViewPos  = inverse(vOut.TBN) * u_CameraPosition;
    vOut.tangentFragPos  = inverse(vOut.TBN) * vOut.fragPos;

    gl_Position =  u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}


//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------

//#type fragment
#version 330 core

#define MAX_LIGHTS 10
const float PI = 3.14159265359;

in VTOF {
    vec3 fragPos;
    vec2 UV;
    mat3 TBN;
    vec3 tangentFragPos;
    vec3 tangentViewPos;
} fIn;

out vec4 fragColor;

uniform vec3 u_CameraPosition;

// Material
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AmbientOcclusionMap;
uniform sampler2D u_EmissiveMap;
uniform sampler2D u_HeightMap;

uniform float u_HeightScale;

// IBL
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BrdfLUT;

// Lights
uniform int u_NumLights;
uniform struct Light
{
   vec4 position;
   vec3 color;
   float attenuation;
} u_AllLights[MAX_LIGHTS];

// ----------------------------------------------------------------------------
float SampleDepth(vec2 uv)
{
    return 1.0 - texture(u_HeightMap, uv).r;
}
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * u_HeightScale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = SampleDepth(currentTexCoords);
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = SampleDepth(currentTexCoords);  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = SampleDepth(prevTexCoords) - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
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

    //return specular;
    return (kD * diffuse + specular) * ao;
}
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
void main()
{	
    vec3 viewDir = normalize(fIn.tangentViewPos - fIn.tangentFragPos);
    vec2 UV = fIn.UV;
    UV = ParallaxMapping(UV,  viewDir);       
    if(UV.x > 1.0 || UV.y > 1.0 || UV.x < 0.0 || UV.y < 0.0)
        discard;


    vec3 albedo = pow(texture(u_AlbedoMap, UV).rgb, vec3(2.2));
    float metallic = texture(u_MetallicMap, UV).r;
    float roughness = texture(u_RoughnessMap, UV).r;
    float ao = texture(u_AmbientOcclusionMap, UV).r;
    vec3 emissive = texture(u_EmissiveMap, UV).rgb;
    float height = texture(u_HeightMap, UV).r;

    vec3 N = texture(u_NormalMap, UV).rgb;
    N = normalize(N * 2.0 - 1.0);
	N = normalize(fIn.TBN * N);

    vec3 V = normalize(u_CameraPosition - fIn.fragPos);
    vec3 R = reflect(-V, N); 

    // Calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 of 0.04
    // and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = mix(vec3(0.04), albedo, metallic);


    vec3 ambient = Ambient(N, V, R, F0, roughness, metallic, ao, albedo);
    vec3 Lo = CalcLight(fIn.fragPos, N, V, F0, roughness, metallic, albedo);
    
    vec3 color = ambient + Lo + emissive;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    //fragColor = vec4((N + 1) / 2, 1.0);
    //fragColor = vec4((fIn.TBN[1] + 1) / 2, 1.0);
    //fragColor = vec4(albedo, 1.0);
    fragColor = vec4(color, 1.0);
}
