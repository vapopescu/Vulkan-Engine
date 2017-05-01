#include "mgimage.h"

/**
 * Create the image.
 */
VkResult MgImage::create(const VkcDevice *pDevice, MgImageInfo *pCreateInfo)
{
    handle = pCreateInfo->image;
    info = *pCreateInfo;

    if (handle == VK_NULL_HANDLE)
    {
        // This image is unique and must be destroyed.
        sharedImage = false;

        // Get queue families.
        QVector<uint32_t> queueFamilies;
        pDevice->getQueueFamilies(queueFamilies);

        // Fill image create info.
        VkImageCreateInfo imageInfo =
        {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,    // VkStructureType          sType;
            nullptr,                                // const void*              pNext;
            0,                                      // VkImageCreateFlags       flags;

            info.type,                              // VkImageType              imageType;
            info.format,                            // VkFormat                 format;

            info.extent,                            // VkExtent3D               extent;
            1,                                      // uint32_t                 mipLevels;
            1,                                      // uint32_t                 arrayLayers;

            VK_SAMPLE_COUNT_1_BIT,                  // VkSampleCountFlagBits    samples;
            VK_IMAGE_TILING_OPTIMAL,                // VkImageTiling            tiling;
            info.usage,                             // VkImageUsageFlags        usage;
            VK_SHARING_MODE_EXCLUSIVE,              // VkSharingMode            sharingMode;

            (uint32_t)queueFamilies.size(),         // uint32_t                 queueFamilyIndexCount;
            queueFamilies.data(),                   // const uint32_t*          pQueueFamilyIndices;

            VK_IMAGE_LAYOUT_UNDEFINED,              // VkImageLayout            initialLayout;
        };

        // Create image.
        vkCreateImage(pDevice->logical, &imageInfo, nullptr, &handle);

        // Get image memory requirements.
        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(pDevice->logical, handle, &memoryRequirements);

        // Get memory type index.
        uint32_t memoryTypeIdx = 0;
        VkMemoryPropertyFlags memoryType = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        pDevice->getMemoryTypeIndex(memoryType, memoryRequirements, &memoryTypeIdx);

        // Fill image memory allocate info.
        VkMemoryAllocateInfo memoryInfo =
        {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // VkStructureType    sType;
            nullptr,                                // const void*        pNext;

            memoryRequirements.size,                // VkDeviceSize       allocationSize;
            memoryTypeIdx                           // uint32_t           memoryTypeIndex;
        };

        // Allocate image memory.
        vkAllocateMemory(pDevice->logical, &memoryInfo, nullptr, &memory);

        // Bind memory to image.
        vkBindImageMemory(pDevice->logical, handle, memory, 0);

        // Load data and change image layout.
        loadImage(pDevice);
    }

    if (pCreateInfo->createView)
    {
        // Fill image view info.
        VkImageViewCreateInfo viewInfo =
        {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,   // VkStructureType            sType;
            nullptr,                                       // const void*                pNext;
            0,                                          // VkImageViewCreateFlags     flags;

            handle,                                     // VkImage                    image;
            (VkImageViewType)info.type,                 // VkImageViewType            viewType;
            info.format,                                // VkFormat                   format;
            {  },                                       // VkComponentMapping         components;
            info.resourceRange                          // VkImageSubresourceRange    subresourceRange;
        };

        // Create image view.
        vkCreateImageView(pDevice->logical, &viewInfo, nullptr, &view);
    }

    if (pCreateInfo->createSampler)
    {
        VkBool32 anisotropy = pDevice->features.samplerAnisotropy;

        // Fill sampler info.
        VkSamplerCreateInfo samplerInfo =
        {
            VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,      // VkStructureType         sType;
            nullptr,                                       // const void*             pNext;
            0,                                          // VkSamplerCreateFlags    flags;

            VK_FILTER_LINEAR,                           // VkFilter                magFilter;
            VK_FILTER_LINEAR,                           // VkFilter                minFilter;
            VK_SAMPLER_MIPMAP_MODE_LINEAR,              // VkSamplerMipmapMode     mipmapMode;

            VK_SAMPLER_ADDRESS_MODE_REPEAT,             // VkSamplerAddressMode    addressModeU;
            VK_SAMPLER_ADDRESS_MODE_REPEAT,             // VkSamplerAddressMode    addressModeV;
            VK_SAMPLER_ADDRESS_MODE_REPEAT,             // VkSamplerAddressMode    addressModeW;
            0.0f,                                       // float                   mipLodBias;

            anisotropy,                                 // VkBool32                anisotropyEnable;
            anisotropy ? 8.0f : 1.0f,                   // float                   maxAnisotropy;

            VK_FALSE,                                   // VkBool32                compareEnable;
            VK_COMPARE_OP_NEVER,                        // VkCompareOp             compareOp;

            0.0f,                                       // float                   minLod;
            1.0f,                                       // float                   maxLod;

            VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,    // VkBorderColor           borderColor;
            VK_FALSE,                                   // VkBool32                unnormalizedCoordinates;
        };

        // Create sampler.
        vkCreateSampler(pDevice->logical, &samplerInfo, nullptr, &sampler);
    }

    return VK_SUCCESS;
}

