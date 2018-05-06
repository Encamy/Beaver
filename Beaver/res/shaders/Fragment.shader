#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform vec3 u_Color;
uniform sampler2D u_Texture;
uniform bool use_tex;
uniform bool u_enable_lighting;

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
		float ambientStrength = 0.1;
		vec3 ambient = ambientStrength * vec3(1.0f, 1.0f, 1.0f);

		vec4 result = vec4(ambient, 1.0f) * color;
		color = result;
	}
}