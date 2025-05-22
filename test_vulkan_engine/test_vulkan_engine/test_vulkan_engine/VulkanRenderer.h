#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <array>

#include "stb_image.h"

#include "Mesh.h"
#include "MeshModel.h"
#include "Utilities.h"
#include "Camera.h"
#include <iostream>


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

/**
 * @class VulkanRenderer
 * @brief Handles Vulkan rendering operations, including validation layers, debug messages, and required extensions.
 *
 * This class is responsible for setting up Vulkan validation layers, retrieving required extensions,
 * and handling debug messages through a debug messenger.
 */
class VulkanRenderer
{
public:
	/**
	 * @brief List of validation layers to enable.
	 *
	 * This vector contains the names of Vulkan validation layers that are used for debugging.
	 * Validation layers help catch errors and provide useful debugging information.
	 */
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	/**
	 * @brief Vulkan debug messenger handle.
	 *
	 * This handle stores the Vulkan debug messenger instance used for validation layer debugging.
	 */
	VkDebugUtilsMessengerEXT debugMessenger;

	/**
	 * @brief Checks if the requested Vulkan validation layers are available.
	 *
	 * This function retrieves the list of available instance layers and verifies
	 * that all requested validation layers are supported.
	 *
	 * @return True if all requested validation layers are supported, otherwise false.
	 */
	bool checkValidationLayerSupport();

	/**
	 * @brief Retrieves the required Vulkan instance extensions.
	 *
	 * This function queries GLFW for necessary Vulkan extensions and adds
	 * the debug utilities extension if validation layers are enabled.
	 *
	 * @return A vector containing the required Vulkan instance extensions.
	 */
	std::vector<const char*> getRequiredExtensions();

	/**
	 * @brief Callback function for Vulkan validation layer messages.
	 *
	 * This function is registered as a debug callback for Vulkan validation layers.
	 * It prints messages about validation errors, warnings, and performance issues.
	 *
	 * @param messageSeverity The severity of the message (error, warning, info, verbose).
	 * @param messageType The type of message (general, validation, performance).
	 * @param pCallbackData Contains the debug message details.
	 * @param pUserData User-defined data (not used in this implementation).
	 * @return Always returns VK_FALSE.
	 */
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	/**
	 * @brief Fills the debug messenger creation info structure.
	 *
	 * Populates a `VkDebugUtilsMessengerCreateInfoEXT` structure with the
	 * required settings to enable Vulkan validation layer debugging.
	 *
	 * @param createInfo A reference to a `VkDebugUtilsMessengerCreateInfoEXT` structure.
	 */
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	/**
	 * @brief Sets up the Vulkan debug messenger.
	 *
	 * This function creates a debug messenger instance that captures validation
	 * layer messages and routes them to the `debugCallback` function.
	 */
	void setupDebugMessenger();

