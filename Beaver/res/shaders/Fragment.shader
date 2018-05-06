#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform vec3 u_Color;
uniform sampler2D u_Texture;
uniform bool use_tex;

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
}