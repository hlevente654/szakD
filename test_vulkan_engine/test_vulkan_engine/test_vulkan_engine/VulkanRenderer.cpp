#include "VulkanRenderer.h"




/**
 * @brief Initializes the Vulkan renderer.
 *
 * This function sets up all necessary Vulkan components, including the instance,
 * physical and logical devices, swapchain, pipelines, and synchronization objects.
 * It also loads essential resources, such as textures.
 *
 * @param newWindow Pointer to the GLFW window that Vulkan will render to.
 * @param newCamera Pointer to the Camera object used for rendering.
 * @return Returns 0 on success, or EXIT_FAILURE if initialization fails.
 */
int VulkanRenderer::init(GLFWwindow* newWindow, Camera* newCamera)
{
	window = newWindow;
	camera = newCamera;

	try {
		// Core Vulkan setup
		createInstance();           ///< Create the Vulkan instance.
		setupDebugMessenger();      ///< Enable validation layers (if enabled).
		createSurface();            ///< Create the Vulkan rendering surface.
		getPhysicalDevice();        ///< Select a suitable GPU.
		createLogicalDevice();      ///< Create the Vulkan logical device.
		createSwapChain();          ///< Create the swapchain for frame buffering.
		createRenderPass();         ///< Define framebuffer attachments and rendering behavior.
		createDescriptorSetLayout(); ///< Define descriptor layouts for shader resources.
		createPushConstantRange();  ///< Create push constants for fast shader updates.
		createGraphicsPipeline();   ///< Build the rendering pipeline.
		createDepthBufferImage();   ///< Set up depth testing for 3D rendering.
		createFramebuffers();       ///< Create framebuffers for each swapchain image.
		createCommandPool();        ///< Create the command pool for rendering.
		createCommandBuffers();     ///< Allocate and record command buffers.
		createTextureSampler();     ///< Create a texture sampler for image filtering.

		// Shader resource allocation
		// allocateDynamicBufferTransferSpace(); ///< Uncomment if using dynamic UBOs.
		createUniformBuffers();      ///< Allocate buffers for transformation matrices.
		createDescriptorPool();      ///< Create a descriptor pool for resource binding.
		createDescriptorSets();      ///< Allocate descriptor sets for uniform buffers.

		// Synchronization setup
		createSynchronisation();     ///< Set up semaphores and fences.

		// Load default texture
		createTexture("plain.png");  ///< Load a default texture for untextured models.
	}
	catch (const std::runtime_error& e) {
		printf("ERROR: %s\n", e.what()); ///< Print error message on failure.
		return EXIT_FAILURE;
	}

	return 0;
}


/**
 * @brief Destroys the Vulkan debug messenger.
 *
 * This function retrieves the `vkDestroyDebugUtilsMessengerEXT` function dynamically
 * and calls it to clean up the debug messenger if it was created.
 *
 * @param instance The Vulkan instance.
 * @param debugMessenger The debug messenger handle to destroy.
 * @param pAllocator Optional memory allocator (can be nullptr).
 */
void VulkanRenderer::DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}


/**
 * @brief Creates a Vulkan debug messenger for validation layers.
 *
 * This function dynamically loads the `vkCreateDebugUtilsMessengerEXT` function
 * from the Vulkan instance and uses it to create a debug messenger.
 *
 * @param instance The Vulkan instance.
 * @param pCreateInfo Pointer to the debug messenger creation info structure.
 * @param pAllocator Optional memory allocator (can be nullptr).
 * @param pDebugMessenger Output parameter for the created debug messenger.
 * @return VK_SUCCESS if creation was successful, otherwise an error code.
 */
VkResult VulkanRenderer::CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}


/**
 * @brief Callback function for Vulkan validation layers.
 *
 * This function is triggered when a validation layer reports an issue.
 * It logs the validation message to standard error output.
 *
 * @param messageSeverity The severity of the validation message.
 * @param messageType The type of message (general, validation, performance).
 * @param pCallbackData Pointer to callback data containing the message.
 * @param pUserData User-defined data (not used in this implementation).
 * @return Always returns VK_FALSE.
 */
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}


/**
 * @brief Retrieves the required Vulkan instance extensions.
 *
 * This function queries GLFW for required Vulkan instance extensions and
 * adds the debug utilities extension if validation layers are enabled.
 *
 * @return A vector containing the required Vulkan instance extensions.
 */
std::vector<const char*> VulkanRenderer::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}


/**
 * @brief Checks if the requested validation layers are supported by the Vulkan instance.
 *
 * This function queries the available Vulkan layers and verifies that the requested validation layers
 * are present. If any requested layer is missing, the function returns false.
 *
 * @return True if all requested validation layers are supported, false otherwise.
 */
bool VulkanRenderer::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

/**
 * @brief Sets up the Vulkan debug messenger for validation layers.
 *
 * This function initializes the Vulkan debug messenger if validation layers are enabled.
 * It configures the debug callback function to capture Vulkan validation messages.
 */
void VulkanRenderer::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}


/**
 * @brief Configures lighting parameters for the scene.
 *
 * This function sets up ambient lighting and spotlight properties.
 * The spotlight is positioned based on a model's location and direction.
 *
 * @param source The model ID used as the light source.
 */
void VulkanRenderer::setLighting(int source)
{
	// Set ambient lighting color and intensity
	uboLighting.ambiantLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	uboLighting.ambiantStr = 0.2f;

	// Get the model's position and direction for the spotlight
	glm::vec3 flashlightDirection = getMeshModel(source)->getDirection();
	float offset = 1.0f;
	glm::vec3 flashlightFront = getMeshModel(source)->getPosition() + flashlightDirection * offset;
	glm::vec3 flashlightPosition = getMeshModel(source)->getPosition();

	// Adjust position to match Vulkan's coordinate system
	flashlightPosition = glm::vec3(flashlightPosition.z, flashlightPosition.x, flashlightPosition.y);

	// Set spotlight position and direction
	uboLighting.spotlight[0].lightPosition = glm::vec4(flashlightPosition, 0.0f);
	uboLighting.spotlight[0].lightDirection = flashlightDirection;

	// Configure spotlight properties
	uboLighting.spotlight[0].lightColor = glm::vec3(0.5f, 0.5f, 0.5f);
	uboLighting.spotlight[0].diffuseStr = 0.8f;
	uboLighting.spotlight[0].specularStr = 0.8f;
	uboLighting.spotlight[0].shininess = 12.0f;

	// Spotlight cutoff angles
	uboLighting.spotlight[0].innerCutOff = glm::cos(glm::radians(15.0f));
	uboLighting.spotlight[0].outerCutOff = glm::cos(glm::radians(25.0f));
}

/**
 * @brief Updates the transformation matrix of a specific model.
 *
 * This function modifies the transformation of a model by updating its model matrix.
 *
 * @param modelId The ID of the model to update.
 * @param newModel The new transformation matrix for the model.
 */
void VulkanRenderer::updateModel(int modelId, glm::mat4 newModel)
{
	if (modelId >= modelList.size()) return;

	modelList[modelId].setModel(newModel);
}

