#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include <GL/glew.h>
#include <vector>

class VertexBuffer 
{
public:
	VertexBuffer();
	VertexBuffer(const void *data, unsigned int size);
	~VertexBuffer();

	void SetData(const void *data, unsigned int size);
	void Bind() const;
	void UnBind() const;

private:
	GLuint m_rendererID;
};

#endif