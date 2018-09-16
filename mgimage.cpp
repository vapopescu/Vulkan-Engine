#include "mgimage.h"

/**
 * Create the image.
 */
VkResult MgImage::create(const VkcDevice *pDevice, MgImageInfo *pCreateInfo)
{
    // Save the device and image info.
    this->pDevice = pDevice;
    info = *pCreateInfo;

    // Set member fields depending on type.
    switch(info.type) {
    case MG_IMAGE_TYPE_SWAPCHAIN_COLOR:
        imageType = VK_IMAGE_TYPE_2D;
        usageMask = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        break;

    case MG_IMAGE_TYPE_SWAPCHAIN_DEPTH_STENCIL:
        imageType = VK_IMAGE_TYPE_2D;
        usageMask = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        mipLevels = 1;
        break;

    case MG_IMAGE_TYPE_TEXTURE_2D:
        imageType = VK_IMAGE_TYPE_2D;
        usageMask = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (info.type == MG_IMAGE_TYPE_SWAPCHAIN_COLOR)
    {
        // Image created by swapchain.
        handle = info.image;
    }
    else
    {
        // Load image data.
        MgBuffer imageBuffer;
        if (info.type == MG_IMAGE_TYPE_TEXTURE_2D)
        {
            loadImageData(&imageBuffer);
        }

        // Get queue families.
        QVector<uint32_t> queueFamilies;
        pDevice->getQueueFamilies(queueFamilies);

        // Fill image create info.
        VkImageCreateInfo imageInfo =
        {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,            // VkStructureType          sType;
            nullptr,                                        // const void*              pNext;
            0,                                              // VkImageCreateFlags       flags;

            imageType,                                      // VkImageType              imageType;
            info.format,                                    // VkFormat                 format;

            info.extent,                                    // VkExtent3D               extent;
            mipLevels,                                      // uint32_t                 mipLevels;
            1,                                              // uint32_t                 arrayLayers;

            VK_SAMPLE_COUNT_1_BIT,                          // VkSampleCountFlagBits    samples;
            VK_IMAGE_TILING_OPTIMAL,                        // VkImageTiling            tiling;
            usageMask,                                      // VkImageUsageFlags        usage;
            VK_SHARING_MODE_EXCLUSIVE,                      // VkSharingMode            sharingMode;

            static_cast<uint32_t>(queueFamilies.size()),    // uint32_t                 queueFamilyIndexCount;
            queueFamilies.data(),                           // const uint32_t*          pQueueFamilyIndices;

            VK_IMAGE_LAYOUT_UNDEFINED,                      // VkImageLayout            initialLayout;
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
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,         // VkStructureType    sType;
            nullptr,                                        // const void*        pNext;

            memoryRequirements.size,                        // VkDeviceSize       allocationSize;
            memoryTypeIdx                                   // uint32_t           memoryTypeIndex;
        };

        // Allocate image memory.
        vkAllocateMemory(pDevice->logical, &memoryInfo, nullptr, &memory);

        // Bind memory to image.
        vkBindImageMemory(pDevice->logical, handle, memory, 0);

        // Load data and change image layout.
        loadImage(&imageBuffer);
    }

    if (pCreateInfo->createView)
    {
        // Fill image view info.
        VkImageViewCreateInfo viewInfo =
        {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,       // VkStructureType            sType;
            nullptr,                                        // const void*                pNext;
            0,                                              // VkImageViewCreateFlags     flags;

            handle,                                         // VkImage                    image;
            static_cast<VkImageViewType>(imageType),        // VkImageViewType            viewType;
            info.format,                                    // VkFormat                   format;
            {  },                                           // VkComponentMapping         components;

            {                                               // VkImageSubresourceRange    subresourceRange;
                aspectMask,                                     // VkImageAspectFlags    aspectMask;
                0,                                              // uint32_t              baseMipLevel;
                VK_REMAINING_MIP_LEVELS,                        // uint32_t              levelCount;
                0,                                              // uint32_t              baseArrayLayer;
                VK_REMAINING_ARRAY_LAYERS,                      // uint32_t              layerCount;
            }
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
            VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,          // VkStructureType         sType;
            nullptr,                                        // const void*             pNext;
            0,                                              // VkSamplerCreateFlags    flags;

            VK_FILTER_LINEAR,                               // VkFilter                magFilter;
            VK_FILTER_LINEAR,                               // VkFilter                minFilter;
            VK_SAMPLER_MIPMAP_MODE_LINEAR,                  // VkSamplerMipmapMode     mipmapMode;

            VK_SAMPLER_ADDRESS_MODE_REPEAT,                 // VkSamplerAddressMode    addressModeU;
            VK_SAMPLER_ADDRESS_MODE_REPEAT,                 // VkSamplerAddressMode    addressModeV;
            VK_SAMPLER_ADDRESS_MODE_REPEAT,                 // VkSamplerAddressMode    addressModeW;
            0.0f,                                           // float                   mipLodBias;

            anisotropy,                                     // VkBool32                anisotropyEnable;
            anisotropy ? 8.0f : 1.0f,                       // float                   maxAnisotropy;

            VK_FALSE,                                       // VkBool32                compareEnable;
            VK_COMPARE_OP_NEVER,                            // VkCompareOp             compareOp;

            0.0f,                                           // float                   minLod;
            12.0f,                                          // float                   maxLod;

            VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,        // VkBorderColor           borderColor;
            VK_FALSE,                                       // VkBool32                unnormalizedCoordinates;
        };

        // Create sampler.
        vkCreateSampler(pDevice->logical, &samplerInfo, nullptr, &sampler);
    }

    return VK_SUCCESS;
}

/**
 * Destroy the image.
 */
void MgImage::destroy()
{
    if (pDevice != nullptr)
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

        if (handle != VK_NULL_HANDLE && info.type != MG_IMAGE_TYPE_SWAPCHAIN_COLOR)
        {
            vkDestroyImage(pDevice->logical, handle, nullptr);
            handle = VK_NULL_HANDLE;
        }
    }
}