	/**
	 * @brief Creates a Vulkan debug messenger for validation layer debugging.
	 *
	 * This function dynamically loads the `vkCreateDebugUtilsMessengerEXT` function
	 * from the Vulkan instance and uses it to create a debug messenger.
	 * If the function is not found, it returns `VK_ERROR_EXTENSION_NOT_PRESENT`.
	 *
	 * @param instance The Vulkan instance.
	 * @param pCreateInfo Pointer to the debug messenger creation info structure.
	 * @param pAllocator Optional custom memory allocator (can be nullptr).
	 * @param pDebugMessenger Output parameter for the created debug messenger.
	 * @return VK_SUCCESS if creation was successful, otherwise an error code.
	 */
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);

	/**
	 * @brief Destroys the Vulkan debug messenger.
	 *
	 * This function dynamically loads the `vkDestroyDebugUtilsMessengerEXT` function
	 * from the Vulkan instance and properly destroys the debug messenger.
	 *
	 * @param instance The Vulkan instance.
	 * @param debugMessenger The debug messenger handle to be destroyed.
	 * @param pAllocator Optional memory allocator (can be nullptr).
	 */
	void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);


	/**
	 * @brief Sets the lighting for the scene based on a model's position.
	 *
	 * This function updates the lighting parameters using the position of a specified
	 * model as the light source. It configures the light properties such as direction,
	 * intensity, and cutoff angles.
	 *
	 * @param sourceModelId The ID of the model that acts as the light source.
	 */
	void setLighting(int sourceModelId);






	/**
	 * @brief Initializes the Vulkan renderer.
	 *
	 * This function sets up the Vulkan instance, swap chain, pipeline, buffers, and shaders.
	 * It also associates the renderer with a GLFW window and a camera for rendering.
	 *
	 * @param newWindow Pointer to the GLFW window that will be used for rendering.
	 * @param camera Pointer to the Camera object that defines the view matrix.
	 * @return Returns 0 on success, or an error code if initialization fails.
	 */
	int init(GLFWwindow* newWindow, Camera* camera);

	/**
	 * @brief Loads a 3D model and adds it to the scene.
	 *
	 * This function loads a mesh model from a file, assigns it an initial position,
	 * and optionally makes it controllable or oriented towards a target position.
	 *
	 * @param modelFile Path to the 3D model file.
	 * @param controlable Specifies whether the model can be moved by the user.
	 * @param startPos The initial position of the model in world space.
	 * @param isLookingAt If true, the model is rotated to face a target.
	 * @param lookAt The position the model should face (default is (1,0,0)).
	 * @return The ID assigned to the created model.
	 */
	int createMeshModel(std::string modelFile, bool controlable, glm::vec3 startPos, bool isLookingAt, glm::vec3 lookAt = glm::vec3(1.0f, 0.0f, 0.0f));

	/**
	 * @brief Updates the transformation matrix of an existing model.
	 *
	 * This function modifies the model matrix of a specific mesh, allowing transformations
	 * such as translation, rotation, and scaling.
	 *
	 * @param modelId The ID of the model to be updated.
	 * @param newModel The new transformation matrix for the model.
	 */
	void updateModel(int modelId, glm::mat4 newModel);

	/**
	 * @brief Updates the view matrix based on the current camera position and orientation.
	 *
	 * This function recalculates the camera's view matrix and updates the shader's uniform buffers
	 * to reflect the new perspective.
	 */
	void updateView();

	/**
	 * @brief Renders the scene using Vulkan.
	 *
	 * This function records Vulkan commands, submits them to the GPU, and presents
	 * the final rendered image to the screen.
	 */
	void draw();

	/**
	 * @brief Cleans up Vulkan resources and deallocates memory.
	 *
	 * This function destroys Vulkan objects, frees GPU memory, and shuts down
	 * the rendering pipeline to ensure proper cleanup.
	 */
	void cleanup();

	// get
	MeshModel* getMeshModel(int meshId);

	~VulkanRenderer();


