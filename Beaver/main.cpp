#include <cstdlib>
#include <thread>
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

#include "Logger.h"
#include "shader.hpp"
#include <windows.h>
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Texture.h"
#include "Camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>

#define log __log__
#define LOG_TRACE(a) LOG_TRACE(a, __func__)
#define LOG_FATAL(a) LOG_FATAL(a, __func__);
#define LOG_ERROR(a) LOG_ERROR(a, __func__);
#define LOG_INFO(a) LOG_INFO(a, __func__);

GLFWwindow* window;
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

bool keys[1024];
bool debugMode = false;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

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

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	float xoffset = xPos - lastX;
	float yoffset = lastY - yPos; // reversed since y-coordinates go from bottom to top

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void commandsListener()
{
	std::string cmd;
	while (true)
	{
		std::getline(std::cin, cmd);
		log.LOG_INFO(cmd);

		if (cmd == "debug")
		{
			debugMode = !debugMode;
			if (debugMode)
			{

			}
			else
			{
				glfwSetKeyCallback(window, key_callback);
				glfwSetCursorPosCallback(window, MouseCallback);
			}
		}
	}
}


int main() {
	log.LOG_TRACE("Creating command listener");
	std::thread listenerThread(commandsListener);
	log.LOG_TRACE("Commad listener created!");

	log.LOG_INFO("GLSL compiler can optimize unused uniforms");
	log.LOG_INFO("Just ignore \"Invalid location of iniform\" error!");
	log.LOG_TRACE("Initializing glfw");

	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "TEST", nullptr, nullptr);
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

	if (!debugMode)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	glEnable(GL_DEPTH_TEST);

	GLfloat g_vertex_buffer_data[] = {
		-0.5f, -0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, 1.0f, 0.0,
		0.5f,  0.5f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.0f, 1.0f
	};

	GLuint g_indecies[] = {
		0,1,2,
		2,3,0,
	};

	GLfloat cube_vertices[] = {
		// front
		-1.0, -1.0,  1.0, 0.0, 0.0,
		1.0, -1.0,  1.0, 1.0, 0.0, 
		1.0,  1.0,  1.0, 1.0, 1.0,
		-1.0,  1.0,  1.0, 0.0, 1.0,
		// back
		-1.0, -1.0, -1.0, 1.0, 0.0,
		1.0, -1.0, -1.0, 0.0, 0.0,
		1.0,  1.0, -1.0, 0.0, 1.0,
		-1.0,  1.0, -1.0, 1.0, 1.0,
	};

	GLuint cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3,
	};

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	VertexArray va;
	VertexBuffer vb(cube_vertices, sizeof(cube_vertices));

	VertexBufferLayout layout;

	layout.Push<float>(3);
	layout.Push<float>(2);
	va.AddBuffer(vb, layout);
	
	IndexBuffer ib(cube_elements, 36);

	glClearColor(0, 0.5, 1, 1);
	GLuint programID = LoadShaders("res/shaders/SimpleVertex.shader", "res/shaders/SimpleFragment.shader");
	glUseProgram(programID);
	int location = glGetUniformLocation(programID, "u_Color");
	if (location == -1)
	{
		log.LOG_ERROR("Invalid location of iniform!");
	}

	int MVP_position = glGetUniformLocation(programID, "u_MVP");
	if (MVP_position == -1)
	{
		log.LOG_ERROR("Invalid location of iniform!");
	}

	va.UnBind();
	glUseProgram(0);
	vb.UnBind();
	ib.UnBind();

	Texture texture("res/images/sample.png");
	texture.Bind();
	int texture_uniform = glGetUniformLocation(programID, "u_Texture");
	glUniform1i(texture_uniform, 0);

	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();

	glm::vec3 translation(1, 1, 0);
	GLfloat angle = 0;

	Renderer renderer;

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, MouseCallback);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		GlClearError();

		renderer.Clear();

		if (keys[GLFW_KEY_W] == true)
		{
			camera.ProcessKeyboard(FORWARD, 0.13f);
		}
		if (keys[GLFW_KEY_S] == true)
		{
			camera.ProcessKeyboard(BACKWARD, 0.13f);
		}
		if (keys[GLFW_KEY_A] == true)
		{
			camera.ProcessKeyboard(LEFT, 0.13f);
		}
		if (keys[GLFW_KEY_D] == true)
		{
			camera.ProcessKeyboard(RIGHT, 0.13f);
		}

		if (debugMode)
		{
			ImGui_ImplGlfwGL3_NewFrame();

			ImGui::Begin("Transformation", 0);
			ImGui::SliderFloat3("Translation", &translation.x, -5.0f, 5.0f);
			ImGui::SliderFloat("Rotation", &angle, -360, 360);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		glUseProgram(programID);

		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		for (int i = 0; i < 10; i++)
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);
			model = model * glm::translate(glm::mat4(1.0f), cubePositions[i]);
			model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
			glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
			rotate = rotate * glm::rotate(glm::mat4(1.0f), glm::radians(20.0f * i), glm::vec3(1.0f, 1.0f, 1.0f));
			glm::mat4 mvp = proj * view * model * rotate;

			glUniformMatrix4fv(MVP_position, 1, GL_FALSE, &mvp[0][0]);

			renderer.Draw(va, ib, programID);
		}

		OpenGlError();

		if (debugMode)
		{
			ImGui::Render();
			ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);

	}
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	exit(0);

	return 0;
}