/**
 * @brief Updates the camera's view and projection matrices.
 *
 * This function recalculates the camera's view and projection matrices and updates
 * the uniform buffer for rendering.
 */
void VulkanRenderer::updateView()
{
	glm::vec3 cameraPosition = this->camera->getPosition();
	glm::vec3 cameraTarget = this->camera->getPosition() + this->camera->getFront();
	glm::vec3 upDirection = this->camera->getUp();

	// Set up the projection matrix (Perspective Projection)
	uboViewProjection.projection = glm::perspective(
		glm::radians(45.0f),
		(float)swapChainExtent.width / (float)swapChainExtent.height,
		0.1f,
		1000.0f
	);

	// Compute the camera view matrix
	uboViewProjection.view = glm::lookAt(cameraPosition, cameraTarget, upDirection);

	// Flip Y-axis for Vulkan coordinate system
	uboViewProjection.projection[1][1] *= -1;
}


/**
 * @brief Handles the rendering of a single frame in Vulkan.
 *
 * This function acquires the next available swapchain image, records commands,
 * submits the command buffer to the graphics queue, and presents the rendered
 * image to the screen.
 */
void VulkanRenderer::draw()
{
	// -- GET NEXT IMAGE --
	// Wait for the previous frame to finish rendering before continuing
	vkWaitForFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	// Reset the fence for the next frame
	vkResetFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame]);

	// Acquire the next image from the swapchain
	uint32_t imageIndex;
	vkAcquireNextImageKHR(mainDevice.logicalDevice, swapchain, std::numeric_limits<uint64_t>::max(),
		imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

	// Record and update command buffer for this image
	recordCommands(imageIndex);
	updateUniformBuffers(imageIndex);

	// -- SUBMIT COMMAND BUFFER TO RENDER --
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// Wait for the swapchain image to be available before rendering
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailable[currentFrame];

	// Wait at the color output stage before rendering
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitStages;

	// Submit the command buffer for this frame
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	// Signal the semaphore when rendering is finished
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinished[currentFrame];

	// Submit the command buffer to the graphics queue
	VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFences[currentFrame]);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit Command Buffer to Queue!");
	}

	// -- PRESENT RENDERED IMAGE TO SCREEN --
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	// Wait for the rendering to be finished before presenting
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinished[currentFrame];

	// Specify the swapchain and image index to present
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;

	// Present the image to the screen
	result = vkQueuePresentKHR(presentationQueue, &presentInfo);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present Image!");
	}

	// Move to the next frame (looping back if necessary)
	currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}

/**
 * @brief Cleans up Vulkan resources before shutting down the application.
 *
 * This function ensures that all allocated Vulkan objects, including buffers,
 * textures, pipelines, and synchronization primitives, are properly destroyed.
 */
void VulkanRenderer::cleanup()
{
	// Destroy debug messenger if validation layers are enabled
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	// Ensure all GPU operations are complete before cleanup
	vkDeviceWaitIdle(mainDevice.logicalDevice);

	// Free memory for model transfer space (if used)
	//_aligned_free(modelTransferSpace);

	// Destroy all loaded models
	for (size_t i = 0; i < modelList.size(); i++) {
		modelList[i].destroyMeshModel();
	}

	// Destroy texture samplers and descriptor layouts
	vkDestroyDescriptorPool(mainDevice.logicalDevice, samplerDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(mainDevice.logicalDevice, samplerSetLayout, nullptr);
	vkDestroySampler(mainDevice.logicalDevice, textureSampler, nullptr);

	// Destroy texture images and their associated memory
	for (size_t i = 0; i < textureImages.size(); i++) {
		vkDestroyImageView(mainDevice.logicalDevice, textureImageViews[i], nullptr);
		vkDestroyImage(mainDevice.logicalDevice, textureImages[i], nullptr);
		vkFreeMemory(mainDevice.logicalDevice, textureImageMemory[i], nullptr);
	}

	// Destroy depth buffer resources
	vkDestroyImageView(mainDevice.logicalDevice, depthBufferImageView, nullptr);
	vkDestroyImage(mainDevice.logicalDevice, depthBufferImage, nullptr);
	vkFreeMemory(mainDevice.logicalDevice, depthBufferImageMemory, nullptr);

	// Destroy descriptor pools and layouts
	vkDestroyDescriptorPool(mainDevice.logicalDevice, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(mainDevice.logicalDevice, descriptorSetLayout, nullptr);

	// Destroy uniform buffers and free allocated memory
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		vkDestroyBuffer(mainDevice.logicalDevice, vpUniformBuffer[i], nullptr);
		vkFreeMemory(mainDevice.logicalDevice, vpUniformBufferMemory[i], nullptr);
		vkDestroyBuffer(mainDevice.logicalDevice, lightingUniformBuffer[i], nullptr);
		vkFreeMemory(mainDevice.logicalDevice, lightingUniformBufferMemory[i], nullptr);
		// vkDestroyBuffer(mainDevice.logicalDevice, modelDUniformBuffer[i], nullptr);
		// vkFreeMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[i], nullptr);
	}

	// Destroy synchronization objects (semaphores and fences)
	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {
		vkDestroySemaphore(mainDevice.logicalDevice, renderFinished[i], nullptr);
		vkDestroySemaphore(mainDevice.logicalDevice, imageAvailable[i], nullptr);
		vkDestroyFence(mainDevice.logicalDevice, drawFences[i], nullptr);
	}

	// Destroy command pool
	vkDestroyCommandPool(mainDevice.logicalDevice, graphicsCommandPool, nullptr);

	// Destroy framebuffers
	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(mainDevice.logicalDevice, framebuffer, nullptr);
	}

	// Destroy pipeline and render pass
	vkDestroyPipeline(mainDevice.logicalDevice, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mainDevice.logicalDevice, pipelineLayout, nullptr);
	vkDestroyRenderPass(mainDevice.logicalDevice, renderPass, nullptr);

	// Destroy swapchain images and associated views
	for (auto image : swapChainImages) {
		vkDestroyImageView(mainDevice.logicalDevice, image.imageView, nullptr);
	}
	vkDestroySwapchainKHR(mainDevice.logicalDevice, swapchain, nullptr);

	// Destroy Vulkan surface
	vkDestroySurfaceKHR(instance, surface, nullptr);

	// Destroy Vulkan logical device and instance
	vkDestroyDevice(mainDevice.logicalDevice, nullptr);
	vkDestroyInstance(instance, nullptr);
}


VulkanRenderer::~VulkanRenderer()
{
}

/**
 * @brief Creates a Vulkan instance.
 *
 * The Vulkan instance initializes the Vulkan API and provides access to GPU features.
 * It also configures validation layers and required extensions.
 *
 * @throws std::runtime_error if validation layers are requested but unavailable,
 *                            or if Vulkan instance creation fails.
 */
