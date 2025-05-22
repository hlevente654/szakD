#include "Camera.h"

Camera::Camera() {}

/**
 * @brief Constructs a Camera object with specified parameters.
 *
 * Initializes the camera's position, orientation, and movement parameters.
 * Calls Update() to ensure the direction vectors are properly initialized.
 *
 * @param startPosition Initial position of the camera in world space.
 * @param startUp Initial up vector defining the global "up" direction.
 * @param startYaw Initial yaw angle (horizontal rotation).
 * @param startPitch Initial pitch angle (vertical rotation).
 * @param startMoveSpeed Movement speed of the camera (default = 1.0f).
 * @param startTurnSpeed Mouse sensitivity for rotation (default = 1.0f).
 */
Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch, float startMoveSpeed, float startTurnSpeed)
{
	this->position = startPosition;
	this->worldUp = startUp;
	this->up = startUp;
	this->yaw = startYaw;
	this->pitch = startPitch;
	this->front = glm::vec3(0.0f, 0.0f, -2.0f); // Initial default forward direction
	this->moveSpeed = startMoveSpeed;
	this->turnSpeed = startTurnSpeed;

	this->Update(); // Recalculate front, right, and up vectors
}


/**
 * @brief Handles movement input based on pressed keys.
 *
 * Moves the camera in the appropriate direction based on the pressed keys.
 * Movement is scaled by deltaTime to ensure consistent speed regardless of frame rate.
 *
 * @param keys Boolean array representing the state of keys.
 * @param deltaTime Time elapsed since the last frame, used to maintain frame-independent movement.
 */
void Camera::keyControl(bool* keys, float deltaTime)
{
	float velocity = moveSpeed * deltaTime;

	// Move forward
	if (keys[GLFW_KEY_W]) {
		position += front * velocity;
	}

	// Move backward
	if (keys[GLFW_KEY_S]) {
		position -= front * velocity;
	}

	// Move left (strafe)
	if (keys[GLFW_KEY_A]) {
		position -= right * velocity;
	}

	// Move right (strafe)
	if (keys[GLFW_KEY_D]) {
		position += right * velocity;
	}

	// Move up (ascend)
	if (keys[GLFW_KEY_SPACE]) {
		position += up * velocity;
	}

	// Move down (descend)
	if (keys[GLFW_KEY_LEFT_CONTROL]) {
		position -= up * velocity;
	}
}


/**
 * @brief Handles mouse movement input to rotate the camera.
 *
 * Updates the yaw (horizontal rotation) and pitch (vertical rotation) based on
 * mouse movement. Limits the pitch angle to avoid flipping the camera.
 *
 * @param xChange Change in the X-axis (horizontal mouse movement).
 * @param yChange Change in the Y-axis (vertical mouse movement).
 */
void Camera::mouseControl(float xChange, float yChange)
{
	// Scale the change by the camera's turn speed (sensitivity).
	xChange *= this->turnSpeed;
	yChange *= this->turnSpeed;

	// Adjust yaw (left/right rotation) and pitch (up/down rotation).
	this->yaw += xChange;
	this->pitch += yChange;

	// Clamp pitch to prevent the camera from flipping upside down.
	if (this->pitch > 89.0f)
	{
		this->pitch = 89.0f;
	}
	if (this->pitch < -89.0f)
	{
		this->pitch = -89.0f;
	}

	// Update the camera's orientation vectors.
	this->Update();
}


/**
 * @brief Updates the camera's direction vectors based on yaw and pitch angles.
 *
 * Recalculates the front, right, and up vectors using trigonometric functions.
 * This ensures the camera moves and orients correctly in 3D space.
 */
void Camera::Update()
{
	// Calculate the new front vector based on yaw and pitch.
	this->front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	this->front.y = sin(glm::radians(this->pitch));
	this->front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));

	// Normalize the front vector to maintain consistent direction.
	this->front = glm::normalize(this->front);

	// Recalculate the right vector (perpendicular to front and worldUp).
	this->right = glm::normalize(glm::cross(this->front, this->worldUp));

	// Recalculate the up vector (perpendicular to right and front).
	this->up = glm::normalize(glm::cross(this->right, this->front));
}


/**
 * @brief Gets the current position of the camera.
 *
 * @return The camera's position as a glm::vec3.
 */
glm::vec3 Camera::getPosition()
{
	return this->position;
}

/**
 * @brief Sets the position of the camera.
 *
 * @param newPosition The new position to set.
 */
void Camera::setPosition(glm::vec3 newPosition)
{
	this->position = newPosition;
}

/**
 * @brief Gets the front direction of the camera.
 *
 * @return The camera's front direction as a glm::vec3.
 */
glm::vec3 Camera::getFront()
{
	return this->front;
}

/**
 * @brief Sets the front direction of the camera.
 *
 * @param newFront The new front direction vector.
 */
void Camera::setFront(glm::vec3 newFront)
{
	this->front = newFront;
}

/**
 * @brief Gets the up direction of the camera.
 *
 * @return The camera's up direction as a glm::vec3.
 */
glm::vec3 Camera::getUp()
{
	return this->up;
}

/**
 * @brief Sets the up direction of the camera.
 *
 * @param newUp The new up direction vector.
 */
void Camera::setUp(glm::vec3 newUp)
{
	this->up = newUp;
}



Camera::~Camera()
{
}
