#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera
{
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch, float startMoveSpeed, float startTurnSpeed);

	void keyControl(bool* keys, float deltaTime);
	void mouseControl(float xChange, float yChange);

	glm::mat4 calculateViewMatrix();

	glm::vec3 getPosition();
	void setPosition(glm::vec3 newPosition);

	glm::vec3 getFront();
	void setFront(glm::vec3 newFront);

	glm::vec3 getUp();
	void setUp(glm::vec3 newUp);

	~Camera();

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	float yaw;
	float pitch;

	float moveSpeed;
	float turnSpeed;

	void Update();
};

