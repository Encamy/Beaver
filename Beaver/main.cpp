//C
#include <cstdlib>
#include <thread>
#include <iostream>
#include <cmath>
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

#define SQR(x) (x) * (x)

GLFWwindow* window;
int screenWidth = 800, screenHeight = 600;

float lastX = screenWidth / 2.0f;
float lastY = screenHeight / 2.0f;
bool firstMouse = true;

bool keys[1024];
bool debugMode = false;
bool openDoor = false;
bool closeDoor = false;
int curDoor = 0;
//float sin_door = 0;

std::vector <bool> doorStates; //false - closed, true - opened;
std::vector <GLfloat> doorAngles;
std::vector <GLfloat> sinDoors;
std::vector <glm::vec3> stonePos;

std::vector <glm::vec3> lightsPos;

Camera camera(glm::vec3(0.0f, 2.0f, 5.0f));

std::vector<std::string> objs = { "landscape", "house", "fence", "windows", "main_door", "stone", "sphere", "cube" };

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void commandsListener();

int main() {
	bool enableLighting = true;
	bool enableTextures = true;
	bool polygonMode = false;
	doorStates.push_back(false);
	doorStates.push_back(false);
	doorStates.push_back(false);
	doorStates.push_back(false);
	doorAngles.push_back(0.0f);
	doorAngles.push_back(0.0f);
	doorAngles.push_back(0.0f);
	doorAngles.push_back(0.0f);
	sinDoors.push_back(0.0f);
	sinDoors.push_back(0.0f);
	sinDoors.push_back(0.0f);
	sinDoors.push_back(0.0f);

	stonePos.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	stonePos.push_back(glm::vec3(2.4f, 0.3f, 4.4f));
	stonePos.push_back(glm::vec3(-3.2f, -0.73f, -5.9f));

	std::vector <glm::vec3> lightsPos;
	std::vector <glm::vec3> lightsColor;

	log.LOG_TRACE("Creating command listener");
	std::thread listenerThread(commandsListener);
	log.LOG_TRACE("Command listener created!");

	Renderer renderer(screenWidth, screenHeight, &window);

	log.LOG_INFO("GLSL compiler can optimize unused uniforms");
	log.LOG_INFO("Just ignore \"Invalid location of iniform\" error!");

	glfwSetKeyCallback(window, keyCallback);
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
	int u_specularStregth = glGetUniformLocation(ProgramID, "specularStrength");

	glUseProgram(0);

	int u_texture = glGetUniformLocation(ProgramID, "u_Texture");
	glUniform1i(u_texture, 0);

	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();

	glm::vec3 transition(1, 1, 0);
	lightsPos.push_back(glm::vec3(-2.3f, 7.0f, 5.0f));
	lightsPos.push_back(glm::vec3(2.6f, 2.85f, 1.13f));
	lightsPos.push_back(glm::vec3(6.2f, 2.37f, -0.12f));
	lightsColor.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
	lightsColor.push_back(glm::vec3(0.2f, 0.2f, 0.2f));
	lightsColor.push_back(glm::vec3(0.2f, 0.2f, 0.2f));
	bool lightOn[] = { true, true, true };

	std::vector<glm::vec3> doorTransitions;
	glm::vec3 mainDoorTransition(-0.75f, 2.005f, 0.0f);
	doorTransitions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	doorTransitions.push_back(glm::vec3(-0.75f, 2.005f, 0.0f));
	doorTransitions.push_back(glm::vec3(-0.75f, 0.0f, -6.62f));
	doorTransitions.push_back(glm::vec3(-1.5f, 0.0f, -6.62f));
	GLfloat angle = 0;
	GLfloat door_angle = 0;

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		GlClearError();

		renderer.Clear();

		if (polygonMode)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

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
		if (curDoor != -1)
		{
			if (openDoor && doorAngles[curDoor] < 90.0f)
			{
				sinDoors[curDoor] += 5.0f;
				if (curDoor == 2)
				{
					doorAngles[3] = sin(glm::radians(0.5 * (-sinDoors[curDoor]))) * 90;
				}
				doorAngles[curDoor] = sin(glm::radians(0.5 * sinDoors[curDoor])) * 90;
			}
			if (doorAngles[curDoor] >= 90.0f)
			{
				doorStates[curDoor] = true;
				openDoor = false;
			}
			if (closeDoor && doorAngles[curDoor] > 0.0f)
			{
				sinDoors[curDoor] -= 5.0f;
				if (curDoor == 2)
				{
					doorAngles[3] = sin(glm::radians(0.5 * (-sinDoors[curDoor]))) * 90;
				}
				doorAngles[curDoor] = sin(glm::radians(0.5 * sinDoors[curDoor])) * 90;
			}
			if (doorAngles[curDoor] <= 0.0f)
			{
				doorStates[curDoor] = false;
				closeDoor = false;
			}
		}

		glUseProgram(ProgramID);

		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glm::vec3 viewPos = camera.GetPos();
		glUniform3f(u_viewPos, viewPos[0], viewPos[1], viewPos[2]);
		glUniform1i(u_enableLighting, enableLighting);
		glUniform1i(u_useTextures, enableTextures);
		glUniform3f(u_color, 1.0, 1.0, 1.0);
		glm::mat4 model;
		glm::mat4 mvp;

		for (size_t i = 0; i < lightsPos.size(); i++)
		{
			int pos = glGetUniformLocation(ProgramID, (std::string("lightPositions[") + std::to_string(i) + std::string("]")).c_str());
			glUniform3f(pos, lightsPos[i][0], lightsPos[i][1], lightsPos[i][2]);
			pos = glGetUniformLocation(ProgramID, (std::string("lightColor[") + std::to_string(i) + std::string("]")).c_str());
			glUniform3f(pos, lightsColor[i][0], lightsColor[i][1], lightsColor[i][2]);
			pos = glGetUniformLocation(ProgramID, (std::string("lightTurnOn[") + std::to_string(i) + std::string("]")).c_str());
			glUniform1i(pos, lightOn[i]);
		}

		for (size_t i = 0; i < objs.size(); i++)
		{
			glUniform1f(u_specularStregth, 0.5f);
			if (objs[i] == "landscape")
				glUniform1f(u_specularStregth, 0.15f);
			if (objs[i] == "main_door")
				for (int j = 0; j < 4; j++)
				{
					model = glm::translate(glm::mat4(1.0f), transition);
					model = model * glm::translate(glm::mat4(1.0f), doorTransitions[j]);
					glm::mat4 rotate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

					if (j == 2)
					{
						rotate = glm::translate(glm::mat4(1.0f), glm::vec3(4.687f, 0.177f, -0.238f));
						rotate = rotate * glm::rotate(glm::mat4(1.0f), glm::radians(doorAngles[j]), glm::vec3(0.0f, 1.0f, 0.0f));
						rotate = rotate * glm::translate(glm::mat4(1.0f), glm::vec3(-4.687f, -0.177f, 0.238f));
					}
					else
					{
						rotate = glm::translate(glm::mat4(1.0f), glm::vec3(3.287f, 0.177f, -0.238f));
						rotate = rotate * glm::rotate(glm::mat4(1.0f), glm::radians(doorAngles[j]), glm::vec3(0.0f, 1.0f, 0.0f));
						rotate = rotate * glm::translate(glm::mat4(1.0f), glm::vec3(-3.287f, -0.177f, 0.238f));
					}

					model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
					model = model * rotate;
					mvp = proj * view * model;

					glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
					glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

					textures[i]->Bind();
					renderer.Draw(*vas[i], ProgramID, vertices[i].size() / 8);
				}
			else if (objs[i] == "stone")
			{
				for (size_t j = 0; j < stonePos.size(); j++)
				{
					model = glm::translate(glm::mat4(1.0f), transition);
					model = model * glm::translate(glm::mat4(1.0f), stonePos[j]);
					glm::mat4 rotate = glm::translate(glm::mat4(1.0f), glm::vec3(-5.895f, -1.474f, 0.362f));
					rotate = rotate * glm::rotate(glm::mat4(1.0f), glm::radians(40.0f*j), glm::vec3(1.0f, 1.0f, 1.0f));
					rotate = rotate * glm::translate(glm::mat4(1.0f), glm::vec3(5.895f, 1.474f, -0.362));
					model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
					model = model * rotate;
					mvp = proj * view * model;

					glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
					glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

					textures[i]->Bind();
					renderer.Draw(*vas[i], ProgramID, vertices[i].size() / 8);
				}
			}
			else if (objs[i] == "sphere")
			{
				for (size_t j = 0; j < lightsPos.size(); j++)
				{
					model = glm::translate(glm::mat4(1.0f), lightsPos[j]);
					glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
					model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
					model = model * rotate;
					mvp = proj * view * model;

					glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
					glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

					textures[i]->Bind();
					renderer.Draw(*vas[i], ProgramID, vertices[i].size() / 8);
				}
			}
			else if (objs[i] == "cube")
			{
				model = glm::translate(glm::mat4(1.0f), glm::vec3(-4.75f, 0.0f, 2.4f));
				//glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
				//model = model * rotate;
				mvp = proj * view * model;
				glUniform1i(u_useTextures, false);
				glUniform3f(u_color, 0.0f, 1.0f, 1.0f);

				glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
				glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

				textures[i]->Bind();
				renderer.Draw(*vas[i], ProgramID, vertices[i].size() / 8);

				glUniform1i(u_useTextures, true);
			}
			else
			{
				model = glm::translate(glm::mat4(1.0f), transition);
				glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
				model = model * rotate;
				mvp = proj * view * model;

				glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
				glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

				textures[i]->Bind();
				renderer.Draw(*vas[i], ProgramID, vertices[i].size() / 8);
			}
		}

		OpenGlError();

		if (debugMode)
		{
			ImGui_ImplGlfwGL3_NewFrame();

			ImGui::Begin("Debug", 0);
			ImGui::SliderFloat3("Transition", &transition.x, -5.0f, 5.0f);
			ImGui::SliderFloat3("Light 1 Pos", &lightsPos[0][0], -5.0f, 5.0f);
			ImGui::SliderFloat3("Light 2 Pos", &lightsPos[1][0], -5.0f, 5.0f);
			ImGui::SliderFloat3("Light 3 Pos", &lightsPos[2][0], -5.0f, 5.0f);
			if (ImGui::Button("Light 1"))
			{
				lightOn[0] = !lightOn[0];
			}
			ImGui::SameLine();
			if (ImGui::Button("Light 2"))
			{
				lightOn[1] = !lightOn[1];
			}
			ImGui::SameLine();
			if (ImGui::Button("Light 3"))
			{
				lightOn[2] = !lightOn[2];
			}
			if (ImGui::Button("Polygon mode"))
			{
				polygonMode = !polygonMode;
			}
			ImGui::SameLine();
			if (ImGui::Button("Lighting enable / disable")) 
			{
				enableLighting = !enableLighting;
			}
			ImGui::SameLine();
			if (ImGui::Button("Texturing"))
			{
				enableTextures = !enableTextures;
			}
			if (ImGui::Button("Hide"))
			{
				debugMode = false;
				glfwSetCursorPosCallback(window, MouseCallback);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				firstMouse = true;
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

	if (key == GLFW_KEY_F)
	{
		glm::vec3 p1 = camera.GetPos();
		std::vector<glm::vec3> pos;
		pos.push_back(glm::vec3(3.0f, 1.38f, 0.0f));		//main
		pos.push_back(glm::vec3(2.3f, 3.7f, 0.0f));			//top
		pos.push_back(glm::vec3(1.8f, 1.8f, -6.7f));		//double door
		pos.push_back(glm::vec3(1.8f, 1.8f, -6.7f));		//double door
		float d = 9999;
		for (int i = 0; i < pos.size(); i++)
		{
			float d1 = sqrt(SQR(pos[i].x - p1.x) + SQR(pos[i].y - p1.y) + SQR(pos[i].z - p1.z));
			if (d1 < d)
			{
				d = d1;
				curDoor = i;
			}
		}
		if (d <= 2.2)
		{
			if (!doorStates[curDoor])
				openDoor = true;
			else
				closeDoor = true;
		}
		else
			curDoor = -1;
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