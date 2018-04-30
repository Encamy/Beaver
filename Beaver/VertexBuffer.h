#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "Renderer.h"

class VertexBuffer 
{
public:
	VertexBuffer(const void *data, unsigned int size);
	~VertexBuffer();

	void Bind();
	void UnBind();

private:
	GLuint m_rendererID;
};

#endif