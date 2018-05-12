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

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define log __log__
#define LOG_TRACE(a) LOG_TRACE(a, __func__)
#define LOG_FATAL(a) LOG_FATAL(a, __func__);
#define LOG_ERROR(a) LOG_ERROR(a, __func__);
#define LOG_INFO(a) LOG_INFO(a, __func__);

GLFWwindow* window;
int screen_width = 800, screen_height = 600;

float lastX = screen_width / 2.0f;
float lastY = screen_height / 2.0f;
bool firstMouse = true;

bool keys[1024];
bool debugMode = false;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		glfwSetCursorPosCallback(window, NULL);
		debugMode = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
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
	bool enable_lighting = true;
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

	window = glfwCreateWindow(screen_width, screen_height, "TEST", nullptr, nullptr);
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

	glfwGetFramebufferSize(window, &screen_width, &screen_height);

	glViewport(0, 0, screen_width, screen_height);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, MouseCallback);

	if (!debugMode)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	glfwSwapInterval(1);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	
	std::string inputfile = "res/obj/cube.obj";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> tex_coords;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());

	if (!err.empty()) {
		log.LOG_ERROR(err);
	}

	for (size_t s = 0; s < shapes.size(); s++) {
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];

			for (size_t v = 0; v < fv; v++) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
				vertices.push_back(attrib.normals[3 * idx.normal_index + 0]);
				vertices.push_back(attrib.normals[3 * idx.normal_index + 1]);
				vertices.push_back(attrib.normals[3 * idx.normal_index + 2]);
				vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
				vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
			}
			index_offset += fv;
		}
	}

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	VertexArray va;
	VertexBuffer vb(&vertices[0], vertices.size()*sizeof(GLfloat));

	VertexBufferLayout layout;

	layout.Push<float>(3);
	layout.Push<float>(3);
	layout.Push<float>(2);
	va.AddBuffer(vb, layout);
	
	glClearColor(0, 0.5, 1, 1);
	GLuint ProgramID = LoadShaders("res/shaders/Vertex.shader", "res/shaders/Fragment.shader");

	int color_position = glGetUniformLocation(ProgramID, "u_Color");
	int MVP_position = glGetUniformLocation(ProgramID, "u_MVP");
	int use_tex_position = glGetUniformLocation(ProgramID, "use_tex");
	int u_lighting_position = glGetUniformLocation(ProgramID, "u_enable_lighting");
	int u_lightPos = glGetUniformLocation(ProgramID, "lightPos");
	int u_model = glGetUniformLocation(ProgramID, "model");
	int u_viewPos = glGetUniformLocation(ProgramID, "viewPos");

	va.UnBind();
	glUseProgram(0);
	vb.UnBind();

	//Texture texture("res/images/brick3K/Tiles05_COL_VAR1_3K.jpg");
	Texture texture("res/images/brick1K/Tiles05_COL_VAR1_1K.jpg");
	//Texture texture("res/images/sample.png");
	texture.Bind();
	int texture_uniform = glGetUniformLocation(ProgramID, "u_Texture");
	glUniform1i(texture_uniform, 0);

	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();

	glm::vec3 translation(1, 1, 0);
	glm::vec3 light_pos(2.0f, 2.0f, 2.0f);
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
			camera.ProcessKeyboard(FORWARD, 0.13f);
		if (keys[GLFW_KEY_S] == true)
			camera.ProcessKeyboard(BACKWARD, 0.13f);
		if (keys[GLFW_KEY_A] == true)
			camera.ProcessKeyboard(LEFT, 0.13f);
		if (keys[GLFW_KEY_D] == true)
			camera.ProcessKeyboard(RIGHT, 0.13f);
		if (keys[GLFW_KEY_SPACE])
			camera.ProcessKeyboard(UP, 0.13f);
		if (keys[GLFW_KEY_LEFT_SHIFT])
			camera.ProcessKeyboard(DOWN, 0.13f);
		if (keys[GLFW_KEY_KP_ADD])
			angle += 1.0f;
		if (keys[GLFW_KEY_KP_SUBTRACT])
			angle -= 1.0f;
		if (keys[GLFW_KEY_KP_6])
			light_pos[0] += 0.1f;
		if (keys[GLFW_KEY_KP_4])
			light_pos[0] -= 0.1f;
		if (keys[GLFW_KEY_KP_8])
			light_pos[1] += 0.1f;
		if (keys[GLFW_KEY_KP_2])
			light_pos[1] -= 0.1f;
		if (keys[GLFW_KEY_KP_3])
			light_pos[2] += 0.1f;
		if (keys[GLFW_KEY_KP_9])
			light_pos[2] -= 0.1f;

		glUseProgram(ProgramID);

		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glm::vec3 viewPos = camera.GetPos();
		glUniform3f(u_viewPos, viewPos[0], viewPos[1], viewPos[2]);
		glUniform1i(u_lighting_position, enable_lighting);
		glUniform3f(u_lightPos, light_pos[0], light_pos[1], light_pos[2]);

		for (int i = 0; i < 11; i++)
		{
			if (i >= 5 == 0)
			{
				glUniform1i(use_tex_position, true);
			}
			else
			{
				glUniform1i(use_tex_position, false);
			}
			glm::mat4 model;
			glm::mat4 mvp;
			if (i == 10)
			{
				glUniform3f(color_position, 1.0f, 1.0f, 1.0f);
				model = glm::translate(glm::mat4(1.0f), light_pos);
				model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
				mvp = proj * view * model;
			}
			else
			{
				glUniform3f(color_position, 0.2f, 0.6f, 0.2f);
				model = glm::translate(glm::mat4(1.0f), translation);
				model = model * glm::translate(glm::mat4(1.0f), cubePositions[i]);
				model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
				glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				rotate = rotate * glm::rotate(glm::mat4(1.0f), glm::radians(20.0f * i), glm::vec3(1.0f, 1.0f, 1.0f));
				model = model * rotate;

				mvp = proj * view * model;
			}
			
			glUniformMatrix4fv(MVP_position, 1, GL_FALSE, &mvp[0][0]);
			glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

			renderer.Draw(va, ProgramID, vertices.size() / 8); //count of triangles! vertices.size() = 3v + 3vn + 2vt
		}

		OpenGlError();

		if (debugMode)
		{
			ImGui_ImplGlfwGL3_NewFrame();

			ImGui::Begin("Debug", 0);
			ImGui::SliderFloat3("Translation", &translation.x, -5.0f, 5.0f);
			ImGui::SliderFloat3("Light Pos", &light_pos[0], -5.0f, 5.0f);
			ImGui::SliderFloat("Rotation", &angle, -360, 360);
			if (ImGui::Button("Hide"))
			{
				debugMode = false;
				glfwSetCursorPosCallback(window, MouseCallback);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				firstMouse = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Lighting")) 
			{
				enable_lighting = !enable_lighting;
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();

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