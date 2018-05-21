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
int mainScreenWidth = 600, mainScreenHeight = 600;
int subScreenWidth = 300, subScreenHeight = 300;

float lastX = mainScreenWidth / 2.0f;
float lastY = mainScreenHeight / 2.0f;
bool firstMouse = true;

bool keys[1024];
bool debugMode = false;
bool useSpotLight = false;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

enum object
{
	cube,
	sphere,
};

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
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

void mouseCallback(GLFWwindow *window, double xPos, double yPos)
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

int main() 
{
	object obj = cube;
	bool enableLighting = true;
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

	window = glfwCreateWindow(mainScreenWidth + subScreenWidth, mainScreenHeight, "TEST", nullptr, nullptr);
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

	glViewport(0, 0, mainScreenWidth, mainScreenHeight);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);

	if (!debugMode)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	glfwSwapInterval(1);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	//glEnable(GL_CULL_FACE);
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
	
	tinyobj::attrib_t cubeAttrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	tinyobj::attrib_t sphereAttrib;

	std::vector<GLfloat> sphereVertices;
	std::vector<GLfloat> cubeVertices;

	GLfloat guiVertices[] = {
		mainScreenWidth,					mainScreenHeight, 0,
		mainScreenWidth,					0,				  0,
		mainScreenWidth + subScreenWidth,	subScreenHeight,  0,
		mainScreenWidth,					subScreenHeight,  0,
	};

	GLuint guiElements[] = {
		0, 1,
		2, 3,
	};

	GLfloat coordinateVertices[] = {
		0.0f,	0.0f,		0.0f,	0, 0, 0,
		1.0f,	0.0f,		0.0f,	1, 0, 0,
		0.0f,	1.0f,		0.0f,	0, 1, 0,
		0.0f,	0.0f,		1.0f,	0, 0, 0,

		1.0f,   -0.1f,		0.0f,	0, 0, 0,		//X
		1.4f,   -0.5f,		0.0f,	0, 0, 0,
		1.4f,   -0.1f,		0.0f,	0, 0, 0,
		1.0f,   -0.5f,		0.0f,	0, 0, 0,

		0.0f,	1.1f,		0.0f,	0, 0, 0,		//Y
		0.0f,	1.3f,		0.0f,	0, 0, 0,
		0.2f,	1.5f,		0.0f,	0, 0, 0,
	   -0.2f,	1.5f,		0.0f,	0, 0, 0,

	   0.0f,	-0.1f,		1.5f,	0, 0, 0,		//Z
	   0.0f,	-0.1f,		1.1f,	0, 0, 0,
	   0.0f,	-0.5f,		1.5f,	0, 0, 0,
	   0.0f,	-0.5f,		1.1f,	0, 0, 0,
	};

	GLuint coordinateElements[] = {
		0, 1,
		0, 2,
		0, 3,
		4, 5,
		6, 7,
		8, 9,
		9, 10,
		9, 11,
		12, 13,
		13, 14,
		14, 15
	};
	
	std::string err;
	tinyobj::LoadObj(&sphereAttrib, &shapes, &materials, &err, "res/obj/sphere.obj");

	if (!err.empty()) 
	{
		log.LOG_ERROR(err);
	}

	for (size_t s = 0; s < shapes.size(); s++) 
	{
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
		{
			int fv = shapes[s].mesh.num_face_vertices[f];

			for (size_t v = 0; v < fv; v++) 
			{
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				sphereVertices.push_back(sphereAttrib.vertices[3 * idx.vertex_index + 0]);
				sphereVertices.push_back(sphereAttrib.vertices[3 * idx.vertex_index + 1]);
				sphereVertices.push_back(sphereAttrib.vertices[3 * idx.vertex_index + 2]);
				sphereVertices.push_back(sphereAttrib.normals[3 * idx.normal_index + 0]);
				sphereVertices.push_back(sphereAttrib.normals[3 * idx.normal_index + 1]);
				sphereVertices.push_back(sphereAttrib.normals[3 * idx.normal_index + 2]);
				sphereVertices.push_back(sphereAttrib.texcoords[2 * idx.texcoord_index + 0]);
				sphereVertices.push_back(sphereAttrib.texcoords[2 * idx.texcoord_index + 1]);
			}
			index_offset += fv;
		}
	}

	tinyobj::LoadObj(&cubeAttrib, &shapes, &materials, &err, "res/obj/cube.obj");

	if (!err.empty()) 
	{
		log.LOG_ERROR(err);
	}

	for (size_t s = 0; s < shapes.size(); s++)
	{
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
		{
			int fv = shapes[s].mesh.num_face_vertices[f];

			for (size_t v = 0; v < fv; v++) 
			{
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				cubeVertices.push_back(cubeAttrib.vertices[3 * idx.vertex_index + 0]);
				cubeVertices.push_back(cubeAttrib.vertices[3 * idx.vertex_index + 1]);
				cubeVertices.push_back(cubeAttrib.vertices[3 * idx.vertex_index + 2]);
				cubeVertices.push_back(cubeAttrib.normals[3 * idx.normal_index + 0]);
				cubeVertices.push_back(cubeAttrib.normals[3 * idx.normal_index + 1]);
				cubeVertices.push_back(cubeAttrib.normals[3 * idx.normal_index + 2]);
				cubeVertices.push_back(cubeAttrib.texcoords[2 * idx.texcoord_index + 0]);
				cubeVertices.push_back(cubeAttrib.texcoords[2 * idx.texcoord_index + 1]);
			}
			index_offset += fv;
		}
	}

	GLuint vao;
	glGenVertexArrays(4, &vao);
	glBindVertexArray(vao);

	//cube
	VertexArray vaCube;
	VertexBuffer vbCube(&cubeVertices[0], cubeVertices.size()*sizeof(GLfloat));

	VertexBufferLayout objLayout;

	objLayout.Push<float>(3);
	objLayout.Push<float>(3);
	objLayout.Push<float>(2);
	vaCube.AddBuffer(vbCube, objLayout);

	//coordinate system
	VertexArray vaCoordinates;
	VertexBuffer vbCoordinates(coordinateVertices, sizeof(coordinateVertices));

	VertexBufferLayout linesLayout;

	linesLayout.Push<float>(3);
	linesLayout.Push<float>(3);
	vaCoordinates.AddBuffer(vbCoordinates, linesLayout);

	IndexBuffer ib(coordinateElements, 22);

	//gui lines
	VertexArray vaGui;
	VertexBuffer vbGui(guiVertices, sizeof(guiVertices));

	VertexBufferLayout guiLayout;
	guiLayout.Push<float>(3);

	vaGui.AddBuffer(vbGui, guiLayout);

	IndexBuffer ibGui(guiElements, 4);
	
	//sphere
	VertexArray vaSphere;
	VertexBuffer vbSphere(&sphereVertices[0], sphereVertices.size() * sizeof(GLfloat));

	vaSphere.AddBuffer(vbSphere, objLayout);

	glClearColor(0, 0.5, 1, 1);
	GLuint ProgramID = LoadShaders("res/shaders/Vertex.shader", "res/shaders/Fragment.shader");

	int u_color = glGetUniformLocation(ProgramID, "u_Color");
	int u_MVP = glGetUniformLocation(ProgramID, "u_MVP");
	int u_useTextures = glGetUniformLocation(ProgramID, "use_tex");
	int u_enableLighting = glGetUniformLocation(ProgramID, "u_enable_lighting");
	int u_lightPos = glGetUniformLocation(ProgramID, "lightPos");
	int u_model = glGetUniformLocation(ProgramID, "model");
	int u_viewPos = glGetUniformLocation(ProgramID, "viewPos");
	int u_lightDirection = glGetUniformLocation(ProgramID, "lightDirection");
	int u_lightCutOff = glGetUniformLocation(ProgramID, "lightCutOff");
	int u_lightOuterCutOff = glGetUniformLocation(ProgramID, "lightOuterCutOff");
	int u_useSpotLight = glGetUniformLocation(ProgramID, "useSpotLight");

	vaCube.UnBind();
	glUseProgram(0);
	vbCube.UnBind();

	//Texture texture("res/images/brick3K/Tiles05_COL_VAR1_3K.jpg");
	Texture texture("res/images/brick1K/Tiles05_COL_VAR1_1K.jpg");
	//Texture texture("res/images/sample.png");
	texture.Bind();
	int u_texture = glGetUniformLocation(ProgramID, "u_Texture");
	glUniform1i(u_texture, 0);

	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();

	glm::vec3 translation(0, 0, 0);
	glm::vec3 lightPosition(1.0f, 0.0f, 2.0f);
	glm::vec3 lightDirection(0.0f, 0.0f, 0.0f);
	float cutOffAngle = 12.5f;
	float outerCutOffAngle = 17.5f;

	glm::vec3 viewPosTop(0.0f, 5.0f, 0.0f);

	GLfloat angle = 0;

	Renderer renderer;

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);

	while (!glfwWindowShouldClose(window))
	{
		glViewport(0, 0, mainScreenWidth, mainScreenHeight);
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


		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)mainScreenWidth / (float)mainScreenHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glm::vec3 viewPos = camera.GetPos();
		glUniform3f(u_viewPos, viewPos[0], viewPos[1], viewPos[2]);
		glUniform1i(u_enableLighting, enableLighting);
		glUniform3f(u_lightPos, lightPosition[0], lightPosition[1], lightPosition[2]);
		glUniform1i(u_useTextures, true);

		glm::mat4 model;
		glm::mat4 mvp;

		//cube
		glUniform3f(u_color, 0.2f, 0.6f, 0.2f);
		model = glm::translate(glm::mat4(1.0f), translation);
		model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = model * rotate;

		mvp = proj * view * model;

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

		switch (obj)
		{
		case cube: renderer.Draw(vaCube, ProgramID, cubeVertices.size() / 8);
			break;
		case sphere: renderer.Draw(vaSphere, ProgramID, sphereVertices.size() / 8);
			break;
		}

		//light
		glUniform3f(u_color, 1.0f, 1.0f, 1.0f);
		model = glm::translate(glm::mat4(1.0f), lightPosition);
		model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
		mvp = proj * view * model;

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

		renderer.Draw(vaSphere, ProgramID, sphereVertices.size() / 8);

		OpenGlError();
		ImGui_ImplGlfwGL3_NewFrame();
		if (debugMode)
		{
			ImGui::SetNextWindowPos(ImVec2(10, 50));
			ImGui::SetNextWindowSize(ImVec2(450, 220));
			ImGui::Begin("Object movement", 0, ImGuiWindowFlags_NoMove);
			ImGui::SliderFloat3("Translation", &translation.x, -5.0f, 5.0f);
			ImGui::InputFloat("Light Position X", &lightPosition[0], 0.01f, 1.0f);
			ImGui::InputFloat("Light Position Y", &lightPosition[1], 0.01f, 1.0f);
			ImGui::InputFloat("Light Position Z", &lightPosition[2], 0.01f, 1.0f);
			ImGui::SliderFloat("Rotation", &angle, -360, 360);
			if (ImGui::Button("Cube"))
			{
				obj = cube;
			}
			ImGui::SameLine();
			if (ImGui::Button("Sphere"))
			{
				obj = sphere;
			}
			if (ImGui::Button("Debug off"))
			{
				debugMode = false;
				glfwSetCursorPosCallback(window, mouseCallback);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				firstMouse = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Lighting")) 
			{
				enableLighting = !enableLighting;
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();

			//ImGui::ShowDemoWindow();

			ImGui::SetNextWindowPos(ImVec2(10, 280));
			ImGui::SetNextWindowSize(ImVec2(350, 150));
			ImGui::Begin("Camera", 0, ImGuiWindowFlags_NoMove);
			ImGui::InputFloat("Movement speed", &camera.MovementSpeed, 0.01f, 1.0f);
			ImGui::InputFloat("Position X", &camera.Position[0], 0.01f, 1.0f);
			ImGui::InputFloat("Position Y", &camera.Position[1], 0.01f, 1.0f);
			ImGui::InputFloat("Position Z", &camera.Position[2], 0.01f, 1.0f);
			ImGui::End();

			ImGui::SetNextWindowPos(ImVec2(470, 50));
			ImGui::SetNextWindowSize(ImVec2(420, 200));
			ImGui::Begin("Lighting", 0);
			ImGui::LabelText("","Light source type");
			if (ImGui::Button("Point light"))
			{
				useSpotLight = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Spot light"))
			{
				useSpotLight = true;
			}
			ImGui::LabelText("", "Inner cutoff angle");
			ImGui::InputFloat("", &cutOffAngle, 0.1f, 1.0f);
			ImGui::LabelText("", "Outer cutoff angle");
			ImGui::InputFloat("", &outerCutOffAngle, 0.1f, 1.0f);

			ImGui::SliderFloat3("Light direction", &lightDirection[0], -2.0f, 2.0f);
			ImGui::End();
		}
		glUniform1i(u_useSpotLight, useSpotLight);
		if (useSpotLight)
		{
			glUniform3f(u_lightDirection, lightDirection[0] - lightPosition[0], lightDirection[1] - lightPosition[1], lightDirection[2] - lightPosition[2]);
			glUniform3f(u_lightPos, lightPosition[0], lightPosition[1], lightPosition[2]);
			glUniform1f(u_lightCutOff, glm::cos(glm::radians(cutOffAngle)));
			glUniform1f(u_lightOuterCutOff, glm::cos(glm::radians(outerCutOffAngle)));
		}

		ImGui::SetNextWindowPos(ImVec2(10, 10));
		ImGui::Begin("main", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
		ImGui::Text("Main");
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(mainScreenWidth+10, 10));
		ImGui::Begin("top", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
		ImGui::Text("Top");
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(mainScreenWidth+10, subScreenHeight+10));
		ImGui::Begin("front", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
		ImGui::Text("Front");
		ImGui::End();

		//top viewport
		glViewport(mainScreenWidth, subScreenHeight, subScreenWidth, subScreenHeight);

		proj = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -100.0f, 100.0f);
		view = glm::lookAt(glm::vec3(0, 7, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));

		viewPos = camera.GetPos();
		glUniform3f(u_viewPos, viewPos[0], viewPos[1], viewPos[2]);
		glUniform1i(u_enableLighting, false);
		glUniform3f(u_lightPos, lightPosition[0], lightPosition[1], lightPosition[2]);
		glUniform1i(u_useTextures, false);

		//main cube
		glUniform3f(u_color, 0.2f, 0.6f, 0.2f);
		model = glm::translate(glm::mat4(1.0f), translation);
		model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
		rotate = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = model * rotate;

		mvp = proj * view * model;

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

		switch (obj)
		{
		case cube: renderer.Draw(vaCube, ProgramID, cubeVertices.size() / 8);
			break;
		case sphere: renderer.Draw(vaSphere, ProgramID, sphereVertices.size() / 8);
			break;
		}

		//light
		glUniform3f(u_color, 1.0f, 1.0f, 1.0f);
		model = glm::translate(glm::mat4(1.0f), lightPosition);
		model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
		mvp = proj * view * model;

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

		renderer.Draw(vaSphere, ProgramID, sphereVertices.size() / 8);

		//camera
		glUniform3f(u_color, 1.0f, 0.0f, 0.0f);
		model = glm::translate(glm::mat4(1.0f), camera.GetPos());
		model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
		mvp = proj * view * model;

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

		renderer.Draw(vaCube, ProgramID, cubeVertices.size() / 8);

		//front viewport
		glViewport(mainScreenWidth, 0, subScreenWidth, subScreenHeight);

		proj = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -100.0f, 100.0f);
		view = glm::lookAt(glm::vec3(0, 0, 7), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		glUniform1i(u_enableLighting, false);
		glUniform1i(u_useTextures, false);

		//main cube
		glUniform3f(u_color, 0.2f, 0.6f, 0.2f);
		model = glm::translate(glm::mat4(1.0f), translation);
		model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
		rotate = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = model * rotate;

		mvp = proj * view * model;

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

		switch (obj)
		{
		case cube: renderer.Draw(vaCube, ProgramID, cubeVertices.size() / 8);
			break;
		case sphere: renderer.Draw(vaSphere, ProgramID, sphereVertices.size() / 8);
			break;
		}

		//light
		glUniform3f(u_color, 1.0f, 1.0f, 1.0f);
		model = glm::translate(glm::mat4(1.0f), lightPosition);
		model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
		mvp = proj * view * model;

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

		renderer.Draw(vaSphere, ProgramID, sphereVertices.size() / 8);

		//camera
		glUniform3f(u_color, 1.0f, 0.0f, 0.0f);
		model = glm::translate(glm::mat4(1.0f), camera.GetPos());
		model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
		mvp = proj * view * model;

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

		renderer.Draw(vaCube, ProgramID, cubeVertices.size() / 8);

		//coordinate system
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, 100, 100);
		proj = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -100.0f, 100.0f);
		view = glm::lookAt(camera.GetPos(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		//view = glm::lookAt(camera.GetPos(), camera.Front, glm::vec3(0, 1, 0));

		glUniform1i(u_enableLighting, false);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

		mvp = proj * view * model;

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
		glUniform3f(u_color, 1.0f, 1.0f, 1.0f);
		renderer.DrawLines(vaCoordinates, ib, ProgramID);

		//gui
		glViewport(0, 0, mainScreenWidth + subScreenWidth, mainScreenHeight);
		proj = glm::ortho(0.0f, (float)(mainScreenWidth+subScreenWidth), 0.0f, (float)mainScreenHeight, -100.0f, 100.0f);
		view = glm::lookAt(glm::vec3(0, 0, 7), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

		mvp = proj * view * model;

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
		glUniform3f(u_color, 1.0f, 1.0f, 1.0f);

		renderer.DrawLines(vaGui, ibGui, ProgramID);

		glEnable(GL_DEPTH_TEST);

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);

	}
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	exit(0);

	return 0;
}