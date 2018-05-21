#include "Texture.h"

#include "stb_image.h"
#include <string>
#include <cmath>

Texture::Texture(const std::string & path)
	: m_FilePath(path), m_LocalBuffer(nullptr), 
	m_Width(0), m_Height(0), m_BPP(0)
{
	stbi_set_flip_vertically_on_load(1);
	//m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
	unsigned char *LocalBuffer[9];

	glGenTextures(1, &m_RendererID);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	for (int i = 0; i < 9; i++)
	{
		std::string file = std::string("res/images/") + std::to_string((int)pow(2, i)) + std::string(".bmp");
		int height, width;
		__log__.LOG_TRACE("Loading texture " + file, __func__);
		LocalBuffer[i] = stbi_load(file.c_str(), &width, &height, &m_BPP, 3);
		glTexImage2D(GL_TEXTURE_2D, 8-i, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, LocalBuffer[i]);
		if (LocalBuffer[i])
		{
			stbi_image_free(LocalBuffer[i]);
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	if (m_LocalBuffer)
	{
		stbi_image_free(m_LocalBuffer);
	}
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_RendererID);
}

void Texture::Bind(GLuint slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

void Texture::UnBind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

int Texture::GetWidth()
{
	return m_Width;
}

int Texture::GetHeight()
{
	return m_Height;
}