/**
 * Registers the commands to change image layout.
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
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType            sType;
        nullptr,                                    // const void*                pNext;

        srcAccessMask,                              // VkAccessFlags              srcAccessMask;
        dstAccessMask,                              // VkAccessFlags              dstAccessMask;

        oldLayout,                                  // VkImageLayout              oldLayout;
        newLayout,                                  // VkImageLayout              newLayout;

        VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                   srcQueueFamilyIndex;
        VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                   dstQueueFamilyIndex;

        handle,                                     // VkImage                    image;
        {                                           // VkImageSubresourceRange    subresourceRange;
            aspectMask,                                 // VkImageAspectFlags    aspectMask;
            0,                                          // uint32_t              baseMipLevel;
            VK_REMAINING_MIP_LEVELS,                    // uint32_t              levelCount;
            0,                                          // uint32_t              baseArrayLayer;
            VK_REMAINING_ARRAY_LAYERS,                  // uint32_t              layerCount;
        }
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
VkResult MgImage::loadImage(MgBuffer *pImageBuffer)
{
    // Fill fence info.
    VkFenceCreateInfo fenceInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,            // VkStructureType           sType;
        nullptr,                                        // const void*               pNext;
        0                                               // VkFenceCreateFlags        flags;
    };

    // Create fence.
    VkFence fence;
    vkCreateFence(pDevice->logical, &fenceInfo, nullptr, &fence);

    // Get active queue and command buffer.
    VkQueue activeQueue = pDevice->queueFamilies[ACTIVE_FAMILY].queues[0];
    VkCommandBuffer commandBuffer = pDevice->queueFamilies[ACTIVE_FAMILY].commandBuffers[0];

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
    if(pImageBuffer->handle != VK_NULL_HANDLE)
    {
        // Change image layout to transfer destination.
        changeLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);

        // Copy from data buffer to image.
        vkCmdCopyBufferToImage(commandBuffer, pImageBuffer->handle, handle,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(regions.size()), regions.data());
    }

    // Change image layout to optimal.
    changeLayout(VK_IMAGE_LAYOUT_UNDEFINED, layout, commandBuffer);

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

    vkWaitForFences(pDevice->logical, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(pDevice->logical, 1, &fence);

    vkResetCommandBuffer(commandBuffer, 0);

    // Destroy the fence.
    vkDestroyFence(pDevice->logical, fence, nullptr);

    // Destroy the buffer.
    pImageBuffer->destroy();

    return VK_SUCCESS;
}

/**
 * Load the image data from file.
 */
