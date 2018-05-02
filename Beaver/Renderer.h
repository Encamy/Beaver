#ifndef RENDERER_H
#define RENDERER_H

#include "Logger.h"
#include <GL/glew.h>
#include <string>
#include <sstream>
#include "VertexArray.h"
#include "IndexBuffer.h"

#define ASSERT(x) if(!(x)) __debugbreak();
#define GlCall(x) GlClearError(); x; ASSERT(OpenGlError);

void GlClearError();
bool OpenGlError();

class Renderer {
public:
	void Draw(VertexArray& va, IndexBuffer& ib, int ShaderID);
	void Clear();

private:
};

#endif