void VulkanRenderer::createInstance()
{
	// Check if validation layers are available if enabled
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("Validation layers requested, but not available!");
	}

	// Fill in Vulkan application information
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan App";         // Custom application name
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";              // No custom engine
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;        // Vulkan version

	// Create instance creation information
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Retrieve required GLFW extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Store required extensions
	std::vector<const char*> instanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	// Ensure Vulkan instance supports required extensions
	if (!checkInstanceExtensionSupport(&instanceExtensions)) {
		throw std::runtime_error("VkInstance does not support required extensions!");
	}

	// Retrieve required extensions and assign to Vulkan create info
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Default layer configuration (no validation layers by default)
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;

	// If validation layers are enabled, configure them
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		// Populate debug messenger settings
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.pNext = nullptr;
	}

	// Create the Vulkan instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Vulkan Instance!");
	}
}


/**
 * @brief Populates debug messenger creation info.
 *
 * This function configures the Vulkan debug messenger for capturing validation
 * layer messages related to errors, warnings, and performance issues.
 *
 * @param createInfo Reference to VkDebugUtilsMessengerCreateInfoEXT structure.
 */
void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	// Define message severity levels
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |  // Informational messages
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |  // Warnings
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;     // Critical errors

	// Define message types to capture
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |      // General messages
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |   // Validation layer messages
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;   // Performance-related messages

	// Assign the debug callback function
	createInfo.pfnUserCallback = debugCallback;
}

/**
 * @brief Creates a Vulkan logical device.
 *
 * The logical device provides an interface to interact with the physical GPU,
 * allowing the application to submit rendering commands.
 *
 * @throws std::runtime_error if the logical device creation fails.
 */
void VulkanRenderer::createLogicalDevice()
{
	// Get the queue family indices for the selected physical device
	QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

	// Set of unique queue families required
	std::set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };

	// Queue creation information
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (int queueFamilyIndex : queueFamilyIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;  // Assign queue family index
		queueCreateInfo.queueCount = 1;  // Create a single queue per family
		float priority = 1.0f;
		queueCreateInfo.pQueuePriorities = &priority;  // Assign highest priority to the queue

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Logical device creation information
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	// Enable required device extensions (e.g., swapchain support)
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Specify physical device features (e.g., anisotropic filtering)
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;  // Enable anisotropic filtering
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Create the logical device
	VkResult result = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Logical Device!");
	}

	// Retrieve handles for the graphics and presentation queues
	vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(mainDevice.logicalDevice, indices.presentationFamily, 0, &presentationQueue);
}

/**
 * @brief Creates a Vulkan surface for rendering.
 *
 * The surface connects the Vulkan instance with the GLFW window,
 * allowing the application to present rendered images to the screen.
 *
 * @throws std::runtime_error if the surface creation fails.
 */
void VulkanRenderer::createSurface()
{
	// Create the Vulkan surface using GLFW
	VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a surface!");
	}
}

/**
 * @brief Creates the Vulkan swapchain.
 *
 * The swapchain is responsible for managing the images used for rendering and
 * presenting frames to the screen. This function configures and initializes
 * the swapchain based on the surface capabilities of the GPU.
 *
 * @throws std::runtime_error if the swapchain creation fails.
 */
void VulkanRenderer::createSwapChain()
{
	// Retrieve swapchain details from the physical device
	SwapChainDetails swapChainDetails = getSwapChainDetails(mainDevice.physicalDevice);

	// Select the best surface format, presentation mode, and swap extent
	VkSurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(swapChainDetails.formats);
	VkPresentModeKHR presentMode = chooseBestPresentationMode(swapChainDetails.presentationModes);
	VkExtent2D extent = chooseSwapExtent(swapChainDetails.surfaceCapabilities);

	// Determine the number of swapchain images (triple buffering if possible)
	uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;

	// Ensure image count does not exceed the maximum allowed
	if (swapChainDetails.surfaceCapabilities.maxImageCount > 0 &&
		swapChainDetails.surfaceCapabilities.maxImageCount < imageCount)
	{
		imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;
	}

	// Configure swapchain creation parameters
	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = surface;  // The window surface for rendering
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageArrayLayers = 1;  // Single layer (for 2D rendering)
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Render target usage
	swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform; // Apply pre-transform
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Ignore blending with external windows
	swapChainCreateInfo.clipped = VK_TRUE;  // Clip parts not visible to the user

	// Retrieve queue family indices for graphics and presentation
	QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

	// If the graphics and presentation queues are different, enable image sharing
	if (indices.graphicsFamily != indices.presentationFamily)
	{
		uint32_t queueFamilyIndices[] = {
			static_cast<uint32_t>(indices.graphicsFamily),
			static_cast<uint32_t>(indices.presentationFamily)
		};

		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainCreateInfo.queueFamilyIndexCount = 2;
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	// If replacing an old swapchain, link it for efficiency (not used here)
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create the swapchain
	VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice, &swapChainCreateInfo, nullptr, &swapchain);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Swapchain!");
	}

	// Store swapchain image format and resolution for later use
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	// Retrieve the swapchain images
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapChainImageCount, nullptr);
	std::vector<VkImage> images(swapChainImageCount);
	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapChainImageCount, images.data());

	// Create image views for each swapchain image
	for (VkImage image : images)
	{
		SwapchainImage swapChainImage = {};
		swapChainImage.image = image;
		swapChainImage.imageView = createImageView(image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		swapChainImages.push_back(swapChainImage);
	}
}

/**
 * @brief Creates the Vulkan render pass.
 *
 * The render pass defines how framebuffers are used during rendering.
 * It includes color and depth attachments and specifies subpass dependencies
 * for layout transitions.
 *
 * @throws std::runtime_error if the render pass creation fails.
 */
void VulkanRenderer::createRenderPass()
{
	// ATTACHMENTS
	// Colour attachment of render pass
	VkAttachmentDescription colourAttachment = {};
	colourAttachment.format = swapChainImageFormat;						// Format to use for attachment
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;					// Number of samples to write for multisampling
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// Describes what to do with attachment before rendering
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;			// Describes what to do with attachment after rendering
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// Describes what to do with stencil before rendering
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// Describes what to do with stencil after rendering

	// Framebuffer data will be stored as an image, but images can be given different data layouts
	// to give optimal use for certain operations
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// Image data layout before render pass starts
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// Image data layout after render pass (to change to)


	// Depth attachment of render pass
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = chooseSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// REFERENCES
	// Attachment reference uses an attachment index that refers to index in the attachment list passed to renderPassCreateInfo
	VkAttachmentReference colourAttachmentReference = {};
	colourAttachmentReference.attachment = 0;
	colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth Attachment Reference
	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Information about a particular subpass the Render Pass is using
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;		// Pipeline type subpass is to be bound to
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colourAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;

	// Need to determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 2> subpassDependencies;

	// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// Subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		// Pipeline stage
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// Stage access mask (memory access)
	// But must happen before...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;


	// Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Transition must happen after...
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
	// But must happen before...
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	std::array<VkAttachmentDescription, 2> renderPassAttachments = { colourAttachment, depthAttachment };

	// Create info for Render Pass
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
	renderPassCreateInfo.pAttachments = renderPassAttachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VkResult result = vkCreateRenderPass(mainDevice.logicalDevice, &renderPassCreateInfo, nullptr, &renderPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Render Pass!");
	}
}

