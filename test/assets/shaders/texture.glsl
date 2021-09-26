//#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_TexCoord;

layout(binding = 0, std140) uniform Camera
{
	mat4 u_ViewProjection;
};

layout(binding = 2, std140) uniform Object
{
    mat4 u_Model;
    vec2 u_Tiling;
};

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}

//#type fragment
#version 450 core

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D u_AlbedoMap;

layout(binding = 2, std140) uniform Object
{
    mat4 u_Model;
    vec2 u_Tiling;
};

void main()
{
	color = texture(u_AlbedoMap, v_TexCoord * u_Tiling);
}