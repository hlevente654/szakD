#include <vulkan/vulkan.h>

#include <iostream>

class ShadowMappingHandler {
public:

    VkImage        shadowDepthImage;      // Shadow map mélységi textúra
    VkDeviceMemory shadowDepthMemory;     // A mélységi textúra memóriája
    VkImageView    shadowDepthImageView;  // A mélységi textúra image view-ja
    VkFramebuffer  shadowFramebuffer;     // Framebuffer, ami a depth image-t használja
    VkSampler      shadowSampler;         // Sampler az árnyéktérkép lekérdezéséhez

    // Devices
    VkDevice* logicalDevice;
    VkPhysicalDevice* physicalDevice;

    // Shadow render pass, amely az árnyéktérkép generálására szolgál
    VkRenderPass shadowRenderPass;

    // Konstruktor: inicializálja az objektumot és létrehozza a shadow render pass-t
    ShadowMappingHandler(VkDevice *device, VkPhysicalDevice * physicalDevice) {
        this->logicalDevice = device;
        this->physicalDevice = physicalDevice;
        createRenderPass(*this->logicalDevice);
        createShadowDepthImage();
        createShadowDepthImageView();
        createShadowFramebuffer();
        createShadowSampler();
    }

    ~ShadowMappingHandler()
    {
        cleanUp();
    }


private:

    void createShadowSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;  // nem használjuk, clamp to edge van
        samplerInfo.compareEnable = VK_TRUE;
        samplerInfo.compareOp = VK_COMPARE_OP_LESS;  // árnyék összehasonlítás (depth teszt a samplerben)

        VkResult res = vkCreateSampler(*logicalDevice, &samplerInfo, nullptr, &shadowSampler);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shadow map sampler!");
        }

    }

    void createShadowFramebuffer()
    {
        // Tegyük fel, hogy rendelkezésre áll egy shadowRenderPass, amely egyetlen depth attachment-et vár
        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = shadowRenderPass;               // Render pass, amely a shadow map-et készíti
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = &shadowDepthImageView;        // Csak a depth attachment
        fbInfo.width = 1024;
        fbInfo.height = 1024;
        fbInfo.layers = 1;

        VkResult res = vkCreateFramebuffer(*logicalDevice, &fbInfo, nullptr, &shadowFramebuffer);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shadow framebuffer!");
        }

    }

    void createShadowDepthImageView()
    {
        // 2. ImageView létrehozása a depth image-hez
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = shadowDepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_D32_SFLOAT;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult res = vkCreateImageView(*logicalDevice, &viewInfo, nullptr, &shadowDepthImageView);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shadow depth ImageView!");
        }

    };

    void createShadowDepthImage()
    {
        // 1. VkImage létrehozása a depth textúrához (1024x1024, D32_SFLOAT, Depth/Stencil attachment és sampled használat)
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = 1024;
        imageInfo.extent.height = 1024;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_D32_SFLOAT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult res = vkCreateImage(*logicalDevice, &imageInfo, nullptr, &shadowDepthImage);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shadow depth image!");
        }

        // 1/a. Memória követelmények lekérdezése és memória allokálása (Device local)
        VkMemoryRequirements memReq;
        vkGetImageMemoryRequirements(*logicalDevice, shadowDepthImage, &memReq);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        // findMemoryType egy segédfüggvény, ami kiválasztja a megfelelõ memória típust 
        // (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT tulajdonsággal)
        allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        res = vkAllocateMemory(*logicalDevice, &allocInfo, nullptr, &shadowDepthMemory);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate shadow depth memory!");
        }

        // 1/b. Az image és a memória összekapcsolása
        vkBindImageMemory(*logicalDevice, shadowDepthImage, shadowDepthMemory, 0);
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory type!");
    }

    void cleanUp()
    {
        if (shadowSampler != VK_NULL_HANDLE) {
            vkDestroySampler(*logicalDevice, shadowSampler, nullptr);
            shadowSampler = VK_NULL_HANDLE;
        }

        if (shadowDepthImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(*logicalDevice, shadowDepthImageView, nullptr);
            shadowDepthImageView = VK_NULL_HANDLE;
        }

        if (shadowFramebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(*logicalDevice, shadowFramebuffer, nullptr);
            shadowFramebuffer = VK_NULL_HANDLE;
        }

        if (shadowDepthImage != VK_NULL_HANDLE) {
            vkDestroyImage(*logicalDevice, shadowDepthImage, nullptr);
            shadowDepthImage = VK_NULL_HANDLE;
        }

        if (shadowDepthMemory != VK_NULL_HANDLE) {
            vkFreeMemory(*logicalDevice, shadowDepthMemory, nullptr);
            shadowDepthMemory = VK_NULL_HANDLE;
        }

        if (shadowRenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(*logicalDevice, shadowRenderPass, nullptr);
            shadowRenderPass = VK_NULL_HANDLE;
        }
    }


    // Render pass létrehozása a mélységi árnyéktérképhez
    void createRenderPass(VkDevice device) {
        // 1. Depth attachment leírása (formátum: D32 float, csak depth, nincs color)
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = VK_FORMAT_D32_SFLOAT;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // A render pass végén a depth kép shader-bõl olvasható lesz
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // 2. Depth attachment referencia a subpass számára
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 0;  // az elsõ (és egyetlen) attachment indexe
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // 3. Subpass leírása (csak depth attachment, nincs color attachment)
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;                            // nincs color attachment
        subpass.pColorAttachments = nullptr;                      // nincs color attachment lista
        subpass.pDepthStencilAttachment = &depthAttachmentRef;          // depth attachment referencia
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = nullptr;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments = nullptr;
        subpass.pResolveAttachments = nullptr;

        // 4. Render pass create info összeállítása
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;                 // egy attachment (depth)
        renderPassInfo.pAttachments = &depthAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 0;                 // késõbb itt adhatunk meg subpass függõségeket
        renderPassInfo.pDependencies = nullptr;

        // 5. Render pass létrehozása a Vulkan eszközzel (device)
        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &shadowRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shadow render pass!");
        }
    }
};
