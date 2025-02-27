#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

class ShadowMapFrameBuffer
{
public:
	ShadowMapFrameBuffer();

	~ShadowMapFrameBuffer();

	bool Init(unsigned int width, unsigned int height);

	void BlindForWriting();

	void BlindForReading();

private:
	uint32_t m_width = 0;
	uint32_t m_height = 0;

	VkFramebuffer m_framebuffer;

	VkImage m_shadowMapImage;
	VkImageView m_shadowMapView;
	VkDeviceMemory m_shadowMapMemory;
};

