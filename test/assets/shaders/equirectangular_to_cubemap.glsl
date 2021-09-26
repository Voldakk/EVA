//#type vertex
#version 450 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

layout (location = 0) out vec3 localPos;

layout(binding = 0, std140) uniform Uniforms
{
	mat4 u_Projection;
	mat4 u_View;
};

void main()
{
    localPos = a_Position;  
    gl_Position =  u_Projection * u_View * vec4(a_Position, 1.0);
}


//#type fragment
#version 450 core
layout (location = 0) in vec3 localPos;
layout (location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_EquirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(localPos));
    vec3 color = texture(u_EquirectangularMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}
