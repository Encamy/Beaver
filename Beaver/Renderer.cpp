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