private:
	/**
 * @struct Spotlight
 * @brief Defines a spotlight for dynamic lighting in the scene.
 *
 * This structure contains parameters for a spotlight, including its position,
 * direction, color, and intensity. The cutoff angles control the beam spread.
 */
	struct Spotlight {
		glm::vec3 lightDirection; ///< The direction in which the spotlight points.
		float diffuseStr;         ///< Intensity of the diffuse light component.

		glm::vec3 lightColor;     ///< The color of the spotlight.
		float specularStr;        ///< Intensity of the specular reflection.

		float shininess;          ///< Shininess factor for specular highlights.

		float innerCutOff;        ///< Inner cutoff angle (in radians) for the spotlight.
		float outerCutOff;        ///< Outer cutoff angle (in radians) for the spotlight.

		glm::vec4 lightPosition;  ///< The position of the spotlight in world space.
		float padding;            ///< Unused padding to align the structure in memory.
	};


	/**
	 * @struct UboLighting
	 * @brief Represents the lighting data to be sent to the GPU.
	 *
	 * This structure contains ambient lighting properties and an array of spotlights.
	 * It is used to store lighting data in a Vulkan uniform buffer.
	 */
	struct UboLighting {
		glm::vec3 ambiantLightColor;  ///< The color of the ambient light.
		float ambiantStr;             ///< The intensity of the ambient light.

		Spotlight spotlight[1];       ///< An array containing spotlight data.
	} uboLighting;


	/**
	 * @brief Vulkan buffers for storing lighting uniform data.
	 *
	 * These buffers hold lighting data that is transferred to the GPU for rendering.
	 */
	std::vector<VkBuffer> lightingUniformBuffer;

	/**
	 * @brief Vulkan device memory associated with the lighting uniform buffers.
	 *
	 * This memory stores the lighting UBO, which is accessed by the shaders.
	 */
	std::vector<VkDeviceMemory> lightingUniformBufferMemory;


	/**
	 * @brief Pointer to the GLFW window used for rendering.
	 *
	 * This window is created using GLFW and serves as the target for Vulkan rendering.
	 */
	GLFWwindow* window;

	/**
	 * @brief Pointer to the Camera object.
	 *
	 * The camera is responsible for defining the view matrix used in rendering.
	 * It is updated based on user input to control the scene perspective.
	 */
	Camera* camera;


	/**
	 * @brief Tracks the current frame index in the rendering loop.
	 *
	 * Used for synchronizing resources in Vulkan's multi-buffered rendering system.
	 */
	int currentFrame = 0;


	/**
	 * @brief List of all 3D models present in the scene.
	 *
	 * Each model is represented by a MeshModel object, which contains
	 * its mesh data, transformations, and material properties.
	 */
	std::vector<MeshModel> modelList;

	/**
	 * @struct UboViewProjection
	 * @brief Stores the view and projection matrices for rendering.
	 *
	 * These matrices are sent to the GPU via uniform buffers to transform
	 * world coordinates into camera space and apply perspective projection.
	 */
	struct UboViewProjection {
		glm::mat4 projection; ///< Projection matrix (perspective or orthographic).
		glm::mat4 view;       ///< View matrix (camera transformation).
	} uboViewProjection;


	/**
	 * @brief Vulkan instance handle.
	 *
	 * The Vulkan instance is the top-level object that initializes the Vulkan API.
	 */
	VkInstance instance;

	/**
	 * @brief Vulkan debug report callback.
	 *
	 * Used for capturing validation layer messages related to Vulkan API usage.
	 */
	VkDebugReportCallbackEXT callback;

	/**
	 * @struct mainDevice
	 * @brief Stores Vulkan's physical and logical device handles.
	 */
	struct MainDevice{
		VkPhysicalDevice physicalDevice; ///< The selected physical GPU.
		VkDevice logicalDevice;          ///< The Vulkan logical device.
	} mainDevice;

	/**
	 * @brief Vulkan queue for submitting graphics commands.
	 *
	 * Used to execute rendering operations on the GPU.
	 */
	VkQueue graphicsQueue;

	/**
	 * @brief Vulkan queue for presenting images to the screen.
	 *
	 * This queue handles presenting the rendered frame to the swapchain.
	 */
	VkQueue presentationQueue;

	/**
	 * @brief Vulkan rendering surface.
	 *
	 * The surface represents the platform-specific window where Vulkan will render frames.
	 */
	VkSurfaceKHR surface;

	/**
	 * @brief Vulkan swapchain for handling frame buffers.
	 *
	 * The swapchain manages a set of images used for double or triple buffering.
	 */
	VkSwapchainKHR swapchain;

	/**
	 * @brief List of images in the Vulkan swapchain.
	 *
	 * Each swapchain image represents a frame buffer that will be presented to the screen.
	 * Vulkan uses multiple images in a swapchain to enable double or triple buffering.
	 */
	std::vector<SwapchainImage> swapChainImages;

	/**
	 * @brief List of Vulkan framebuffers for each swapchain image.
	 *
	 * Framebuffers store color, depth, and stencil attachments for rendering.
	 */
	std::vector<VkFramebuffer> swapChainFramebuffers;

	/**
	 * @brief Command buffers for recording Vulkan draw commands.
	 *
	 * These command buffers contain rendering instructions that are submitted to the GPU.
	 */
	std::vector<VkCommandBuffer> commandBuffers;


	/**
	 * @brief Depth buffer image for handling depth testing.
	 *
	 * The depth buffer is used to determine which objects are in front of others.
	 */
	VkImage depthBufferImage;

	/**
	 * @brief Memory allocation for the depth buffer image.
	 *
	 * Stores depth values in GPU memory.
	 */
	VkDeviceMemory depthBufferImageMemory;

	/**
	 * @brief Image view for the depth buffer.
	 *
	 * Used to bind the depth buffer as an attachment in the Vulkan pipeline.
	 */
	VkImageView depthBufferImageView;

	/**
	 * @brief Vulkan texture sampler for filtering and mipmapping.
	 *
	 * The sampler is used to sample textures with filtering and mipmap support.
	 */
	VkSampler textureSampler;

	/**
	 * @brief Descriptor set layout for uniform buffers.
	 *
	 * Defines how uniform buffers (e.g., transformation matrices) are structured in shaders.
	 */
	VkDescriptorSetLayout descriptorSetLayout;

	/**
	 * @brief Descriptor set layout for texture samplers.
	 *
	 * Defines how textures and image samplers are bound in shaders.
	 */
	VkDescriptorSetLayout samplerSetLayout;

	/**
	 * @brief Push constant range for passing small amounts of data to shaders.
	 *
	 * Push constants allow passing transformation data or small parameters
	 * to shaders efficiently without using descriptor sets.
	 */
	VkPushConstantRange pushConstantRange;

	/**
	 * @brief Vulkan descriptor pool for allocating descriptor sets.
	 *
	 * This pool is used to allocate descriptor sets for uniform buffers,
	 * allowing shaders to access transformation matrices and other uniform data.
	 */
	VkDescriptorPool descriptorPool;

	/**
	 * @brief Vulkan descriptor pool for texture samplers.
	 *
	 * This pool is used for allocating descriptor sets related to image samplers.
	 * It enables binding and using textures in shaders.
	 */
	VkDescriptorPool samplerDescriptorPool;

	/**
	 * @brief Descriptor sets for uniform buffers.
	 *
	 * Each descriptor set contains information about a uniform buffer,
	 * allowing shaders to read transformation matrices and other per-frame data.
	 */
	std::vector<VkDescriptorSet> descriptorSets;

	/**
	 * @brief Descriptor sets for texture samplers.
	 *
	 * These descriptor sets are used to bind texture samplers in shaders.
	 */
	std::vector<VkDescriptorSet> samplerDescriptorSets;

	/**
	 * @brief Uniform buffers for storing view and projection matrices.
	 *
	 * These buffers hold the camera's view matrix and projection matrix,
	 * which are updated every frame and sent to shaders.
	 */
	std::vector<VkBuffer> vpUniformBuffer;

	/**
	 * @brief Device memory for view-projection uniform buffers.
	 *
	 * This memory stores transformation matrices in GPU-accessible memory.
	 */
	std::vector<VkDeviceMemory> vpUniformBufferMemory;

	/**
	 * @brief Uniform buffers for storing per-model transformation data.
	 *
	 * These buffers hold model-specific transformation matrices,
	 * allowing each object to have independent movement and rotation.
	 */
	std::vector<VkBuffer> modelDUniformBuffer;

	/**
	 * @brief Device memory for model uniform buffers.
	 *
	 * This memory stores per-model transformations in GPU-accessible memory.
	 */
	std::vector<VkDeviceMemory> modelDUniformBufferMemory;

	// - Assets
	/**
	 * @brief Vulkan images used as textures in the scene.
	 *
	 * These images store texture data that will be applied to models.
	 */
	std::vector<VkImage> textureImages;

	/**
	 * @brief Device memory associated with texture images.
	 *
	 * Each texture image requires dedicated GPU memory to store its pixel data.
	 */
	std::vector<VkDeviceMemory> textureImageMemory;

	/**
	 * @brief Image views for texture images.
	 *
	 * Image views describe how textures are accessed in shaders.
	 */
	std::vector<VkImageView> textureImageViews;


	/**
	 * @brief Vulkan graphics pipeline.
	 *
	 * Defines the rendering pipeline stages, including shaders, vertex input,
	 * rasterization, and fragment processing.
	 */
	VkPipeline graphicsPipeline;

	/**
	 * @brief Layout for the graphics pipeline.
	 *
	 * Specifies how descriptors, push constants, and shader resources are structured.
	 */
	VkPipelineLayout pipelineLayout;

	/**
	 * @brief Vulkan render pass.
	 *
	 * Describes the framebuffer attachments and how they are processed during rendering.
	 */
	VkRenderPass renderPass;

	/**
	 * @brief Command pool for allocating Vulkan command buffers.
	 *
	 * The graphics command pool manages memory for Vulkan command buffers,
	 * which store GPU instructions for rendering.
	 */
	VkCommandPool graphicsCommandPool;

	// - Utility
	/**
	 * @brief Format of swapchain images.
	 *
	 * Defines the color format used for rendering frames.
	 */
	VkFormat swapChainImageFormat;

	/**
	 * @brief Dimensions of swapchain images.
	 *
	 * Defines the resolution of the rendering surface.
	 */
	VkExtent2D swapChainExtent;


	/**
	 * @brief Semaphores for synchronizing image availability.
	 *
	 * These semaphores ensure that the swapchain image is available before rendering begins.
	 */
	std::vector<VkSemaphore> imageAvailable;

	/**
	 * @brief Semaphores for synchronizing render completion.
	 *
	 * These semaphores ensure that rendering is finished before the image is presented to the screen.
	 */
	std::vector<VkSemaphore> renderFinished;

	/**
	 * @brief Fences for synchronizing frame drawing.
	 *
	 * Fences are used to ensure that command buffer execution is complete before reusing it.
	 */
	std::vector<VkFence> drawFences;


	// Vulkan Functions
	// - Create Functions
	/**
	 * @brief Creates the Vulkan instance.
	 *
	 * The Vulkan instance is the top-level object that initializes the Vulkan API
	 * and provides access to Vulkan functionality.
	 */
	void createInstance();

	/**
	 * @brief Creates the Vulkan logical device.
	 *
	 * The logical device provides an interface to interact with the physical GPU.
	 * It enables command execution, memory management, and rendering operations.
	 */
	void createLogicalDevice();

	/**
	 * @brief Creates a Vulkan rendering surface.
	 *
	 * The surface represents the platform-specific window where Vulkan will render frames.
	 */
	void createSurface();

	/**
	 * @brief Creates the Vulkan swapchain.
	 *
	 * The swapchain manages multiple frame buffers that enable double or triple buffering,
	 * allowing seamless frame rendering and presentation.
	 */
	void createSwapChain();

	/**
	 * @brief Creates the Vulkan render pass.
	 *
	 * The render pass defines how framebuffers and attachments (color, depth, stencil)
	 * are used during rendering.
	 */
	void createRenderPass();

	/**
	 * @brief Creates the descriptor set layout.
	 *
	 * This function defines how uniform buffers, textures, and other GPU resources
	 * are structured and accessed by shaders.
	 */
	void createDescriptorSetLayout();

	/**
	 * @brief Creates the Vulkan push constant range.
	 *
	 * Push constants allow small amounts of frequently changing data to be passed
	 * to shaders efficiently.
	 */
	void createPushConstantRange();

	/**
	 * @brief Creates the Vulkan graphics pipeline.
	 *
	 * The pipeline defines the sequence of operations needed for rendering,
	 * including shader stages, rasterization, and fragment processing.
	 */
	void createGraphicsPipeline();

	/**
	 * @brief Creates the Vulkan depth buffer image.
	 *
	 * The depth buffer is used for depth testing to ensure proper rendering order
	 * based on object distance from the camera.
	 */
	void createDepthBufferImage();

	/**
	 * @brief Creates the framebuffers for the swapchain images.
	 *
	 * Each framebuffer stores color and depth attachments for rendering.
	 */
	void createFramebuffers();

	/**
	 * @brief Creates the Vulkan command pool.
	 *
	 * The command pool manages memory for command buffers, which store rendering
	 * commands submitted to the GPU.
	 */
	void createCommandPool();

	/**
	 * @brief Creates the Vulkan command buffers.
	 *
	 * Command buffers store rendering commands that are executed by the GPU.
	 * These include draw calls, pipeline state changes, and memory operations.
	 */
	void createCommandBuffers();

	/**
	 * @brief Creates synchronization objects for frame rendering.
	 *
	 * Synchronization objects include semaphores and fences that ensure proper
	 * frame ordering and prevent rendering stalls.
	 */
	void createSynchronisation();

	/**
	 * @brief Creates a Vulkan texture sampler.
	 *
	 * Texture samplers determine how textures are filtered and accessed in shaders.
	 */
	void createTextureSampler();

	/**
	 * @brief Creates uniform buffers for storing transformation matrices.
	 *
	 * These buffers hold view, projection, and model matrices, which are updated
	 * every frame and sent to shaders.
	 */
	void createUniformBuffers();

	/**
	 * @brief Creates the Vulkan descriptor pool.
	 *
	 * The descriptor pool is used to allocate descriptor sets for uniform buffers,
	 * allowing shaders to access transformation data.
	 */
	void createDescriptorPool();

	/**
	 * @brief Creates the Vulkan descriptor sets.
	 *
	 * Descriptor sets define how uniform buffers and textures are bound to shaders.
	 */
	void createDescriptorSets();

	/**
	 * @brief Updates the uniform buffers for a given frame.
	 *
	 * This function updates the transformation matrices (view, projection, and model)
	 * for the specified swapchain image index.
	 *
	 * @param imageIndex The index of the current swapchain image being rendered.
	 */
	void updateUniformBuffers(uint32_t imageIndex);

	/**
	 * @brief Records Vulkan command buffers for rendering.
	 *
	 * This function encodes drawing commands, including pipeline state, vertex buffers,
	 * and draw calls, into a command buffer for execution.
	 *
	 * @param currentImage The index of the current swapchain image.
	 */
	void recordCommands(uint32_t currentImage);

	/**
	 * @brief Selects a suitable Vulkan physical device (GPU).
	 *
	 * This function searches for an available GPU that supports the required Vulkan features.
	 * It evaluates device extensions, queue families, and swapchain capabilities.
	 */
	void getPhysicalDevice();

	/**
	 * @brief Allocates memory for dynamic uniform buffer transfers.
	 *
	 * This function ensures efficient data transfer between CPU and GPU for dynamic uniform buffers.
	 */
	void allocateDynamicBufferTransferSpace();


	// - Support Functions
	// -- Checker Functions
	/**
	 * @brief Checks if the required Vulkan instance extensions are supported.
	 *
	 * This function verifies that all necessary instance extensions are available.
	 *
	 * @param checkExtensions A pointer to a vector containing the required extensions.
	 * @return True if all extensions are supported, false otherwise.
	 */
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);

	/**
	 * @brief Checks if a Vulkan physical device supports required extensions.
	 *
	 * This function verifies that a given physical device has the necessary extensions
	 * for swapchain support and other Vulkan features.
	 *
	 * @param device The Vulkan physical device to check.
	 * @return True if all required extensions are supported, false otherwise.
	 */
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	/**
	 * @brief Checks if a Vulkan physical device meets all rendering requirements.
	 *
	 * This function verifies that a GPU supports required queue families, extensions,
	 * and swapchain capabilities.
	 *
	 * @param device The Vulkan physical device to check.
	 * @return True if the device is suitable, false otherwise.
	 */
	bool checkDeviceSuitable(VkPhysicalDevice device);


	// -- Getter Functions
	/**
	 * @brief Retrieves queue family indices for a given Vulkan device.
	 *
	 * This function queries a physical device to determine which queue families support
	 * graphics and presentation operations.
	 *
	 * @param device The Vulkan physical device to query.
	 * @return QueueFamilyIndices containing the indices of the supported queues.
	 */
	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);

	/**
	 * @brief Retrieves details about a Vulkan device's swapchain support.
	 *
	 * This function queries a physical device to determine supported swapchain formats,
	 * present modes, and surface capabilities.
	 *
	 * @param device The Vulkan physical device to check.
	 * @return SwapChainDetails containing supported swapchain settings.
	 */
	SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);

	// -- Choose Functions
	/**
	 * @brief Selects the best surface format for the swapchain.
	 *
	 * This function iterates through the available surface formats and selects
	 * the most suitable format for rendering.
	 *
	 * @param formats A vector of available VkSurfaceFormatKHR formats.
	 * @return The selected VkSurfaceFormatKHR format.
	 */
	VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);

	/**
	 * @brief Selects the best presentation mode for the swapchain.
	 *
	 * This function determines the optimal presentation mode based on the available options.
	 *
	 * @param presentationModes A vector of available VkPresentModeKHR modes.
	 * @return The selected VkPresentModeKHR mode.
	 */
	VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes);

	/**
	 * @brief Selects the best swapchain extent (resolution).
	 *
	 * This function determines the ideal image resolution for the swapchain.
	 *
	 * @param surfaceCapabilities The Vulkan surface capabilities.
	 * @return The selected swapchain extent (resolution).
	 */
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

	/**
	 * @brief Selects a supported format for a Vulkan image or buffer.
	 *
	 * This function chooses a suitable format based on tiling and feature flags.
	 *
	 * @param formats A vector of candidate VkFormat options.
	 * @param tiling The required tiling mode (linear or optimal).
	 * @param featureFlags The required format features.
	 * @return The best matching VkFormat.
	 */
	VkFormat chooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

	// -- Create Functions
	/**
	 * @brief Creates a Vulkan image.
	 *
	 * This function creates a Vulkan image and allocates memory for it.
	 *
	 * @param width Image width.
	 * @param height Image height.
	 * @param format Image format.
	 * @param tiling Image tiling mode.
	 * @param useFlags Image usage flags (e.g., color attachment, depth buffer, etc.).
	 * @param propFlags Memory property flags.
	 * @param imageMemory Output parameter for allocated memory.
	 * @return The created Vulkan image.
	 */
	VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory);

	/**
	 * @brief Creates a Vulkan image view.
	 *
	 * Image views describe how an image is accessed (e.g., as a texture, depth buffer, etc.).
	 *
	 * @param image The Vulkan image.
	 * @param format The image format.
	 * @param aspectFlags Specifies the aspect of the image to view (color or depth).
	 * @return The created Vulkan image view.
	 */
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	/**
	 * @brief Creates a Vulkan shader module.
	 *
	 * This function loads and compiles a SPIR-V shader binary into a Vulkan shader module.
	 *
	 * @param code The binary SPIR-V shader code.
	 * @return The created VkShaderModule.
	 */
	VkShaderModule createShaderModule(const std::vector<char>& code);

	/**
	 * @brief Creates a Vulkan texture image from a file.
	 *
	 * This function loads an image from disk, converts it into a Vulkan-compatible texture,
	 * and uploads it to the GPU.
	 *
	 * @param fileName The path to the texture file.
	 * @return The ID of the created texture image.
	 */
	int createTextureImage(std::string fileName);

	/**
	 * @brief Creates a Vulkan texture.
	 *
	 * This function initializes a Vulkan texture, including the image, memory, and sampler.
	 *
	 * @param fileName The path to the texture file.
	 * @return The ID of the created texture.
	 */
	int createTexture(std::string fileName);

	/**
	 * @brief Creates a descriptor for a texture.
	 *
	 * This function generates a descriptor set for a given texture image view.
	 *
	 * @param textureImage The image view associated with the texture.
	 * @return The ID of the created texture descriptor.
	 */
	int createTextureDescriptor(VkImageView textureImage);


	/**
	 * @brief Loads a texture file from disk.
	 *
	 * This function uses stb_image to load a texture file and extract its pixel data.
	 *
	 * @param fileName The path to the texture file.
	 * @param width Output parameter for the image width.
	 * @param height Output parameter for the image height.
	 * @param imageSize Output parameter for the total image size in bytes.
	 * @return A pointer to the loaded pixel data.
	 */
	stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);

};