/**
 * @brief Creates descriptor set layouts for uniform buffers and texture samplers.
 *
 * This function sets up descriptor bindings for view-projection matrices, lighting
 * information, and texture sampling, allowing shaders to access these resources.
 *
 * @throws std::runtime_error if descriptor set layout creation fails.
 */
void VulkanRenderer::createDescriptorSetLayout()
{
	// --- UNIFORM BUFFER DESCRIPTOR SET LAYOUT ---
	// View-Projection Uniform Buffer Binding
	VkDescriptorSetLayoutBinding vpLayoutBinding = {};
	vpLayoutBinding.binding = 0;  // Matches the binding number in the shader
	vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vpLayoutBinding.descriptorCount = 1;  // Single descriptor per set
	vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Used in the vertex shader
	vpLayoutBinding.pImmutableSamplers = nullptr;  // Not used for uniform buffers

	// --- LIGHTING UNIFORM BUFFER DESCRIPTOR SET LAYOUT ---
	VkDescriptorSetLayoutBinding lightBindingInfo = {};
	lightBindingInfo.binding = 1; // Binding point in the shader
	lightBindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightBindingInfo.descriptorCount = 1;
	lightBindingInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Used in the fragment shader
	lightBindingInfo.pImmutableSamplers = nullptr;

	// Combine descriptor bindings into a layout
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings = { vpLayoutBinding, lightBindingInfo };

	// Descriptor Set Layout creation info
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutCreateInfo.pBindings = layoutBindings.data();

	// Create descriptor set layout for uniform buffers
	VkResult result = vkCreateDescriptorSetLayout(mainDevice.logicalDevice, &layoutCreateInfo, nullptr, &descriptorSetLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Descriptor Set Layout!");
	}

	// --- TEXTURE SAMPLER DESCRIPTOR SET LAYOUT ---
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // Texture sampler
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Used in fragment shader
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	// Descriptor set layout creation info for texture sampling
	VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {};
	textureLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	textureLayoutCreateInfo.bindingCount = 1;
	textureLayoutCreateInfo.pBindings = &samplerLayoutBinding;

	// Create descriptor set layout for textures
	result = vkCreateDescriptorSetLayout(mainDevice.logicalDevice, &textureLayoutCreateInfo, nullptr, &samplerSetLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Descriptor Set Layout!");
	}
}

/**
 * @brief Defines the push constant range for the graphics pipeline.
 *
 * Push constants allow small amounts of data to be sent directly to shaders
 * with low overhead, enabling efficient per-draw updates.
 */
void VulkanRenderer::createPushConstantRange()
{
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // Used in the vertex shader
	pushConstantRange.offset = 0;  // Start at the beginning of the data block
	pushConstantRange.size = sizeof(Model);  // Size of the data being passed
}

/**
 * @brief Creates the Vulkan graphics pipeline.
 *
 * This function configures and initializes all stages of the graphics pipeline,
 * including shaders, vertex input, rasterization, depth testing, and blending.
 *
 * @throws std::runtime_error if the graphics pipeline creation fails.
 */
void VulkanRenderer::createGraphicsPipeline()
{
	// --- Load and Create Shader Modules ---
	auto vertexShaderCode = readFile("Shaders/vert.spv");
	auto fragmentShaderCode = readFile("Shaders/frag.spv");

	VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

	// --- Shader Stage Information ---
	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;				// Shader Stage name
	vertexShaderCreateInfo.module = vertexShaderModule;						// Shader module to be used by stage
	vertexShaderCreateInfo.pName = "main";									// Entry point in to shader

	// Fragment Stage creation information
	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
	fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;				// Shader Stage name
	fragmentShaderCreateInfo.module = fragmentShaderModule;						// Shader module to be used by stage
	fragmentShaderCreateInfo.pName = "main";									// Entry point in to shader


	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

	// How the data for a single vertex (including info such as position, colour, texture coords, normals, etc) is as a whole
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;									// Can bind multiple streams of data, this defines which one
	bindingDescription.stride = sizeof(Vertex);						// Size of a single vertex object
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;		// How to move between data after each vertex.
	// VK_VERTEX_INPUT_RATE_INDEX		: Move on to the next vertex
	// VK_VERTEX_INPUT_RATE_INSTANCE	: Move to a vertex for the next instance

// How the data for an attribute is defined within a vertex
	std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions;

	// Position Attribute
	attributeDescriptions[0].binding = 0;							// Which binding the data is at (should be same as above)
	attributeDescriptions[0].location = 0;							// Location in shader where data will be read from
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;	// Format the data will take (also helps define size of data)
	attributeDescriptions[0].offset = offsetof(Vertex, pos);		// Where this attribute is defined in the data for a single vertex

	// Colour Attribute
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, col);

	// Texture Attribute
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, tex);

	// Normal Attribute
	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Vertex, norm);


	// -- VERTEX INPUT --
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;											// List of Vertex Binding Descriptions (data spacing/stride information)
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();								// List of Vertex Attribute Descriptions (data format and where to bind to/from)


	// --- Input Assembly ---
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;		// Primitive type to assemble vertices as
	inputAssembly.primitiveRestartEnable = VK_FALSE;					// Allow overriding of "strip" topology to start new primitives


	// --- Viewport and Scissor ---
	VkViewport viewport = {};
	viewport.x = 0.0f;									// x start coordinate
	viewport.y = 0.0f;									// y start coordinate
	viewport.width = (float)swapChainExtent.width;		// width of viewport
	viewport.height = (float)swapChainExtent.height;	// height of viewport
	viewport.minDepth = 0.0f;							// min framebuffer depth
	viewport.maxDepth = 1.0f;							// max framebuffer depth

	// Create a scissor info struct
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };							// Offset to use region from
	scissor.extent = swapChainExtent;					// Extent to describe region to use, starting at offset

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	// --- Rasterizer ---
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;					// Change if fragments beyond near/far planes are clipped (default) or clamped to plane
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;			// Whether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer output
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;			// How to handle filling points between vertices
	rasterizerCreateInfo.lineWidth = 1.0f;								// How thick lines should be when drawn
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;				// Which face of a tri to cull
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	// Winding to determine which side is front
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;					// Whether to add depth bias to fragments (good for stopping "shadow acne" in shadow mapping)


	// --- Multisampling ---
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;					// Enable multisample shading or not
	multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;	// Number of samples to use per fragment


	// --- Color Blending ---andled)
	VkPipelineColorBlendAttachmentState colourState = {};
	colourState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT	// Colours to apply blending to
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colourState.blendEnable = VK_TRUE;													// Enable blending

	// Blending uses equation: (srcColorBlendFactor * new colour) colorBlendOp (dstColorBlendFactor * old colour)
	colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourState.colorBlendOp = VK_BLEND_OP_ADD;

	colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourState.alphaBlendOp = VK_BLEND_OP_ADD;
	// Summarised: (1 * new alpha) + (0 * old alpha) = new alpha

	VkPipelineColorBlendStateCreateInfo colourBlendingCreateInfo = {};
	colourBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colourBlendingCreateInfo.logicOpEnable = VK_FALSE;				// Alternative to calculations is to use logical operations
	colourBlendingCreateInfo.attachmentCount = 1;
	colourBlendingCreateInfo.pAttachments = &colourState;

	// -- PIPELINE LAYOUT --
	std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = { descriptorSetLayout, samplerSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	// Create Pipeline Layout
	VkResult result = vkCreatePipelineLayout(mainDevice.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}


	// --- Depth Stencil ---
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;				// Enable checking depth to determine fragment write
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;				// Enable writing to depth buffer (to replace old values)
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;		// Comparison operation that allows an overwrite (is in front)
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;		// Depth Bounds Test: Does the depth value exist between two bounds
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;			// Enable Stencil Test


	// --- Create Graphics Pipeline ---
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;									// Number of shader stages
	pipelineCreateInfo.pStages = shaderStages;							// List of shader stages
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;		// All the fixed function pipeline states
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colourBlendingCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;							// Pipeline Layout pipeline should use
	pipelineCreateInfo.renderPass = renderPass;							// Render pass description the pipeline is compatible with
	pipelineCreateInfo.subpass = 0;										// Subpass of render pass to use with pipeline

	// Pipeline Derivatives : Can create multiple pipelines that derive from one another for optimisation
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;	// Existing pipeline to derive from...
	pipelineCreateInfo.basePipelineIndex = -1;				// or index of pipeline being created to derive from (in case creating multiple at once)

	// Create Graphics Pipeline
	result = vkCreateGraphicsPipelines(mainDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Graphics Pipeline!");
	}

	// Destroy Shader Modules, no longer needed after Pipeline created
	vkDestroyShaderModule(mainDevice.logicalDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule, nullptr);
}

