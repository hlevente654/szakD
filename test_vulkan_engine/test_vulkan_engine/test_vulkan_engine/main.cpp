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

	int seahawk = vulkanRenderer.createMeshModel("Models/Seahawk.obj", false, { {-100.0f}, {0.0f}, {80.0f} }, false, { {0.0f}, {0.0f}, {0.0f} });
	modelIds.push_back(seahawk);
	int ground = vulkanRenderer.createMeshModel("Models/ground.obj", false, { {0.0f}, {-20.0f}, {0.0f} }, false, { {0.0f}, {0.0f}, {0.0f} });
	modelIds.push_back(ground);
	int flashlight = vulkanRenderer.createMeshModel("Models/flashlight.obj", true, { {0.0f}, {0.0f}, {0.0f} }, true, { {(-1.0f)}, {(0.0f)}, {(0.0f)} });
	modelIds.push_back(flashlight);

	
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

		// lighting
		// Lighting
			vulkanRenderer.uboLighting.ambiantLightColor = glm::vec3(1.0f, 1.0f, 1.0f); // Ambient f�ny sz�ne
			vulkanRenderer.uboLighting.ambiantStr = 0.2f; // Ambient er�ss�ge

			// Spotlight be�ll�t�sa
			glm::vec3 flashlightDirection = vulkanRenderer.getMeshModel(flashlight)->getDirection();
			float offset = 1.0f;  // Az eltol�s m�rt�ke
			glm::vec3 flashlightFront = vulkanRenderer.getMeshModel(flashlight)->getPosition() + flashlightDirection * offset;

			glm::vec3 flashlightPosition = vulkanRenderer.getMeshModel(flashlight)->getPosition();
			vulkanRenderer.uboLighting.spotlight[0].lightPosition = flashlightPosition;

			vulkanRenderer.uboLighting.spotlight[0].lightDirection = flashlightDirection; // Ir�ny
			vulkanRenderer.uboLighting.spotlight[0].lightColor = glm::vec3(0.5f, 0.5f, 0.5f);  // F�ny sz�ne
			vulkanRenderer.uboLighting.spotlight[0].diffuseStr = 0.8f; // Diff�z f�ny
			vulkanRenderer.uboLighting.spotlight[0].specularStr = 0.8f; // Specul�ris f�ny
			vulkanRenderer.uboLighting.spotlight[0].shininess = 12.0f;  // Specul�ris �less�g

			// Spotlight sz�gek be�ll�t�sa
			vulkanRenderer.uboLighting.spotlight[0].innerCutOff = glm::cos(glm::radians(15.0f)); // Sz�k bels� sz�g
			vulkanRenderer.uboLighting.spotlight[0].outerCutOff = glm::cos(glm::radians(25.0f));


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