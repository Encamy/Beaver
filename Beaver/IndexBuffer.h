#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include <GL/glew.h>

class IndexBuffer 
{
public:
	IndexBuffer(const GLuint *data, GLuint count);
	~IndexBuffer();

	void Bind();
	void UnBind();

	GLuint GetCount();
private:
	GLuint m_rendererID;
	GLuint m_count;
};

#endif