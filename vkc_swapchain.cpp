#include "vkc_swapchain.h"


/**
 * Initialize with empty fields.
 */
VkcSwapchain::VkcSwapchain()
{
    handle =                VK_NULL_HANDLE;
    renderPass =            VK_NULL_HANDLE;

    clearValues[0] =        {0.8f, 0.8f, 1.0f, 1.0f};
    clearValues[1] =        {0.0f, 0};

    imageCount =            0;
}


/**
 * Initialize the swapchain.
 */
VkcSwapchain::VkcSwapchain(VkSurfaceKHR surface, const VkcDevice *device) : VkcSwapchain()
{
    createSwapchain(surface, device);
    createImages();
    createRenderPass();
    createFramebuffers();
}


/**
 * Initialize the swapchain using existent renderpass.
 */
VkcSwapchain::VkcSwapchain(VkRenderPass renderPass, VkSurfaceKHR surface, const VkcDevice *device) : VkcSwapchain()
{
    createSwapchain(surface, device);
    createImages();
    this->renderPass = renderPass;
    createFramebuffers();
}


/**
 * Destroy the swapchain.
 */
VkcSwapchain::~VkcSwapchain()
{
    if (device->logical != VK_NULL_HANDLE)
    {
        if (renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device->logical, renderPass, nullptr);
        }

        while (framebuffers.size() > 0)
        {
            if (framebuffers[0] != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(device->logical, framebuffers[0], nullptr);
            }

            framebuffers.removeFirst();
        }

        while (colorImages.size() > 0)
        {
            colorImages[0]->destroy();
            colorImages.removeFirst();
        }

        depthStencilImage.destroy();

        if (handle != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device->logical, handle, nullptr);
        }
    }
}


/**
 * Create the swapchain.
 */
void VkcSwapchain::createSwapchain(VkSurfaceKHR surface, const VkcDevice *device)
{
    // Fill data fields.
    this->device = device;

    // Get surface format number.
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, surface, &formatCount, nullptr);

    // Get surface formats.
    surfaceFormats.clear();
    surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, surface, &formatCount, surfaceFormats.data());

    if  (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormats[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    }

    for (int i = 1; i < surfaceFormats.size(); i++)
    {
        if (surfaceFormats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
        {
           surfaceFormats.move(i, 0);
           break;
        }
    }

    // Get surface capabilities.
    VkSurfaceCapabilitiesKHR    surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical, surface, &surfaceCapabilities);

    extent = surfaceCapabilities.currentExtent;

    VkSurfaceTransformFlagBitsKHR preTransform = surfaceCapabilities.currentTransform;
    if (preTransform & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;


    // Get present mode number.
    uint32_t modeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical, surface, &modeCount, nullptr);

    // Get present modes.
    QVector<VkPresentModeKHR> presentModes;
    presentModes.resize(modeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical, surface, &modeCount, presentModes.data());

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    imageCount = 2;
    for (int i = 0; i < presentModes.size(); i++)
    {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            imageCount = 3;
            break;
        }
    }

    if (imageCount < surfaceCapabilities.minImageCount)
    {
        imageCount = surfaceCapabilities.minImageCount;
    }
    if (imageCount > surfaceCapabilities.maxImageCount && surfaceCapabilities.maxImageCount != 0)
    {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    // Get queue families.
    QVector<uint32_t> queueFamilies;
    device->getQueueFamilies(queueFamilies);

    // Fill swap chain info.
    VkSwapchainCreateInfoKHR swapchainInfo =
    {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,    // VkStructureType                  sType;
        nullptr,                                           // const void*                      pNext;
        0,                                              // VkSwapchainCreateFlagsKHR        flags;

        surface,                                        // VkSurfaceKHR                     surface;
        imageCount,                                     // uint32_t                         minImageCount;

        surfaceFormats[0].format,                       // VkFormat                         imageFormat;
        surfaceFormats[0].colorSpace,                   // VkColorSpaceKHR                  imageColorSpace;

        extent,                                         // VkExtent2D                       imageExtent;
        1,                                              // uint32_t                         imageArrayLayers;
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |           // VkImageUsageFlags                imageUsage;
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_SHARING_MODE_EXCLUSIVE,                      // VkSharingMode                    imageSharingMode;

        (uint32_t)queueFamilies.count(),                // uint32_t                         queueFamilyIndexCount;
        queueFamilies.data(),                           // const uint32_t*                  pQueueFamilyIndices;

        preTransform,                                   // VkSurfaceTransformFlagBitsKHR    preTransform;
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,              // VkCompositeAlphaFlagBitsKHR      compositeAlpha;
        presentMode,                                    // VkPresentModeKHR                 presentMode;
        VK_TRUE,                                        // VkBool32                         clipped;
        VK_NULL_HANDLE                                  // VkSwapchainKHR                   oldSwapchain;
    };

    // Create swap chain.
    vkCreateSwapchainKHR(device->logical, &swapchainInfo, nullptr, &handle);
}