/**
 * Destroy the image.
 */
void MgImage::destroy(const VkcDevice* pDevice)
{
    if (view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(pDevice->logical, view, nullptr);
        view = VK_NULL_HANDLE;
    }

    if (sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(pDevice->logical, sampler, nullptr);
        sampler = VK_NULL_HANDLE;
    }

    if (memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(pDevice->logical, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }

    imageBuffer.destroy();

    if (handle != VK_NULL_HANDLE && !sharedImage)
    {
        vkDestroyImage(pDevice->logical, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}

/**
 * Record commands for copying image data to buffer.
 */
void MgImage::getImageData(MgBuffer buffer, VkCommandBuffer commandBuffer)
{
    // Change image layout to transfer source.
    changeLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, commandBuffer);

    // Fill resource layer info.
    VkImageSubresourceLayers resourceLayer =
    {
        info.resourceRange.aspectMask,      // VkImageAspectFlags    aspectMask;
        info.resourceRange.baseMipLevel,    // uint32_t              mipLevel;
        info.resourceRange.baseArrayLayer,  // uint32_t              baseArrayLayer;
        info.resourceRange.layerCount       // uint32_t              layerCount;
    };

    // Fill image copy info.
    VkBufferImageCopy region =
    {
        0,                                  // VkDeviceSize                bufferOffset;
        info.extent.width,                  // uint32_t                    bufferRowLength;
        info.extent.height,                 // uint32_t                    bufferImageHeight;

        resourceLayer,                      // VkImageSubresourceLayers    imageSubresource;
        {0, 0, 0},                          // VkOffset3D                  imageOffset;
        info.extent                         // VkExtent3D                  imageExtent;
    };

    // Copy from image to buffer.
    vkCmdCopyImageToBuffer(commandBuffer, handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.handle, 1, &region);
}

/**
 * Registers the commands to load image data and change image layout.
 */
void MgImage::changeLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer)
{
    // Setup access masks.
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;

    getAccessMask(srcAccessMask, oldLayout);
    getAccessMask(dstAccessMask, newLayout);

    // Setup image barrier.
    VkImageMemoryBarrier imageBarrier =
    {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, // VkStructureType            sType;
        nullptr,                                // const void*                pNext;

        srcAccessMask,                          // VkAccessFlags              srcAccessMask;
        dstAccessMask,                          // VkAccessFlags              dstAccessMask;

        oldLayout,                              // VkImageLayout              oldLayout;
        newLayout,                              // VkImageLayout              newLayout;

        VK_QUEUE_FAMILY_IGNORED,                // uint32_t                   srcQueueFamilyIndex;
        VK_QUEUE_FAMILY_IGNORED,                // uint32_t                   dstQueueFamilyIndex;

        handle,                                 // VkImage                    image;
        info.resourceRange                      // VkImageSubresourceRange    subresourceRange;
    };

    // Register barrier in command buffer.
    vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageBarrier
                );
}