VkResult MgImage::loadImageData(MgBuffer *pImageBuffer)
{
    // Load the image.
    QImage imageData;

    if (!imageData.load(info.filePath))
    {
        qDebug(QString("ERROR:   [@qDebug]").leftJustified(32, ' ')
                .append("- Image \"%1\" not found").arg(info.filePath).toStdString().c_str());
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Prepare the image.
    imageData = imageData.rgbSwapped();
    imageData = imageData.convertToFormat(QImage::Format_RGBA8888);

    // Decide on the image extent.
    if (info.extent.width == 0 || info.extent.height == 0)
    {
        info.extent.width = static_cast<uint32_t>(imageData.width());
        info.extent.height = static_cast<uint32_t>(imageData.height());
    } else {
        imageData = imageData.scaled(static_cast<int>(info.extent.width), static_cast<int>(info.extent.height),
                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // Compute the mip level number.
    uint32_t maxDimension = static_cast<uint32_t>(imageData.width() > imageData.height() ? imageData.width() : imageData.height());
    mipLevels = 0;

    while (maxDimension > 0)
    {
        maxDimension /= 2;
        ++mipLevels;
    }

    // Allocate mip data.
    uint32_t *mipOffsets = new uint32_t[mipLevels];
    uint32_t *mipSizes = new uint32_t[mipLevels];

    // If there is no buffer, return.
    if (pImageBuffer == nullptr)
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // If the buffer exists, destroy it.
    pImageBuffer->destroy();

    // Create image buffer.
    int size = (imageData.width() + 1) * (imageData.height() + 1) * 4 * 4 / 3;
    mgAssert(pImageBuffer->create(static_cast<uint32_t>(size), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, pDevice));

    // Copy image data to the buffer.
    mipOffsets[0] = 0;

    for (uint32_t mip = 0; mip < mipLevels; mip++)
    {
        if (mip > 0)
        {
            mipOffsets[mip] = mipOffsets[mip - 1] + mipSizes[mip - 1];
        }

        mipSizes[mip] = static_cast<uint32_t>(imageData.byteCount());

        // Map buffer memory to host.
        void *data;
        mgAssert(vkMapMemory(pDevice->logical, pImageBuffer->memory, mipOffsets[mip], mipSizes[mip], 0, &data));

        // Copy mip data to buffer.
        memcpy(data, imageData.bits(), mipSizes[mip]);

        // Unmap memory.
        vkUnmapMemory(pDevice->logical, pImageBuffer->memory);

        // Fill region data.
        int width = imageData.width();
        int height = imageData.height();

        VkBufferImageCopy region  =
        {
            mipOffsets[mip],                    // VkDeviceSize                bufferOffset;
            static_cast<uint32_t>(width),       // uint32_t                    bufferRowLength;
            static_cast<uint32_t>(height),          // uint32_t                    bufferImageHeight;

            {                                   // VkImageSubresourceLayers    imageSubresource;
                aspectMask,                         // VkImageAspectFlags    aspectMask;
                mip,                                // uint32_t              mipLevel;
                0,                                  // uint32_t              baseArrayLayer;
                1,                                  // uint32_t              layerCount;
            },

            {0, 0, 0},                          // VkOffset3D                  imageOffset;
            {                                   // VkExtent3D                  imageExtent;
                static_cast<uint32_t>(width),       // uint32_t    width;
                static_cast<uint32_t>(height),      // uint32_t    height;
                1                                   // uint32_t    depth;
            }
        };

        regions.append(region);

        // Downscale the image.
        width = width == 1 ? 1 : width / 2;
        height = height == 1 ? 1 : height / 2;

        imageData = imageData.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // Deallocate mip data.
    delete [] mipOffsets;
    delete [] mipSizes;

    return VK_SUCCESS;
}

/**
 * Record commands for copying image data to buffer.
 */
void MgImage::getImageData(MgBuffer buffer, VkCommandBuffer commandBuffer)
{
    // Change image layout to transfer source.
    changeLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, commandBuffer);

    // Fill image copy info.
    VkBufferImageCopy region =
    {
        0,                          // VkDeviceSize                bufferOffset;
        info.extent.width,          // uint32_t                    bufferRowLength;
        info.extent.height,         // uint32_t                    bufferImageHeight;

        {                           // VkImageSubresourceLayers    imageSubresource;
            aspectMask,                 // VkImageAspectFlags    aspectMask;
            0,                          // uint32_t              mipLevel;
            0,                          // uint32_t              baseArrayLayer;
            1,                          // uint32_t              layerCount;
        },

        {0, 0, 0},                  // VkOffset3D                  imageOffset;
        info.extent                 // VkExtent3D                  imageExtent;
    };

    // Copy from image to buffer.
    vkCmdCopyImageToBuffer(commandBuffer, handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.handle, 1, &region);

    // Change image layout to default.
    changeLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, layout, commandBuffer);
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
