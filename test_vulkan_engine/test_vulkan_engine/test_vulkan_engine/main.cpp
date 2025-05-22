#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
	std::vector<int> modelIds;

	// Looad modells
	int seahawk = vulkanRenderer.createMeshModel("Models/Seahawk.obj", false, { {200.0f}, {-20.0f}, {0.0f} }, false, { {0.0f}, {0.0f}, {0.0f} });
	modelIds.push_back(seahawk);
	int ground = vulkanRenderer.createMeshModel("Models/ground.obj", false, { {0.0f}, {-20.0f}, {0.0f} }, false, { {0.0f}, {0.0f}, {0.0f} });
	modelIds.push_back(ground);
	int flashlight = vulkanRenderer.createMeshModel("Models/flashlight.obj", true, { {0.0f}, {0.0f}, {0.0f} }, true, { {(-1.0f)}, {(0.0f)}, {(0.0f)} });
	modelIds.push_back(flashlight);

	
	// Main loop
	while (!glfwWindowShouldClose(window.mainWindow))
	{
		// Update events
		glfwPollEvents();

		//Add key and mouse controll
		camera.keyControl(window.getsKeys(), deltaTime);
		camera.mouseControl(window.getXChange(), window.getYChange());
		
		// Use delta time
		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// Set lighting related variables
		vulkanRenderer.setLighting(flashlight);

		// update camera
		vulkanRenderer.updateView();

		// update models
		for (size_t i = 0; i < modelIds.size(); i++)
		{
			vulkanRenderer.getMeshModel(modelIds[i])->keyControl(window.getsKeys(), deltaTime, 8.0f, 10.0f);
		}
		

		vulkanRenderer.draw();
	}

	vulkanRenderer.cleanup();

	// Destroy GLFW window and stop GLFW
	glfwDestroyWindow(window.mainWindow);
	glfwTerminate();

	return 0;
}