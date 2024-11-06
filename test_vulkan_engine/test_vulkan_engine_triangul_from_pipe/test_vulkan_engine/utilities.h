#pragma once

#include <fstream>

const int MAX_FRAMES_DRAWS = 2;

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Indices (locations) of Queue Families (if they exist at all)
struct QueueFamilyIndices {
	int graphicsFamily = -1;			// Location of Graphics Queue Family
	int presentationFamily = -1;		// Location of Presentation Queue Family

	// Check if queue families are valid
	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

struct SwapChainDetails {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;				// Surface properties, e.g. image size/extent
	std::vector<VkSurfaceFormatKHR> formats;					// Surface image formats, e.g. RGBA and size of each colour
	std::vector<VkPresentModeKHR> presentationModes;			// How images should be presented to screen
};

struct SwapchainImage {
	VkImage image;
	VkImageView imageView;
};

static std::vector<char> readFile(const std::string& filename)
{
	// Open stream from a file
	// read binary file frome end of file
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	// Check if stream is opend
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open a file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> fileBuffer(fileSize);

	// move pointer to start of file
	file.seekg(0);

	// read the file data into the buffer
	file.read(fileBuffer.data(), fileSize);

	file.close();

	return fileBuffer;
}