/**
 * Create the swapchain images.
 */
void VkcSwapchain::createImages()
{
    // Get actual image number.
    vkGetSwapchainImagesKHR(device->logical, handle, &imageCount, nullptr);

    // Get images.
    QVector<VkImage> images;
    images.resize(imageCount);
    colorImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device->logical, handle, &imageCount, images.data());

    // Create color images.
    for (uint32_t i = 0; i < imageCount; i++)
    {
        MgImageInfo colorImageInfo =
        {
            MG_IMAGE_TYPE_SWAPCHAIN_COLOR,              // MgImageType                 type;
            "",                                         // QString                     filePath;
            {                                           // VkExtent3D                  extent;
                extent.width,                               // uint32_t                width;
                extent.height,                              // uint32_t                height;
                1                                           // uint32_t                depth;
            },
            surfaceFormats[0].format,                   // VkFormat                    format;

            images[i],                                  // const VkImage               image;
            VK_TRUE,                                    // VkBool32                    createView;
            VK_TRUE                                     // VkBool32                    createSampler;
        };

        colorImages[i] = new MgImage();
        colorImages[i]->create(device, &colorImageInfo);
    }

    MgImageInfo depthStencilImageInfo =
    {
        MG_IMAGE_TYPE_SWAPCHAIN_DEPTH_STENCIL,          // MgImageType                 type;
        "",                                             // QString                     filePath;
        {                                               // VkExtent3D                  extent;
            extent.width,                                   // uint32_t                width;
            extent.height,                                  // uint32_t                height;
            1                                               // uint32_t                depth;
        },
        VK_FORMAT_D32_SFLOAT_S8_UINT,                   // VkFormat                    format;

        VK_NULL_HANDLE,                                 // const VkImage               image;
        VK_TRUE,                                        // VkBool32                    createView;
        VK_TRUE                                         // VkBool32                    createSampler;
    };

    // Create depth image.
    depthStencilImage.create(device, &depthStencilImageInfo);
}


/**
 * Creates the render pass.
 */