/**
 * @brief Creates the depth buffer image and its corresponding image view.
 *
 * The depth buffer is used for depth testing, ensuring that objects in the scene
 * are correctly rendered based on their distance from the camera.
 */
void VulkanRenderer::createDepthBufferImage()
{
	// Select a supported depth format
	VkFormat depthFormat = chooseSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	// Create the depth buffer image
	depthBufferImage = createImage(
		swapChainExtent.width, swapChainExtent.height,
		depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&depthBufferImageMemory
	);

	// Create an image view for the depth buffer
	depthBufferImageView = createImageView(depthBufferImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

/**
 * @brief Creates framebuffers for each swap chain image.
 *
 * Framebuffers bind attachments (color, depth) to the render pass for rendering.
 * Each swap chain image has an associated framebuffer.
 *
 * @throws std::runtime_error if framebuffer creation fails.
 */
void VulkanRenderer::createFramebuffers()
{
	// Resize framebuffer list to match swap chain image count
	swapChainFramebuffers.resize(swapChainImages.size());

	// Create a framebuffer for each swap chain image
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
	{
		// Attach color and depth images to the framebuffer
		std::array<VkImageView, 2> attachments = {
			swapChainImages[i].imageView,  // Color attachment
			depthBufferImageView           // Depth attachment
		};

		// Configure framebuffer creation
		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;  // Render pass compatibility
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data(); // Color & depth attachments
		framebufferCreateInfo.width = swapChainExtent.width;
		framebufferCreateInfo.height = swapChainExtent.height;
		framebufferCreateInfo.layers = 1; // Single-layered framebuffer

		// Create the framebuffer
		VkResult result = vkCreateFramebuffer(mainDevice.logicalDevice, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a Framebuffer!");
		}
	}
}


void VulkanRenderer::createCommandPool()
{
	// Get indices of queue families from device
	QueueFamilyIndices queueFamilyIndices = getQueueFamilies(mainDevice.physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;	// Queue Family type that buffers from this command pool will use

	// Create a Graphics Queue Family Command Pool
	VkResult result = vkCreateCommandPool(mainDevice.logicalDevice, &poolInfo, nullptr, &graphicsCommandPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Command Pool!");
	}
}

void VulkanRenderer::createCommandBuffers()
{
	// Resize command buffer count to have one for each framebuffer
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = graphicsCommandPool;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// VK_COMMAND_BUFFER_LEVEL_PRIMARY	: Buffer you submit directly to queue. Cant be called by other buffers.
	// VK_COMMAND_BUFFER_LEVEL_SECONARY	: Buffer can't be called directly. Can be called from other buffers via "vkCmdExecuteCommands" when recording commands in primary buffer
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	// Allocate command buffers and place handles in array of buffers
	VkResult result = vkAllocateCommandBuffers(mainDevice.logicalDevice, &cbAllocInfo, commandBuffers.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Command Buffers!");
	}
}

void VulkanRenderer::createSynchronisation()
{
	imageAvailable.resize(MAX_FRAME_DRAWS);
	renderFinished.resize(MAX_FRAME_DRAWS);
	drawFences.resize(MAX_FRAME_DRAWS);

	// Semaphore creation information
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Fence creation information
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		if (vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailable[i]) != VK_SUCCESS ||
			vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinished[i]) != VK_SUCCESS ||
			vkCreateFence(mainDevice.logicalDevice, &fenceCreateInfo, nullptr, &drawFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a Semaphore and/or Fence!");
		}
	}
}

void VulkanRenderer::createTextureSampler()
{
	// Sampler Creation Info
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;						// How to render when image is magnified on screen
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;						// How to render when image is minified on screen
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle texture wrap in U (x) direction
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle texture wrap in V (y) direction
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle texture wrap in W (z) direction
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;	// Border beyond texture (only workds for border clamp)
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;				// Whether coords should be normalized (between 0 and 1)
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;		// Mipmap interpolation mode
	samplerCreateInfo.mipLodBias = 0.0f;								// Level of Details bias for mip level
	samplerCreateInfo.minLod = 0.0f;									// Minimum Level of Detail to pick mip level
	samplerCreateInfo.maxLod = 0.0f;									// Maximum Level of Detail to pick mip level
	samplerCreateInfo.anisotropyEnable = VK_TRUE;						// Enable Anisotropy
	samplerCreateInfo.maxAnisotropy = 16;								// Anisotropy sample level

	VkResult result = vkCreateSampler(mainDevice.logicalDevice, &samplerCreateInfo, nullptr, &textureSampler);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Filed to create a Texture Sampler!");
	}
}
void VulkanRenderer::createUniformBuffers()
{
	// ViewProjection buffer m�rete
	VkDeviceSize vpBufferSize = sizeof(UboViewProjection);

	// Ambiant lighting buffer m�rete
	VkDeviceSize lightingBufferSize = sizeof(UboLighting);

	// Egy uniform buffer minden egyes swap chain k�phez (�s ezzel egy�tt command bufferhez)
	vpUniformBuffer.resize(swapChainImages.size());
	vpUniformBufferMemory.resize(swapChainImages.size());

	lightingUniformBuffer.resize(swapChainImages.size());
	lightingUniformBufferMemory.resize(swapChainImages.size());

	// Uniform bufferek l�trehoz�sa
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		// ViewProjection buffer
		createBuffer(
			mainDevice.physicalDevice,
			mainDevice.logicalDevice,
			vpBufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&vpUniformBuffer[i],
			&vpUniformBufferMemory[i]
		);

		// Ambient lighting buffer
		createBuffer(
			mainDevice.physicalDevice,
			mainDevice.logicalDevice,
			lightingBufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&lightingUniformBuffer[i],
			&lightingUniformBufferMemory[i]
		);

	}
}