/**
 * Change image layout to optimal and load the buffer data to image.
 */
VkResult MgImage::loadImage(const VkcDevice *device)
{
    // Fill fence info.
    VkFenceCreateInfo fenceInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    // VkStructureType           sType;
        nullptr,                                // const void*               pNext;
        0                                       // VkFenceCreateFlags        flags;
    };

    // Create fence.
    VkFence fence;
    vkCreateFence(device->logical, &fenceInfo, nullptr, &fence);

    // Get active queue and command buffer.
    VkQueue activeQueue = device->queueFamilies[ACTIVE_FAMILY].queues[0];
    VkCommandBuffer commandBuffer = device->queueFamilies[ACTIVE_FAMILY].commandBuffers[0];

    // Fill commmand buffer begin info.
    VkCommandBufferBeginInfo commandBufferBeginInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,    // VkStructureType                          sType;
        nullptr,                                        // const void*                              pNext;
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,    // VkCommandBufferUsageFlags                flags;

        nullptr                                         // const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    };

    // Begin command recording.
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    // If image data exists, load it.
    if(imageBuffer.handle != VK_NULL_HANDLE)
    {
        // Change image layout to transfer destination.
        changeLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);

        // Fill resource layer info.
        VkImageSubresourceLayers resourceLayer =
        {
            info.resourceRange.aspectMask,      // VkImageAspectFlags    aspectMask;
            info.resourceRange.baseMipLevel,    // uint32_t              mipLevel;
            info.resourceRange.baseArrayLayer,  // uint32_t              baseArrayLayer;
            info.resourceRange.layerCount       // uint32_t              layerCount;
        };

        // Fill image copy
        VkBufferImageCopy region =
        {
            0,                                  // VkDeviceSize                bufferOffset;
            info.extent.width,                  // uint32_t                    bufferRowLength;
            info.extent.height,                 // uint32_t                    bufferImageHeight;

            resourceLayer,                      // VkImageSubresourceLayers    imageSubresource;
            {0, 0, 0},                          // VkOffset3D                  imageOffset;
            info.extent                         // VkExtent3D                  imageExtent;
        };

        // Copy from data buffer to image.
        vkCmdCopyBufferToImage(commandBuffer, imageBuffer.handle, handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    // Change image layout to optimal.
    changeLayout(VK_IMAGE_LAYOUT_UNDEFINED, info.layout, commandBuffer);

    // Stop command recording.
    vkEndCommandBuffer(commandBuffer);

    // Fill queue submit info.
    VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo =
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,      // VkStructureType                sType;
        nullptr,                            // const void*                    pNext;

        0,                                  // uint32_t                       waitSemaphoreCount;
        nullptr,                            // const VkSemaphore*             pWaitSemaphores;

        &stageMask,                         // const VkPipelineStageFlags*    pWaitDstStageMask;

        1,                                  // uint32_t                       commandBufferCount;
        &commandBuffer,                     // const VkCommandBuffer*         pCommandBuffers;

        0,                                  // uint32_t                       signalSemaphoreCount;
        nullptr                             // const VkSemaphore*             pSignalSemaphores;
    };

    // Submit queue.
    vkQueueSubmit(activeQueue, 1, &submitInfo, fence);

    vkWaitForFences(device->logical, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->logical, 1, &fence);

    vkResetCommandBuffer(commandBuffer, 0);

    // Destroy the fence.
    vkDestroyFence(device->logical, fence, nullptr);

    return VK_SUCCESS;
}

/**
 * Get the access mask specific to the image layout.
 */
void MgImage::getAccessMask(VkAccessFlags &accessMask, VkImageLayout layout)
{
    switch (layout)
    {
    case VK_IMAGE_LAYOUT_GENERAL:
        accessMask =
                VK_ACCESS_MEMORY_READ_BIT |
                VK_ACCESS_MEMORY_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        accessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        accessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        accessMask =
                VK_ACCESS_MEMORY_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        accessMask =
                VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        accessMask =
                VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        accessMask =
                VK_ACCESS_SHADER_READ_BIT;
        break;

    default:
        accessMask = 0;
    }
}
