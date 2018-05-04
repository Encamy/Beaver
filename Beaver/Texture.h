#ifndef TEXTURE_H
#define TEXTURE_H

#include "Renderer.h"

class Texture {
private:
	GLuint m_RendererID;
	std::string m_FilePath;
	unsigned char *m_LocalBuffer;
	int m_Width, m_Height, m_BPP;
public:
	Texture(const std::string &path);
	~Texture();

	void Bind(GLuint slot = 0);
	void UnBind();

	int GetWidth();
	int GetHeight();
};

#endif