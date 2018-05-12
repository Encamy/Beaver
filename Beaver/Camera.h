#ifndef CAMERA_H

#include <GL\glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float ROLL = 0.0f;
const float SPEED = 0.2f;
const float SENSITIVITY = 0.1f;

class Camera
{
private:
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	//https://ru.wikipedia.org/wiki/Связанная_система_координат 
	float Yaw;
	float Pitch;
	float Roll; //Not used now, maybe will be usefull later

	float MouseSensitivity;

	void updateCameraVectors();
public:
	glm::vec3 Position;
	float MovementSpeed;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
	~Camera();

	void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	
	glm::mat4 GetViewMatrix();
	glm::vec3 Camera::GetPos();
};

#endif