void VulkanRenderer::createDescriptorPool()
{
	// CREATE UNIFORM DESCRIPTOR POOL
	// Type of descriptors + how many DESCRIPTORS, not Descriptor Sets (combined makes the pool size)
	// ViewProjection Pool
	VkDescriptorPoolSize vpPoolSize = {};
	vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vpPoolSize.descriptorCount = static_cast<uint32_t>(vpUniformBuffer.size());

	// light pool
	VkDescriptorPoolSize vpPoolSizeLight = {};
	vpPoolSizeLight.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vpPoolSizeLight.descriptorCount = static_cast<uint32_t>(lightingUniformBuffer.size());

	// Model Pool (DYNAMIC)
	/*VkDescriptorPoolSize modelPoolSize = {};
	modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelPoolSize.descriptorCount = static_cast<uint32_t>(modelDUniformBuffer.size());*/

	// List of pool sizes
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes = { vpPoolSize, vpPoolSizeLight };

	// Data to create Descriptor Pool
	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());					// Maximum number of Descriptor Sets that can be created from pool
	poolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());		// Amount of Pool Sizes being passed
	poolCreateInfo.pPoolSizes = descriptorPoolSizes.data();									// Pool Sizes to create pool with

	// Create Descriptor Pool
	VkResult result = vkCreateDescriptorPool(mainDevice.logicalDevice, &poolCreateInfo, nullptr, &descriptorPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Descriptor Pool!");
	}

	// CREATE SAMPLER DESCRIPTOR POOL
	// Texture sampler pool
	VkDescriptorPoolSize samplerPoolSize = {};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = MAX_OBJECTS;

	VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
	samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
	samplerPoolCreateInfo.poolSizeCount = 1;
	samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;

	result = vkCreateDescriptorPool(mainDevice.logicalDevice, &samplerPoolCreateInfo, nullptr, &samplerDescriptorPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Descriptor Pool!");
	}
}

void VulkanRenderer::createDescriptorSets()
{
	// Resize Descriptor Set list so one for every buffer
	descriptorSets.resize(swapChainImages.size());

	std::vector<VkDescriptorSetLayout> setLayouts(swapChainImages.size(), descriptorSetLayout);

	// Descriptor Set Allocation Info
	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = descriptorPool;									// Pool to allocate Descriptor Set from
	setAllocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());// Number of sets to allocate
	setAllocInfo.pSetLayouts = setLayouts.data();									// Layouts to use to allocate sets (1:1 relationship)

	// Allocate descriptor sets (multiple)
	VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &setAllocInfo, descriptorSets.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Descriptor Sets!");
	}

	// Update all of descriptor set buffer bindings
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		// VIEW PROJECTION DESCRIPTOR
		// Buffer info and data offset info
		VkDescriptorBufferInfo vpBufferInfo = {};
		vpBufferInfo.buffer = vpUniformBuffer[i];		// Buffer to get data from
		vpBufferInfo.offset = 0;						// Position of start of data
		vpBufferInfo.range = sizeof(UboViewProjection);				// Size of data

		// light description
		VkDescriptorBufferInfo lightBufferInfo = {};
		lightBufferInfo.buffer = lightingUniformBuffer[i];
		lightBufferInfo.offset = 0;
		lightBufferInfo.range = sizeof(UboLighting);

		// Data about connection between binding and buffer
		VkWriteDescriptorSet vpSetWrite = {};
		vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vpSetWrite.dstSet = descriptorSets[i];								// Descriptor Set to update
		vpSetWrite.dstBinding = 0;											// Binding to update (matches with binding on layout/shader)
		vpSetWrite.dstArrayElement = 0;									// Index in array to update
		vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;		// Type of descriptor
		vpSetWrite.descriptorCount = 1;									// Amount to update
		vpSetWrite.pBufferInfo = &vpBufferInfo;							// Information about buffer data to bind

		VkWriteDescriptorSet lightSetWrite = {};
		lightSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		lightSetWrite.dstSet = descriptorSets[i];
		lightSetWrite.dstBinding = 1;
		lightSetWrite.dstArrayElement = 0;
		lightSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightSetWrite.descriptorCount = 1;
		lightSetWrite.pBufferInfo = &lightBufferInfo;


		// List of Descriptor Set Writes
		std::vector<VkWriteDescriptorSet> setWrites = { vpSetWrite, lightSetWrite };

		// Update the descriptor sets with new buffer/binding info
		vkUpdateDescriptorSets(mainDevice.logicalDevice, static_cast<uint32_t>(setWrites.size()), setWrites.data(),
			0, nullptr);
	}
}

void VulkanRenderer::updateUniformBuffers(uint32_t imageIndex)
{
	// Friss�tj�k a ViewProjection adatokat
	void* data;
	vkMapMemory(mainDevice.logicalDevice, vpUniformBufferMemory[imageIndex], 0, sizeof(UboViewProjection), 0, &data);
	memcpy(data, &uboViewProjection, sizeof(UboViewProjection));
	vkUnmapMemory(mainDevice.logicalDevice, vpUniformBufferMemory[imageIndex]);

	// Friss�tj�k a Amb Lighting adatokat
	vkMapMemory(mainDevice.logicalDevice, lightingUniformBufferMemory[imageIndex], 0, sizeof(UboLighting), 0, &data);
	memcpy(data, &uboLighting, sizeof(UboLighting));
	vkUnmapMemory(mainDevice.logicalDevice, lightingUniformBufferMemory[imageIndex]);
}

