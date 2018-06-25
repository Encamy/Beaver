#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <GLFW\glfw3.h>
#include <string>
#include <sstream>
#include "Logger.h"
#include "VertexArray.h"
#include "IndexBuffer.h"

#define ASSERT(x) if(!(x)) __debugbreak();
#define GlCall(x) GlClearError(); x; ASSERT(OpenGlError);

void GlClearError();
bool OpenGlError();

class Renderer {
public:
	Renderer(int width, int heigth, GLFWwindow** window);
	~Renderer();
	void Draw(VertexArray& va, IndexBuffer& ib, int ShaderID);
	void Draw(VertexArray& va, int ShaderID, int count);
	void Clear();

private:
};

#endif