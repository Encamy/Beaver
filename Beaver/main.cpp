// GLEW
#define GLEW_STATIC
#include <GL\glew.h>
// GLFW
#include <GLFW\glfw3.h>
#include <iostream>
//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SOIL2/SOIL2.h"

#include "Logger.h"
#include "shader.hpp"
#include <windows.h>
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"

#define log __log__
#define LOG_TRACE(a) LOG_TRACE(a, __func__)
#define LOG_FATAL(a) LOG_FATAL(a, __func__);
#define LOG_ERROR(a) LOG_ERROR(a, __func__);
#define LOG_INFO(a) LOG_INFO(a, __func__);

const GLuint WIDTH = 600, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

bool keys[1024];

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}
}

void ScrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{

}

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{

}

int main() {
	log.LOG_TRACE("Initializing glfw");

	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "TEST", nullptr, nullptr);
	if (window == nullptr)
	{
		log.LOG_FATAL("Failed to create GLFW window");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		log.LOG_FATAL("Failed to create GLFW window");
		return -1;
	}

	log.LOG_TRACE("Initialization complete");
	log.LOG_INFO("Renderer: " + std::string((char*)glGetString(GL_RENDERER)));
	log.LOG_INFO("OpenGL version: " + std::string((char*)glGetString(GL_VERSION)));

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetScrollCallback(window, ScrollCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);

	GLfloat g_vertex_buffer_data[] = {
		-0.5f, -0.5f,
		0.5f, -0.5f,
		0.5f,  0.5f,
		-0.5f, 0.5f,
	};

	GLuint g_indecies[] = {
		0,1,2,
		2,3,0,
	};

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	VertexArray va;
	VertexBuffer vb(g_vertex_buffer_data, 4 * 2 * sizeof(GLfloat));

	VertexBufferLayout layout;

	layout.Push<float>(2);
	va.AddBuffer(vb, layout);
	
	IndexBuffer ib(g_indecies, 6);

	glClearColor(0, 0.5, 1, 1);
	GLuint programID = LoadShaders("res/shaders/SimpleVertex.shader", "res/shaders/SimpleFragment.shader");
	glUseProgram(programID);
	int location = glGetUniformLocation(programID, "u_Color");
	if (location == -1)
	{
		log.LOG_ERROR("Invalid location of iniform!");
	}

	va.UnBind();
	glUseProgram(0);
	vb.UnBind();
	ib.UnBind();

	Renderer renderer;

	float r = 0.0f;
	float increment = 0.05f;
	while (!glfwWindowShouldClose(window))
	{
		GlClearError();
		glfwPollEvents();

		renderer.Clear();

		glUseProgram(programID);
		glUniform3f(location, r, 0, 0);

		if (r > 1.0f)
			increment = -1 * increment;
		if (r < 0.0f)
			increment = -1 * increment;

		r += increment;

		renderer.Draw(va, ib, programID);

		OpenGlError();
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}