#include "Renderer.h"

#define log __log__
#define LOG_TRACE(a) log.LOG_TRACE(a, __func__)
#define LOG_FATAL(a) log.LOG_FATAL(a, __func__);
#define LOG_ERROR(a) log.LOG_ERROR(a, __func__);
#define LOG_INFO(a) log.LOG_INFO(a, __func__);

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
		LOG_ERROR(message.str());
		return true;
	}
	return false;
}

Renderer::Renderer(int width, int heigth, GLFWwindow **window)
{
	LOG_TRACE("Initializing glfw");

	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	*window = glfwCreateWindow(width, heigth, "Beaver", nullptr, nullptr);
	if (*window == nullptr)
	{
		LOG_FATAL("Failed to create GLFW window");
		glfwTerminate();
		throw;
	}
	glfwMakeContextCurrent(*window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		LOG_FATAL("Failed to create GLFW window");
		throw;
	}

	LOG_TRACE("Initialization complete");
	LOG_INFO("Renderer: " + std::string((char*)glGetString(GL_RENDERER)));
	LOG_INFO("OpenGL version: " + std::string((char*)glGetString(GL_VERSION)));

	glfwGetFramebufferSize(*window, &width, &heigth);

	glViewport(0, 0, width, heigth);

	glfwSwapInterval(1);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

Renderer::~Renderer()
{
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
