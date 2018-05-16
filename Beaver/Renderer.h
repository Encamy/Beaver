#ifndef RENDERER_H
#define RENDERER_H

#include "Logger.h"
#include <GL/glew.h>
#include <string>
#include <sstream>
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Logger.h"
#include <GLFW\glfw3.h>

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