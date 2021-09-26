//#type vertex
#version 450 core

layout (location = 0) in vec3 a_Position;

layout (location = 0) out vec3 fragPos;
layout (location = 1) out vec3 fragTexCoord;

layout(binding = 0, std140) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_InverseViewProjection;
	mat4 u_View;
	mat4 u_Projection;
    vec3 u_CameraPosition;
    float u_EnviromentRotation;
};

void main()
{
	// Pass some variables to the fragment shader
    fragTexCoord = a_Position;
    fragPos = a_Position;

    mat4 view = u_View;
    view[3][0] = 0;
    view[3][1] = 0;
    view[3][2] = 0;

    // Apply all matrix transformations to vert
    gl_Position = u_Projection * view * vec4(a_Position, 1);
}


//#type fragment
#version 450 core

#define MAX_LIGHTS 10

layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec3 fragTexCoord;

layout (location = 0) out vec4 fragColor;

layout(binding = 0, std140) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_InverseViewProjection;
	mat4 u_View;
	mat4 u_Projection;
    vec3 u_CameraPosition;
    float u_EnviromentRotation;
};

layout(binding = 0) uniform samplerCube u_EnvironmentMap;

// Lights
struct Light
{
   vec4 position;
   vec3 color;
   float attenuation;
};

layout(binding = 1, std140) uniform Lights
{
	int u_NumLights;
    Light u_Lights[MAX_LIGHTS];
};

mat3 RotateY(float a)
{
    return mat3(vec3(cos(a), 0, sin(a)), vec3(0, 1, 0), vec3(-sin(a), 0, cos(a)));
}
void main()
{
    vec3 v = RotateY(u_EnviromentRotation) * fragTexCoord;
	vec3 color = texture(u_EnvironmentMap, v).xyz; 
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    for(int i = 0; i < u_NumLights; ++i) 
    {
        if (u_Lights[i].position.w != 1.0)
        {
            float sunSize = 1 - 1 / pow(length(u_Lights[i].color + 1), 0.3);
            sunSize *= 0.5;
            float deg = acos(dot(normalize(u_Lights[i].position.xyz), normalize(fragPos.xyz)));
            float a = 1 - clamp(deg, 0, sunSize) / (sunSize);
            color = mix(color, u_Lights[i].color, a*a);
        }
    }

    fragColor = vec4(color, 1.0);
}