void VulkanRenderer::recordCommands(uint32_t currentImage)
{
	// Information about how to begin each command buffer
	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;							// Render Pass to begin
	renderPassBeginInfo.renderArea.offset = { 0, 0 };						// Start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = swapChainExtent;				// Size of region to run render pass on (starting at offset)

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.5f, 0.5f, 0.5f, 1.0f };
	clearValues[1].depthStencil.depth = 1.0f;

	renderPassBeginInfo.pClearValues = clearValues.data();					// List of clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	renderPassBeginInfo.framebuffer = swapChainFramebuffers[currentImage];

	// Start recording commands to command buffer!
	VkResult result = vkBeginCommandBuffer(commandBuffers[currentImage], &bufferBeginInfo);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to start recording a Command Buffer!");
	}

	// Begin Render Pass
	vkCmdBeginRenderPass(commandBuffers[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind Pipeline to be used in render pass
	vkCmdBindPipeline(commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	for (size_t j = 0; j < modelList.size(); j++)
	{
		MeshModel thisModel = modelList[j];

		vkCmdPushConstants(
			commandBuffers[currentImage],
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,		// Stage to push constants to
			0,								// Offset of push constants to update
			sizeof(Model),					// Size of data being pushed
			thisModel.getModelRef());			// Actual data being pushed (can be array)

		for (size_t k = 0; k < thisModel.getMeshCount(); k++)
		{

			VkBuffer vertexBuffers[] = { thisModel.getMesh(k)->getVertexBuffer() };					// Buffers to bind
			VkDeviceSize offsets[] = { 0 };												// Offsets into buffers being bound
			vkCmdBindVertexBuffers(commandBuffers[currentImage], 0, 1, vertexBuffers, offsets);	// Command to bind vertex buffer before drawing with them

			// Bind mesh index buffer, with 0 offset and using the uint32 type
			vkCmdBindIndexBuffer(commandBuffers[currentImage], thisModel.getMesh(k)->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			// Dynamic Offset Amount
			// uint32_t dynamicOffset = static_cast<uint32_t>(modelUniformAlignment) * j;

			// "Push" constants to given shader stage directly (no buffer)


			std::array<VkDescriptorSet, 2> descriptorSetGroup = { descriptorSets[currentImage],
				samplerDescriptorSets[thisModel.getMesh(k)->getTexId()] };

			// Bind Descriptor Sets
			vkCmdBindDescriptorSets(commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);

			// Execute pipeline
			vkCmdDrawIndexed(commandBuffers[currentImage], thisModel.getMesh(k)->getIndexCount(), 1, 0, 0, 0);
		}
	}

	// End Render Pass
	vkCmdEndRenderPass(commandBuffers[currentImage]);

	// Stop recording to command buffer
	result = vkEndCommandBuffer(commandBuffers[currentImage]);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to stop recording a Command Buffer!");
	}

}

void VulkanRenderer::getPhysicalDevice()
{
	// Enumerate Physical devices the vkInstance can access
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// If no devices available, then none support Vulkan!
	if (deviceCount == 0)
	{
		throw std::runtime_error("Can't find GPUs that support Vulkan Instance!");
	}

	// Get list of Physical Devices
	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

	for (const auto& device : deviceList)
	{
		if (checkDeviceSuitable(device))
		{
			mainDevice.physicalDevice = device;
			break;
		}
	}

	// Get properties of our new device
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(mainDevice.physicalDevice, &deviceProperties);

	//minUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;
}

void VulkanRenderer::allocateDynamicBufferTransferSpace()
{
	// Calculate alignment of model data
	/*modelUniformAlignment = (sizeof(UboModel) + minUniformBufferOffset - 1)
							& ~(minUniformBufferOffset - 1);

	// Create space in memory to hold dynamic buffer that is aligned to our required alignment and holds MAX_OBJECTS
	modelTransferSpace = (UboModel *)_aligned_malloc(modelUniformAlignment * MAX_OBJECTS, modelUniformAlignment);*/
}

bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
	// Need to get number of extensions to create array of correct size to hold extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	// Create a list of VkExtensionProperties using count
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	// Check if given extensions are in list of available extensions
	for (const auto& checkExtension : *checkExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(checkExtension, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}
	}

	return true;
}

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	// Get device extension count
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	// If no extensions found, return failure
	if (extensionCount == 0)
	{
		return false;
	}

	// Populate list of extensions
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

	// Check for extension
	for (const auto& deviceExtension : deviceExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}
	}

	return true;
}

bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice device)
{
	/*
	// Information about the device itself (ID, name, type, vendor, etc)
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	*/

	// Information about what the device can do (geo shader, tess shader, wide lines, etc)
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);


	QueueFamilyIndices indices = getQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainValid = false;
	if (extensionsSupported)
	{
		SwapChainDetails swapChainDetails = getSwapChainDetails(device);
		swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
	}

	return indices.isValid() && extensionsSupported && swapChainValid && deviceFeatures.samplerAnisotropy;
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	// Get all Queue Family Property info for the given device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

	// Go through each queue family and check if it has at least 1 of the required types of queue
	int i = 0;
	for (const auto& queueFamily : queueFamilyList)
	{
		// First check if queue family has at least 1 queue in that family (could have no queues)
		// Queue can be multiple types defined through bitfield. Need to bitwise AND with VK_QUEUE_*_BIT to check if has required type
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;		// If queue family is valid, then get index
		}

		// Check if Queue Family supports presentation
		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
		// Check if queue is presentation type (can be both graphics and presentation)
		if (queueFamily.queueCount > 0 && presentationSupport)
		{
			indices.presentationFamily = i;
		}

		// Check if queue family indices are in a valid state, stop searching if so
		if (indices.isValid())
		{
			break;
		}

		i++;
	}

	return indices;
}

SwapChainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device)
{
	SwapChainDetails swapChainDetails;

	// -- CAPABILITIES --
	// Get the surface capabilities for the given surface on the given physical device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.surfaceCapabilities);

	// -- FORMATS --
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	// If formats returned, get list of formats
	if (formatCount != 0)
	{
		swapChainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainDetails.formats.data());
	}

	// -- PRESENTATION MODES --
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);

	// If presentation modes returned, get list of presentation modes
	if (presentationCount != 0)
	{
		swapChainDetails.presentationModes.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, swapChainDetails.presentationModes.data());
	}

	return swapChainDetails;
}

// Best format is subjective, but ours will be:
// format		:	VK_FORMAT_R8G8B8A8_UNORM (VK_FORMAT_B8G8R8A8_UNORM as backup)
// colorSpace	:	VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	// If only 1 format available and is undefined, then this means ALL formats are available (no restrictions)
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	// If restricted, search for optimal format
	for (const auto& format : formats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
			&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	// If can't find optimal format, then just return first format
	return formats[0];
}

VkPresentModeKHR VulkanRenderer::chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes)
{
	// Look for Mailbox presentation mode
	for (const auto& presentationMode : presentationModes)
	{
		if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return presentationMode;
		}
	}

	// If can't find, use FIFO as Vulkan spec says it must be present
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	// If current extent is at numeric limits, then extent can vary. Otherwise, it is the size of the window.
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		// If value can vary, need to set manually

		// Get window size
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		// Create new extent using window size
		VkExtent2D newExtent = {};
		newExtent.width = static_cast<uint32_t>(width);
		newExtent.height = static_cast<uint32_t>(height);

		// Surface also defines max and min, so make sure within boundaries by clamping value
		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

		return newExtent;
	}
}

