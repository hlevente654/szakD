#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <GLFW/glfw3.h>

/**
 * @class Camera
 * @brief Represents a first-person-style camera for a 3D scene.
 *
 * This class handles movement and rotation based on keyboard and mouse input.
 * It provides functions for updating the view matrix, handling user input,
 * and managing the camera's position and orientation.
 */
class Camera
{
public:
    /**
     * @brief Default constructor. Initializes an empty camera instance.
     */
    Camera();

    /**
     * @brief Parameterized constructor to initialize the camera with specific settings.
     *
     * @param startPosition Initial position of the camera in world space.
     * @param startUp Initial up vector of the camera.
     * @param startYaw Initial yaw angle (horizontal rotation).
     * @param startPitch Initial pitch angle (vertical rotation).
     * @param startMoveSpeed Movement speed of the camera.
     * @param startTurnSpeed Mouse sensitivity (rotation speed).
     */
    Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch, float startMoveSpeed, float startTurnSpeed);

    /**
     * @brief Handles keyboard input for camera movement.
     *
     * @param keys A boolean array representing the state of keys.
     * @param deltaTime The time elapsed since the last frame (to ensure smooth movement).
     */
    void keyControl(bool* keys, float deltaTime);

    /**
     * @brief Handles mouse movement input for adjusting camera direction.
     *
     * @param xChange Change in mouse X position (horizontal movement).
     * @param yChange Change in mouse Y position (vertical movement).
     */
    void mouseControl(float xChange, float yChange);

    /**
     * @brief Calculates and returns the view matrix for rendering.
     *
     * @return The view matrix that transforms world coordinates to camera space.
     */
    glm::mat4 calculateViewMatrix();

    /**
     * @brief Gets the current position of the camera.
     *
     * @return The camera's position as a glm::vec3.
     */
    glm::vec3 getPosition();

    /**
     * @brief Sets a new position for the camera.
     *
     * @param newPosition The new position to set.
     */
    void setPosition(glm::vec3 newPosition);

    /**
     * @brief Gets the camera's front (view) direction.
     *
     * @return The front direction as a glm::vec3.
     */
    glm::vec3 getFront();

    /**
     * @brief Sets the front direction of the camera.
     *
     * @param newFront The new front direction vector.
     */
    void setFront(glm::vec3 newFront);

    /**
     * @brief Gets the camera's up direction.
     *
     * @return The up direction as a glm::vec3.
     */
    glm::vec3 getUp();

    /**
     * @brief Sets the up direction of the camera.
     *
     * @param newUp The new up vector.
     */
    void setUp(glm::vec3 newUp);

    /**
     * @brief Destructor. Cleans up camera resources if necessary.
     */
    ~Camera();

private:
    glm::vec3 position;  ///< Current position of the camera.
    glm::vec3 front;     ///< Front (view) direction of the camera.
    glm::vec3 up;        ///< Up direction of the camera.
    glm::vec3 right;     ///< Right direction (perpendicular to front and up).
    glm::vec3 worldUp;   ///< Global up vector (used for recalculating orientation).

    float yaw;           ///< Horizontal rotation angle (left/right).
    float pitch;         ///< Vertical rotation angle (up/down).

    float moveSpeed;     ///< Speed of movement.
    float turnSpeed;     ///< Speed of rotation (mouse sensitivity).

    /**
     * @brief Updates the camera's direction vectors based on yaw and pitch.
     *
     * This function recalculates the front, right, and up vectors whenever
     * the camera's rotation changes.
     */
    void Update();
};
