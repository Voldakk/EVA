//#type vertex
#version 330 core

layout (location = 0) in vec3 a_Position;

out vec3 fragTexCoord;
out vec3 fragPos;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_Model;

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
    gl_Position = u_Projection * view * u_Model * vec4(a_Position, 1);
}


//#type fragment
#version 330 core

uniform samplerCube u_EnvironmentMap;

in vec3 fragTexCoord;

out vec4 fragColor;

void main()
{
	vec3 color = texture(u_EnvironmentMap, fragTexCoord).xyz; 
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    fragColor = vec4(color, 1.0);
}
