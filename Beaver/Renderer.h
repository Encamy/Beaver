#ifndef RENDERER_H
#define RENDERER_H

#include "Logger.h"
#include <GL/glew.h>
#include <string>
#include <sstream>

#define ASSERT(x) if(!(x)) __debugbreak();
#define GlCall(x) GlClearError(); x; ASSERT(OpenGlError);

void GlClearError();
bool OpenGlError();

#endif