VkFormat VulkanRenderer::chooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
{
	// Loop through options and find compatible one
	for (VkFormat format : formats)
	{
		// Get properties for give format on this device
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(mainDevice.physicalDevice, format, &properties);

		// Depending on tiling choice, need to check for different bit flag
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find a matching format!");
}

VkImage VulkanRenderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory)
{
	// CREATE IMAGE
	// Image Creation Info
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;						// Type of image (1D, 2D, or 3D)
	imageCreateInfo.extent.width = width;								// Width of image extent
	imageCreateInfo.extent.height = height;								// Height of image extent
	imageCreateInfo.extent.depth = 1;									// Depth of image (just 1, no 3D aspect)
	imageCreateInfo.mipLevels = 1;										// Number of mipmap levels
	imageCreateInfo.arrayLayers = 1;									// Number of levels in image array
	imageCreateInfo.format = format;									// Format type of image
	imageCreateInfo.tiling = tiling;									// How image data should be "tiled" (arranged for optimal reading)
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// Layout of image data on creation
	imageCreateInfo.usage = useFlags;									// Bit flags defining what image will be used for
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;					// Number of samples for multi-sampling
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;			// Whether image can be shared between queues

	// Create image
	VkImage image;
	VkResult result = vkCreateImage(mainDevice.logicalDevice, &imageCreateInfo, nullptr, &image);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image!");
	}

	// CREATE MEMORY FOR IMAGE

	// Get memory requirements for a type of image
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(mainDevice.logicalDevice, image, &memoryRequirements);

	// Allocate memory using image requirements and user defined properties
	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memoryRequirements.size;
	memoryAllocInfo.memoryTypeIndex = findMemoryTypeIndex(mainDevice.physicalDevice, memoryRequirements.memoryTypeBits, propFlags);

	result = vkAllocateMemory(mainDevice.logicalDevice, &memoryAllocInfo, nullptr, imageMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate memory for image!");
	}

	// Connect memory to image
	vkBindImageMemory(mainDevice.logicalDevice, image, *imageMemory, 0);

	return image;
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;											// Image to create view for
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;						// Type of image (1D, 2D, 3D, Cube, etc)
	viewCreateInfo.format = format;											// Format of image data
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;			// Allows remapping of rgba components to other rgba values
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// Subresources allow the view to view only a part of an image
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;				// Which aspect of image to view (e.g. COLOR_BIT for viewing colour)
	viewCreateInfo.subresourceRange.baseMipLevel = 0;						// Start mipmap level to view from
	viewCreateInfo.subresourceRange.levelCount = 1;							// Number of mipmap levels to view
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;						// Start array level to view from
	viewCreateInfo.subresourceRange.layerCount = 1;							// Number of array levels to view

	// Create image view and return it
	VkImageView imageView;
	VkResult result = vkCreateImageView(mainDevice.logicalDevice, &viewCreateInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image View!");
	}

	return imageView;
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code)
{
	// Shader Module creation information
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();										// Size of code
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());		// Pointer to code (of uint32_t pointer type)

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(mainDevice.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a shader module!");
	}

	return shaderModule;
}

int VulkanRenderer::createTextureImage(std::string fileName)
{
	// Load image file
	int width, height;
	VkDeviceSize imageSize;
	stbi_uc* imageData = loadTextureFile(fileName, &width, &height, &imageSize);

	// Create staging buffer to hold loaded data, ready to copy to device
	VkBuffer imageStagingBuffer;
	VkDeviceMemory imageStagingBufferMemory;
	createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&imageStagingBuffer, &imageStagingBufferMemory);

	// Copy image data to staging buffer
	void* data;
	vkMapMemory(mainDevice.logicalDevice, imageStagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(imageSize));
	vkUnmapMemory(mainDevice.logicalDevice, imageStagingBufferMemory);

	// Free original image data
	stbi_image_free(imageData);

	// Create image to hold final texture
	VkImage texImage;
	VkDeviceMemory texImageMemory;
	texImage = createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);


	// COPY DATA TO IMAGE
	// Transition image to be DST for copy operation
	transitionImageLayout(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool,
		texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Copy image data
	copyImageBuffer(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, imageStagingBuffer, texImage, width, height);

	// Transition image to be shader readable for shader usage
	transitionImageLayout(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool,
		texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Add texture data to vector for reference
	textureImages.push_back(texImage);
	textureImageMemory.push_back(texImageMemory);

	// Destroy staging buffers
	vkDestroyBuffer(mainDevice.logicalDevice, imageStagingBuffer, nullptr);
	vkFreeMemory(mainDevice.logicalDevice, imageStagingBufferMemory, nullptr);

	// Return index of new texture image
	return textureImages.size() - 1;
}

int VulkanRenderer::createTexture(std::string fileName)
{
	// Create Texture Image and get its location in array
	int textureImageLoc = createTextureImage(fileName);

	// Create Image View and add to list
	VkImageView imageView = createImageView(textureImages[textureImageLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	textureImageViews.push_back(imageView);

	// Create Texture Descriptor
	int descriptorLoc = createTextureDescriptor(imageView);

	// Return location of set with texture
	return descriptorLoc;
}

int VulkanRenderer::createTextureDescriptor(VkImageView textureImage)
{
	VkDescriptorSet descriptorSet;

	// Descriptor Set Allocation Info
	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = samplerDescriptorPool;
	setAllocInfo.descriptorSetCount = 1;
	setAllocInfo.pSetLayouts = &samplerSetLayout;

	// Allocate Descriptor Sets
	VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &setAllocInfo, &descriptorSet);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Texture Descriptor Sets!");
	}

	// Texture Image Info
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// Image layout when in use
	imageInfo.imageView = textureImage;									// Image to bind to set
	imageInfo.sampler = textureSampler;									// Sampler to use for set

	// Descriptor Write Info
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	// Update new descriptor set
	vkUpdateDescriptorSets(mainDevice.logicalDevice, 1, &descriptorWrite, 0, nullptr);

	// Add descriptor set to list
	samplerDescriptorSets.push_back(descriptorSet);

	// Return descriptor set location
	return samplerDescriptorSets.size() - 1;
}

int VulkanRenderer::createMeshModel(std::string modelFile, bool controlable, glm::vec3 startPos, bool isLookingAt, glm::vec3 lookAt)
{
	// Import model "scene"
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelFile, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	if (!scene)
	{
		throw std::runtime_error("Failed to load model! (" + modelFile + ")");
	}

	// Get vector of all materials with 1:1 ID placement
	std::vector<std::string> textureNames = MeshModel::LoadMaterials(scene);

	// Conversion from the materials list IDs to our Descriptor Array IDs
	std::vector<int> matToTex(textureNames.size());

	// Loop over textureNames and create textures for them
	for (size_t i = 0; i < textureNames.size(); i++)
	{
		// If material had no texture, set '0' to indicate no texture, texture 0 will be reserved for a default texture
		if (textureNames[i].empty())
		{
			matToTex[i] = 0;
		}
		else
		{
			// Otherwise, create texture and set value to index of new texture
			matToTex[i] = createTexture(textureNames[i]);
		}
	}

	// Load in all our meshes
	std::vector<Mesh> modelMeshes = MeshModel::LoadNode(mainDevice.physicalDevice, mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool,
		scene->mRootNode, scene, matToTex);

	// Create mesh model and add to list
	MeshModel meshModel;

	if (isLookingAt) { meshModel = MeshModel(modelMeshes, controlable, startPos, lookAt); }
	else { meshModel = MeshModel(modelMeshes, controlable, startPos); }
	
	modelList.push_back(meshModel);

	return modelList.size() - 1;
}

stbi_uc* VulkanRenderer::loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize)
{
	// Number of channels image uses
	int channels;

	// Load pixel data for image
	std::string fileLoc = "Textures/" + fileName;
	stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

	if (!image)
	{
		throw std::runtime_error("Failed to load a Texture file! (" + fileName + ")");
	}

	// Calculate image size using given and known data
	*imageSize = *width * *height * 4;

	return image;
}

MeshModel* VulkanRenderer::getMeshModel(int meshId)
{
	return &modelList[meshId];
}