void VkcSwapchain::createRenderPass()
{
    // Fill attachment descriptions.
    VkAttachmentDescription attachmentDescription[2] =
    {
        {
            0,                                                  // VkAttachmentDescriptionFlags    flags;

            surfaceFormats[0].format,                           // VkFormat                        format;
            VK_SAMPLE_COUNT_1_BIT,                              // VkSampleCountFlagBits           samples;

            VK_ATTACHMENT_LOAD_OP_CLEAR,                        // VkAttachmentLoadOp              loadOp;
            VK_ATTACHMENT_STORE_OP_STORE,                       // VkAttachmentStoreOp             storeOp;
            VK_ATTACHMENT_LOAD_OP_CLEAR,                        // VkAttachmentLoadOp              stencilLoadOp;
            VK_ATTACHMENT_STORE_OP_STORE,                       // VkAttachmentStoreOp             stencilStoreOp;

            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,           // VkImageLayout                   initialLayout;
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,           // VkImageLayout                   finalLayout;

        },

        {
            0,                                                  // VkAttachmentDescriptionFlags    flags;

            depthStencilImage.info.format,                      // VkFormat                        format;
            VK_SAMPLE_COUNT_1_BIT,                              // VkSampleCountFlagBits           samples;

            VK_ATTACHMENT_LOAD_OP_CLEAR,                        // VkAttachmentLoadOp              loadOp;
            VK_ATTACHMENT_STORE_OP_STORE,                       // VkAttachmentStoreOp             storeOp;
            VK_ATTACHMENT_LOAD_OP_CLEAR,                        // VkAttachmentLoadOp              stencilLoadOp;
            VK_ATTACHMENT_STORE_OP_STORE,                       // VkAttachmentStoreOp             stencilStoreOp;

            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,   // VkImageLayout                   initialLayout;
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,   // VkImageLayout                   finalLayout;

        }
    };

    // Fill attachment references.
    VkAttachmentReference attachmentReference[2] =
    {
        {
            0,                                                  // uint32_t         attachment;
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL            // VkImageLayout    layout;
        },

        {
            1,                                                  // uint32_t         attachment;
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,   // VkImageLayout    layout;
        }
    };

    // Fill subpass description.
    VkSubpassDescription subpassDescription =
    {
        0,                                  // VkSubpassDescriptionFlags       flags;
        VK_PIPELINE_BIND_POINT_GRAPHICS,    // VkPipelineBindPouint32_t        pipelineBindPoint;

        0,                                  // uint32_t                        inputAttachmentCount;
        nullptr,                            // const VkAttachmentReference*    pInputAttachments;

        1,                                  // uint32_t                        colorAttachmentCount;
        &attachmentReference[0],            // const VkAttachmentReference*    pColorAttachments;

        nullptr,                            // const VkAttachmentReference*    pResolveAttachments;

        &attachmentReference[1],            // const VkAttachmentReference*    pDepthStencilAttachment;

        0,                                  // uint32_t                        preserveAttachmentCount;
        nullptr,                            // const uint32_t*                 pPreserveAttachments;
    };

    // Fill render pass create info.
    VkRenderPassCreateInfo renderPassInfo =
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,  // VkStructureType                   sType;
        nullptr,                                    // const void*                       pNext;
        0,                                          // VkRenderPassCreateFlags           flags;

        2,                                          // uint32_t                          attachmentCount;
        attachmentDescription,                      // const VkAttachmentDescription*    pAttachments;

        1,                                          // uint32_t                          subpassCount;
        &subpassDescription,                        // const VkSubpassDescription*       pSubpasses;

        0,                                          // uint32_t                          dependencyCount;
        nullptr                                     // const VkSubpassDependency*        pDependencies;
    };

    // Create render pass.
    vkCreateRenderPass(device->logical, &renderPassInfo, nullptr, &renderPass);
}


/**
 * Creates the framebuffers.
 */
void VkcSwapchain::createFramebuffers()
{
    // Fill framebuffer info.
    VkImageView attachments[2];
    attachments[1] = depthStencilImage.view;

    VkFramebufferCreateInfo framebufferInfo =
    {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,  // VkStructureType             sType;
        nullptr,                                    // const void*                 pNext;
        0,                                          // VkFramebufferCreateFlags    flags;

        renderPass,                                 // VkRenderPass                renderPass;
        2,                                          // uint32_t                    attachmentCount;
        attachments,                                // const VkImageView*          pAttachments;

        extent.width,                               // uint32_t                    width;
        extent.height,                              // uint32_t                    height;
        1                                           // uint32_t                    layers;
    };

    // Create a framebuffer for each image in the swap chain.
    framebuffers.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        attachments[0] = colorImages[i]->view;

        vkCreateFramebuffer(device->logical, &framebufferInfo, nullptr, &framebuffers[i]);
    }
}
