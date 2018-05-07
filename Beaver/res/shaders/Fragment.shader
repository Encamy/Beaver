#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 u_Color;
uniform sampler2D u_Texture;
uniform bool use_tex;
uniform bool u_enable_lighting;
uniform vec3 lightPos;
uniform vec3 viewPos;

void textureFrag()
{
	vec4 texColor = texture(u_Texture, v_TexCoord);
	color = texColor;
}

void colorFrag()
{
	color = vec4(u_Color, 1);
}

void main()
{
	if (use_tex)
	{
		textureFrag();
	}
	else
	{
		colorFrag();
	}

	if (u_enable_lighting)
	{
		vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
		//ambient
		float ambientStrength = 0.1;
		vec3 ambient = ambientStrength * lightColor;

		//diffuse
		vec3 norm = normalize(Normal);
		vec3 lightDir = normalize(lightPos - FragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * lightColor;

		//specular
		float specularStrength = 0.5;
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = specularStrength*spec*lightColor;

		vec4 result = vec4((ambient + diffuse + specular), 1.0f) * color;
		color = result;
	}
}