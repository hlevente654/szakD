#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>

#include "VulkanRenderer.h"
#include "Window.h"
#include "Camera.h"

VulkanRenderer vulkanRenderer;
Camera camera;

int main()
{
	// Create Window
	Window window = Window(1600, 900, "Vulkan");
	
	// create camera
	camera = Camera(glm::vec3(50.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 10.0f, 0.5f);

	// Create Vulkan Renderer instance
	if (vulkanRenderer.init(window.mainWindow, &camera) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	float angle = 0.0f;
	float deltaTime = 0.0f;
	float lastTime = 0.0f;

	int fullModel = vulkanRenderer.createMeshModel("Models/Seahawk.obj");

	// Loop until closed
	while (!glfwWindowShouldClose(window.mainWindow))
	{
		glfwPollEvents();
		camera.keyControl(window.getsKeys(), deltaTime);
		camera.mouseControl(window.getXChange(), window.getYChange());

		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		angle += 10.0f * deltaTime;
		if (angle > 360.0f) { angle -= 360.0f; }

		// update camera
		//vulkanRenderer.updateView();

		// update model
		//glm::mat4 testMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		//testMat = glm::rotate(testMat, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//vulkanRenderer.updateModel(fullModel, testMat);


		vulkanRenderer.draw();
	}

	vulkanRenderer.cleanup();

	// Destroy GLFW window and stop GLFW
	glfwDestroyWindow(window.mainWindow);
	glfwTerminate();

	return 0;
}