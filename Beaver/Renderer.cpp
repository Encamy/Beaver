#include "Renderer.h"

void GlClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

bool OpenGlError()
{
	while (GLenum error = glGetError())
	{
		std::stringstream message;
		message << error;
		__log__.LOG_ERROR(message.str(), __func__);
		return true;
	}
	return false;
}

void Renderer::Draw(VertexArray& va, IndexBuffer& ib, int ShaderID)
{
	va.Bind();
	ib.Bind();
	glUseProgram(ShaderID);

	glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::Draw(VertexArray & va, int ShaderID, int count)
{
	va.Bind();
	glUseProgram(ShaderID);

	glDrawArrays(GL_TRIANGLES, 0, count);
}

void Renderer::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
