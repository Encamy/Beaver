#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec2 v_TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 u_MVP;
uniform mat4 model;

void main()
{
	gl_Position = u_MVP * position;
	v_TexCoord = texCoord;
	FragPos = vec3(model * position);

	Normal = mat3(transpose(inverse(model))) * normal;
}

