#include "Camera.h"

Camera::Camera() {}

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch, float startMoveSpeed = 1.0f, float startTurnSpeed = 1.0f)
{
	this->position = startPosition;
	this->worldUp = startUp;
	this->up = startUp;
	this->yaw = startYaw;
	this->pitch = startPitch;
	this->front = glm::vec3(0.0f, 0.0f, -2.0f);
	this->moveSpeed = startMoveSpeed;
	this->turnSpeed = startTurnSpeed;

	this->Update();
}

void Camera::keyControl(bool* keys, float deltaTime)
{
	float velocity = moveSpeed * deltaTime;

	if (keys[GLFW_KEY_W]) {
		position += front * velocity;  // Elõre mozgás
	}

	if (keys[GLFW_KEY_S]) {
		position -= front * velocity;  // Hátra mozgás
	}

	if (keys[GLFW_KEY_A]) {
		position -= right * velocity;  // Balra mozgás
	}

	if (keys[GLFW_KEY_D]) {
		position += right * velocity;  // Jobbra mozgás
	}

	// Felfelé mozgás, ha a spacebar lenyomva
	if (keys[GLFW_KEY_SPACE]) {
		position += up * velocity;  // Felfelé mozgás (Y tengely)
	}

	// Lefelé mozgás, ha a Ball Ctrl lenyomva
	if (keys[GLFW_KEY_LEFT_CONTROL]) {
		position -= up * velocity;  // Lefelé mozgás (Y tengely)
	}

}

void Camera::mouseControl(float xChange, float yChange)
{
	xChange *= this->turnSpeed;
	yChange *= this->turnSpeed;

	this->yaw += xChange;
	this->pitch += yChange;

	if (this->pitch > 89.0f)
	{
		this->pitch = 89.0f;
	}

	if (this->pitch < -89.0f)
	{
		this->pitch = -89.0f;
	}

	this->Update();
}

void Camera::Update()
{
	this->front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	this->front.y = sin(glm::radians(this->pitch));
	this->front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	this->front = glm::normalize(this->front);

	this->right = glm::normalize(glm::cross(this->front, this->worldUp));
	this->up = glm::normalize(glm::cross(this->right, this->front));
}

glm::vec3 Camera::getPosition()
{
	return this->position;
}

void Camera::setPosition(glm::vec3 newPosition)
{
	this->position = newPosition;
}

glm::vec3 Camera::getFront()
{
	return this->front;
}

void Camera::setFront(glm::vec3 newFront)
{
	this->front = newFront;
}

glm::vec3 Camera::getUp()
{
	return this->up;
}

void Camera::setUp(glm::vec3 newUp)
{
	this->up = newUp;
}


Camera::~Camera()
{
}
