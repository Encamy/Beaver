//C
#include <cstdlib>
#include <thread>
#include <iostream>
//GLEW
#define GLEW_STATIC
#include <GL\glew.h>
//GLFW
#include <GLFW\glfw3.h>
//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//Beaver
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
//STB
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//IMGUi
#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
//Tiny obj loader
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
//Logger definition
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

Camera camera(glm::vec3(0.0f, 2.0f, 5.0f));

std::vector<std::string> objs = { "landscape", "house" };

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void commandsListener();

int main() {
	bool enable_lighting = true;
	log.LOG_TRACE("Creating command listener");
	std::thread listenerThread(commandsListener);
	log.LOG_TRACE("Command listener created!");

	Renderer renderer(screen_width, screen_height, &window);

	log.LOG_INFO("GLSL compiler can optimize unused uniforms");
	log.LOG_INFO("Just ignore \"Invalid location of iniform\" error!");

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, MouseCallback);

	if (!debugMode)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	std::vector<std::vector<GLfloat>> vertices;
	std::vector<VertexArray*> vas;
	std::vector<VertexBuffer*> vbs;
	std::vector<tinyobj::attrib_t> attribs;
	std::vector<Texture*> textures;

	std::string err;

	GLuint vao;
	glGenVertexArrays(objs.size(), &vao);
	glBindVertexArray(vao);

	VertexBufferLayout objLayout;

	objLayout.Push<float>(3);		//3 for vertices
	objLayout.Push<float>(3);		//3 for normals
	objLayout.Push<float>(2);		//2 for texture coords

	tinyobj::attrib_t cubeAttrib;
	tinyobj::attrib_t sphereAttrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;	

	int i = 0;
	for (auto it : objs)
	{
		VertexBuffer *vb;
		vb = new VertexBuffer;
		VertexArray *va;
		va = new VertexArray;
		std::vector<GLfloat> tempVertices;
		log.LOG_TRACE("Loading texture \"res/images/" + it + ".png\"");
		Texture *texture = new Texture("res/images/" + it + ".png");
		textures.push_back(texture);

		std::string path = "res/obj/" + it + ".obj";
		log.LOG_TRACE("Loading object \"" + it + "\" from \"" + path + "\"");
		
		tinyobj::LoadObj(&cubeAttrib, &shapes, &materials, &err, path.c_str());

		if (!err.empty())
		{
			log.LOG_ERROR(err);
		}

		log.LOG_TRACE("Object \"" + it + "\" loaded from file");

		for (size_t s = 0; s < shapes.size(); s++)
		{
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
			{
				int fv = shapes[s].mesh.num_face_vertices[f];

				for (size_t v = 0; v < fv; v++)
				{
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					tempVertices.push_back(cubeAttrib.vertices[3 * idx.vertex_index + 0]);
					tempVertices.push_back(cubeAttrib.vertices[3 * idx.vertex_index + 1]);
					tempVertices.push_back(cubeAttrib.vertices[3 * idx.vertex_index + 2]);
					tempVertices.push_back(cubeAttrib.normals[3 * idx.normal_index + 0]);
					tempVertices.push_back(cubeAttrib.normals[3 * idx.normal_index + 1]);
					tempVertices.push_back(cubeAttrib.normals[3 * idx.normal_index + 2]);
					tempVertices.push_back(cubeAttrib.texcoords[2 * idx.texcoord_index + 0]);
					tempVertices.push_back(cubeAttrib.texcoords[2 * idx.texcoord_index + 1]);
				}
				index_offset += fv;
			}
		}

		vertices.push_back(tempVertices);
		vb->SetData(&vertices[vertices.size() - 1][0], vertices[vertices.size() - 1].size() * sizeof(GLfloat));
		vbs.push_back(vb);
		va->GenBuf();
		va->AddBuffer(*vb, objLayout);
		vas.push_back(va);
		i++;
		log.LOG_TRACE("VB and VA are created for object \"" + it + "\"");
	}
	
	glClearColor(0, 0.5, 1, 1);
	GLuint ProgramID = LoadShaders("res/shaders/Vertex.shader", "res/shaders/Fragment.shader");

	int u_color = glGetUniformLocation(ProgramID, "u_Color");
	int u_MVP = glGetUniformLocation(ProgramID, "u_MVP");
	int u_useTextures = glGetUniformLocation(ProgramID, "use_tex");
	int u_enableLighting = glGetUniformLocation(ProgramID, "u_enable_lighting");
	int u_lightPos = glGetUniformLocation(ProgramID, "lightPos");
	int u_model = glGetUniformLocation(ProgramID, "model");
	int u_viewPos = glGetUniformLocation(ProgramID, "viewPos");

	glUseProgram(0);

	int texture_uniform = glGetUniformLocation(ProgramID, "u_Texture");
	glUniform1i(texture_uniform, 0);

	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();

	glm::vec3 translation(1, 1, 0);
	glm::vec3 light_pos(-2.3f, 7.0f, 5.0f);
	GLfloat angle = 0;

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, MouseCallback);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		GlClearError();

		renderer.Clear();

		if (keys[GLFW_KEY_W] == true)
			camera.ProcessKeyboard(FORWARD, camera.MovementSpeed);
		if (keys[GLFW_KEY_S] == true)
			camera.ProcessKeyboard(BACKWARD, camera.MovementSpeed);
		if (keys[GLFW_KEY_A] == true)
			camera.ProcessKeyboard(LEFT, camera.MovementSpeed);
		if (keys[GLFW_KEY_D] == true)
			camera.ProcessKeyboard(RIGHT, camera.MovementSpeed);
		if (keys[GLFW_KEY_SPACE])
			camera.ProcessKeyboard(UP, camera.MovementSpeed);
		if (keys[GLFW_KEY_LEFT_SHIFT])
			camera.ProcessKeyboard(DOWN, camera.MovementSpeed);

		glUseProgram(ProgramID);

		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glm::vec3 viewPos = camera.GetPos();
		glUniform3f(u_viewPos, viewPos[0], viewPos[1], viewPos[2]);
		glUniform1i(u_enableLighting, enable_lighting);
		glUniform3f(u_lightPos, light_pos[0], light_pos[1], light_pos[2]);
		glUniform1i(u_useTextures, true);
		glUniform3f(u_color, 1.0, 1.0, 1.0);
		glm::mat4 model;
		glm::mat4 mvp;

		for (int i = 0; i < objs.size(); i++)
		{
			model = glm::translate(glm::mat4(1.0f), translation);
			glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
			model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
			model = model * rotate;
			mvp = proj * view * model;

			glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
			glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

			textures[i]->Bind();
			renderer.Draw(*vas[i], ProgramID, vertices[i].size() / 8);
		}

		//model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
		
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
	}
}