#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class VertexArray
{
private:
	GLuint m_RendererID;
public:
	VertexArray();
	~VertexArray();

	void Bind();
	void UnBind();
	void AddBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout);
};

#endif