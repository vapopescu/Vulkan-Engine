#include "vkc_swapchain.h"


/**
 * Initialize with empty fields.
 */
VkcSwapchain::VkcSwapchain()
{
    handle =                VK_NULL_HANDLE;
    renderPass =            VK_NULL_HANDLE;

    clearValues[0] =        {0.8f, 0.8f, 1.0f, 1.0f};
    clearValues[1] =        {1.0f, 0.0f};

    logicalDevice =         VK_NULL_HANDLE;
}


/**
 * Create the swapchain.
 */
VkcSwapchain::VkcSwapchain(VkSurfaceKHR surface, const VkcDevice *device) : VkcSwapchain()
{
    //Get surface format number.
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, surface, &formatCount, NULL);

    //Get surface formats.
    surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, surface, &formatCount, surfaceFormats.data());

    if  (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
        surfaceFormats[0].format = VK_FORMAT_R8G8B8A8_UNORM;

    for (int i = 1; i < surfaceFormats.count(); i++)
        if (surfaceFormats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
            surfaceFormats.move(i, 0);


    //Get surface capabilities.
    VkSurfaceCapabilitiesKHR    surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical, surface, &surfaceCapabilities);

    uint32_t imageCount = 3;
    if (imageCount < surfaceCapabilities.minImageCount)
        imageCount = surfaceCapabilities.minImageCount;
    if (imageCount > surfaceCapabilities.maxImageCount && surfaceCapabilities.maxImageCount != 0)
        imageCount = surfaceCapabilities.maxImageCount;

    extent = surfaceCapabilities.currentExtent;
    if (extent.width == UINT32_MAX || extent.height == UINT32_MAX)
    {
#if DEBUG == 1
        qDebug() << "ERROR:   [@qDebug]              - Surface extent is undefined. Swapchain not created.";
#endif
        delete this;
        return;
    }

    VkSurfaceTransformFlagBitsKHR preTransform = surfaceCapabilities.currentTransform;
    if (preTransform & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;


    //Get present mode number.
    uint32_t modeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical, surface, &modeCount, NULL);

    //Get present modes.
    QVector<VkPresentModeKHR> presentModes;
    presentModes.resize(modeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical, surface, &modeCount, presentModes.data());

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (int i = 0; i < presentModes.size(); i++)
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }

    //Get queue families.
    QVector<uint32_t> queueFamilies;
    device->getQueueFamilies(queueFamilies);

    //Fill swap chain info.
    VkSwapchainCreateInfoKHR swapchainInfo =
    {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,    //VkStructureType                  sType;
        NULL,                                           //const void*                      pNext;
        0,                                              //VkSwapchainCreateFlagsKHR        flags;

        surface,                                        //VkSurfaceKHR                     surface;
        imageCount,                                     //uint32_t                         minImageCount;

        surfaceFormats[0].format,                       //VkFormat                         imageFormat;
        surfaceFormats[0].colorSpace,                   //VkColorSpaceKHR                  imageColorSpace;

        extent,                                         //VkExtent2D                       imageExtent;
        1,                                              //uint32_t                         imageArrayLayers;
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,            //VkImageUsageFlags                imageUsage;
        VK_SHARING_MODE_EXCLUSIVE,                      //VkSharingMode                    imageSharingMode;

        (uint32_t)queueFamilies.count(),                //uint32_t                         queueFamilyIndexCount;
        queueFamilies.data(),                           //const uint32_t*                  pQueueFamilyIndices;

        preTransform,                                   //VkSurfaceTransformFlagBitsKHR    preTransform;
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,              //VkCompositeAlphaFlagBitsKHR      compositeAlpha;
        presentMode,                                    //VkPresentModeKHR                 presentMode;
        VK_TRUE,                                        //VkBool32                         clipped;
        VK_NULL_HANDLE                                  //VkSwapchainKHR                   oldSwapchain;
    };

    //Create swap chain.
    vkCreateSwapchainKHR(device->logical, &swapchainInfo, NULL, &handle);


    //Get actual image number.
    vkGetSwapchainImagesKHR(device->logical, handle, &imageCount, NULL);

    //Get images.
    QVector<VkImage> images;
    images.resize(imageCount);
    colorImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device->logical, handle, &imageCount, images.data());

    //Fill data fields.
    this->logicalDevice = device->logical;

    for (uint32_t i = 0; i < imageCount; i++)
        colorImages[i] = new VkcColorImage(images[i], {extent.width, extent.height, 1}, surfaceFormats[0].format, device->logical);


    //Create depth image
    depthImage = new VkcDepthImage({extent.width, extent.height, 1}, device);


    //Fill attachment descriptions.
    VkAttachmentDescription attachmentDescription[2] =
    {
        {
            0,                                                  //VkAttachmentDescriptionFlags    flags;

            surfaceFormats[0].format,                           //VkFormat                        format;
            VK_SAMPLE_COUNT_1_BIT,                              //VkSampleCountFlagBits           samples;

            VK_ATTACHMENT_LOAD_OP_CLEAR,                        //VkAttachmentLoadOp              loadOp;
            VK_ATTACHMENT_STORE_OP_STORE,                       //VkAttachmentStoreOp             storeOp;
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,                    //VkAttachmentLoadOp              stencilLoadOp;
            VK_ATTACHMENT_STORE_OP_DONT_CARE,                   //VkAttachmentStoreOp             stencilStoreOp;

            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,           //VkImageLayout                   initialLayout;
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,           //VkImageLayout                   finalLayout;

        },

        {
            0,                                                  //VkAttachmentDescriptionFlags    flags;

            depthImage->format,                                 //VkFormat                        format;
            VK_SAMPLE_COUNT_1_BIT,                              //VkSampleCountFlagBits           samples;

            VK_ATTACHMENT_LOAD_OP_CLEAR,                        //VkAttachmentLoadOp              loadOp;
            VK_ATTACHMENT_STORE_OP_DONT_CARE,                   //VkAttachmentStoreOp             storeOp;
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,                    //VkAttachmentLoadOp              stencilLoadOp;
            VK_ATTACHMENT_STORE_OP_DONT_CARE,                   //VkAttachmentStoreOp             stencilStoreOp;

            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,   //VkImageLayout                   initialLayout;
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,   //VkImageLayout                   finalLayout;

        }
    };

    //Fill attachment references.
    VkAttachmentReference attachmentReference[2] =
    {
        {
            0,                                                  //uint32_t         attachment;
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL            //VkImageLayout    layout;
        },

        {
            1,                                                  //uint32_t         attachment;
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,   //VkImageLayout    layout;
        }
    };

    //Fill subpass description.
    VkSubpassDescription subpassDescription =
    {
        0,                                  //VkSubpassDescriptionFlags       flags;
        VK_PIPELINE_BIND_POINT_GRAPHICS,    //VkPipelineBindPouint32_t        pipelineBindPoint;

        0,                                  //uint32_t                        inputAttachmentCount;
        NULL,                               //const VkAttachmentReference*    pInputAttachments;

        1,                                  //uint32_t                        colorAttachmentCount;
        &attachmentReference[0],            //const VkAttachmentReference*    pColorAttachments;

        NULL,                               //const VkAttachmentReference*    pResolveAttachments;

        &attachmentReference[1],            //const VkAttachmentReference*    pDepthStencilAttachment;

        0,                                  //uint32_t                        preserveAttachmentCount;
        NULL,                               //const uint32_t*                 pPreserveAttachments;
    };

    //Fill render pass create info.
    VkRenderPassCreateInfo renderPassInfo =
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,  //VkStructureType                   sType;
        NULL,                                       //const void*                       pNext;
        0,                                          //VkRenderPassCreateFlags           flags;

        2,                                          //uint32_t                          attachmentCount;
        attachmentDescription,                      //const VkAttachmentDescription*    pAttachments;

        1,                                          //uint32_t                          subpassCount;
        &subpassDescription,                        //const VkSubpassDescription*       pSubpasses;

        0,                                          //uint32_t                          dependencyCount;
        NULL                                        //const VkSubpassDependency*        pDependencies;
    };

    //Create render pass.
    vkCreateRenderPass(device->logical, &renderPassInfo, NULL, &renderPass);


    //Fill framebuffer info.
    VkImageView attachments[2];
    attachments[1] = depthImage->view;

    VkFramebufferCreateInfo frameBufferInfo =
    {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,  //VkStructureType             sType;
        NULL,                                       //const void*                 pNext;
        0,                                          //VkFramebufferCreateFlags    flags;

        renderPass,                                 //VkRenderPass                renderPass;
        2,                                          //uint32_t                    attachmentCount;
        attachments,                                //const VkImageView*          pAttachments;

        extent.width,                               //uint32_t                    width;
        extent.height,                              //uint32_t                    height;
        1                                           //uint32_t                    layers;
    };

    //Create a framebuffer for each image in the swap chain.
    frameBuffers.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        attachments[0] = colorImages[i]->view;

        vkCreateFramebuffer(device->logical, &frameBufferInfo, NULL, &frameBuffers[i]);
    }
}


/**
 * Destroy the swapchain.
 */
VkcSwapchain::~VkcSwapchain()
{
    if (logicalDevice != VK_NULL_HANDLE)
    {
        if (renderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(logicalDevice, renderPass, NULL);

        while (frameBuffers.size() > 0)
        {
            if (frameBuffers[0] != VK_NULL_HANDLE)
                vkDestroyFramebuffer(logicalDevice, frameBuffers[0], NULL);

            frameBuffers.removeFirst();
        }

        while (colorImages.size() > 0)
        {
            if (colorImages[0]->view != VK_NULL_HANDLE)
                vkDestroyImageView(logicalDevice, colorImages[0]->view, NULL);

            colorImages.removeFirst();
        }

        if (depthImage != NULL)
            delete depthImage;

        if (handle != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(logicalDevice, handle, NULL);
    }
}
