#include "vkc_image.h"


/**
 * Initialize with empty fields.
 */
VkcImage::VkcImage()
{
    handle =                VK_NULL_HANDLE;
    view =                  VK_NULL_HANDLE;
    sampler =               VK_NULL_HANDLE;

    buffer =                VK_NULL_HANDLE;
    memory =                VK_NULL_HANDLE;
    logicalDevice =         VK_NULL_HANDLE;
}


/**
 * Destroy the image.
 */
VkcImage::~VkcImage()
{
    if (logicalDevice != VK_NULL_HANDLE)
    {
        if (view != VK_NULL_HANDLE)
        {
            vkDestroyImageView(logicalDevice, view, NULL);
            view = VK_NULL_HANDLE;
        }

        if (sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(logicalDevice, sampler, NULL);
            sampler = VK_NULL_HANDLE;
        }

        if (memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(logicalDevice, memory, NULL);
            memory = VK_NULL_HANDLE;
        }

        if (handle != VK_NULL_HANDLE)
        {
            vkDestroyImage(logicalDevice, handle, NULL);
            handle = VK_NULL_HANDLE;
        }

        logicalDevice = VK_NULL_HANDLE;
    }
}


/**
 * Create the image data and allocate memory.
 */
void VkcImage::createImage(const VkcDevice *device)
{
    //Get queue families.
    QVector<uint32_t> queueFamilies;
    device->getQueueFamilies(queueFamilies);

    //Fill image create info.
    VkImageCreateInfo imageInfo =
    {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,            //VkStructureType          sType;
        NULL,                                           //const void*              pNext;
        0,                                              //VkImageCreateFlags       flags;

        type,                                           //VkImageType              imageType;
        format,                                         //VkFormat                 format;

        extent,                                         //VkExtent3D               extent;
        1,                                              //uint32_t                 mipLevels;
        1,                                              //uint32_t                 arrayLayers;

        VK_SAMPLE_COUNT_1_BIT,                          //VkSampleCountFlagBits    samples;
        VK_IMAGE_TILING_OPTIMAL,                        //VkImageTiling            tiling;
        usage,                                          //VkImageUsageFlags        usage;
        VK_SHARING_MODE_EXCLUSIVE,                      //VkSharingMode            sharingMode;

        (uint32_t)queueFamilies.size(),                 //uint32_t                 queueFamilyIndexCount;
        queueFamilies.data(),                           //const uint32_t*          pQueueFamilyIndices;

        VK_IMAGE_LAYOUT_UNDEFINED,                      //VkImageLayout            initialLayout;
    };

    //Create image.
    vkCreateImage(device->logical, &imageInfo, NULL, &handle);


    //Get image memory requirements.
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device->logical, handle, &memoryRequirements);

    //Get memory type index.
    uint32_t memoryTypeIdx = 0;
    VkMemoryPropertyFlags memoryType = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    device->getMemoryTypeIndex(memoryTypeIdx, memoryType, memoryRequirements);

    //Fill image memory allocate info.
    VkMemoryAllocateInfo memoryInfo =
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, //VkStructureType    sType;
        NULL,                                   //const void*        pNext;

        memoryRequirements.size,                //VkDeviceSize       allocationSize;
        memoryTypeIdx                           //uint32_t           memoryTypeIndex;
    };

    //Allocate image memory.
    vkAllocateMemory(device->logical, &memoryInfo, NULL, &memory);

    //Bind memory to image.
    vkBindImageMemory(device->logical, handle, memory, 0);


    //Fill fence create info.
    VkFenceCreateInfo fenceInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    //VkStructureType           sType;
        NULL,                                   //const void*               pNext;
        0                                       //VkFenceCreateFlags        flags;
    };

    //Create fence.
    VkFence fence;
    vkCreateFence(device->logical, &fenceInfo, NULL, &fence);

    VkQueue activeQueue = device->queueFamilies[ACTIVE_FAMILY].queues[0];
    VkCommandBuffer commandBuffer = device->queueFamilies[ACTIVE_FAMILY].commandBuffers[0];

    //Fill commmand buffer begin info.
    VkCommandBufferBeginInfo commandBeginInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,    //VkStructureType                          sType;
        NULL,                                           //const void*                              pNext;
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,    //VkCommandBufferUsageFlags                flags;

        NULL                                            //const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    };

    //Begin command recording.
    vkBeginCommandBuffer(commandBuffer, &commandBeginInfo);

    //Change image layout to optimal.
    changeLayout(VK_IMAGE_LAYOUT_UNDEFINED, layout, commandBuffer);

    //Stop command recording.
    vkEndCommandBuffer(commandBuffer);

    //Fill queue submit info.
    VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo =
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,      //VkStructureType                sType;
        NULL,                               //const void*                    pNext;

        0,                                  //uint32_t                       waitSemaphoreCount;
        NULL,                               //const VkSemaphore*             pWaitSemaphores;

        &stageMask,                         //const VkPipelineStageFlags*    pWaitDstStageMask;

        1,                                  //uint32_t                       commandBufferCount;
        &commandBuffer,                     //const VkCommandBuffer*         pCommandBuffers;

        0,                                  //uint32_t                       signalSemaphoreCount;
        NULL                                //const VkSemaphore*             pSignalSemaphores;
    };

    //Submit queue.
    vkQueueSubmit(activeQueue, 1, &submitInfo, fence);

    vkWaitForFences(device->logical, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->logical, 1, &fence);

    vkResetCommandBuffer(commandBuffer, 0);

    //Destroy the fence.
    vkDestroyFence(device->logical, fence, NULL);
}


/**
 * Creates a view for the image.
 */
void VkcImage::createView()
{
    //Fill image view info.
    VkImageViewCreateInfo viewInfo =
    {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,   //VkStructureType            sType;
        NULL,                                       //const void*                pNext;
        0,                                          //VkImageViewCreateFlags     flags;

        handle,                                     //VkImage                    image;
        (VkImageViewType)type,                      //VkImageViewType            viewType;
        format,                                     //VkFormat                   format;
        {  },                                       //VkComponentMapping         components;
        resourceRange                               //VkImageSubresourceRange    subresourceRange;
    };

    //Create image view.
    vkCreateImageView(logicalDevice, &viewInfo, NULL, &view);
}


/**
 * Creates a sampler for the image.
 */
void VkcImage::createSampler()
{
    //Fill sampler info.
    VkSamplerCreateInfo samplerInfo =
    {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,      //VkStructureType         sType;
        NULL,                                       //const void*             pNext;
        0,                                          //VkSamplerCreateFlags    flags;

        VK_FILTER_LINEAR,                           //VkFilter                magFilter;
        VK_FILTER_LINEAR,                           //VkFilter                minFilter;
        VK_SAMPLER_MIPMAP_MODE_LINEAR,              //VkSamplerMipmapMode     mipmapMode;

        VK_SAMPLER_ADDRESS_MODE_REPEAT,             //VkSamplerAddressMode    addressModeU;
        VK_SAMPLER_ADDRESS_MODE_REPEAT,             //VkSamplerAddressMode    addressModeV;
        VK_SAMPLER_ADDRESS_MODE_REPEAT,             //VkSamplerAddressMode    addressModeW;
        0.0f,                                       //float                   mipLodBias;

        VK_TRUE,                                    //VkBool32                anisotropyEnable;
        8,                                          //float                   maxAnisotropy;

        VK_FALSE,                                   //VkBool32                compareEnable;
        VK_COMPARE_OP_NEVER,                        //VkCompareOp             compareOp;

        0.0f,                                       //float                   minLod;
        5.0f,                                       //float                   maxLod;

        VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,    //VkBorderColor           borderColor;
        VK_FALSE,                                   //VkBool32                unnormalizedCoordinates;
    };

    //Create sampler.
    vkCreateSampler(logicalDevice, &samplerInfo, NULL, &sampler);
}


/**
 * Registers the commands to change image layout.
 */
void VkcImage::changeLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer)
{
    //Setup access masks.
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;

    getAccessMask(srcAccessMask, oldLayout);
    getAccessMask(dstAccessMask, newLayout);

    //Setup image barrier.
    VkImageMemoryBarrier imageBarrier =
    {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, //VkStructureType            sType;
        NULL,                                   //const void*                pNext;

        srcAccessMask,                          //VkAccessFlags              srcAccessMask;
        dstAccessMask,                          //VkAccessFlags              dstAccessMask;

        oldLayout,                              //VkImageLayout              oldLayout;
        newLayout,                              //VkImageLayout              newLayout;

        VK_QUEUE_FAMILY_IGNORED,                //uint32_t                   srcQueueFamilyIndex;
        VK_QUEUE_FAMILY_IGNORED,                //uint32_t                   dstQueueFamilyIndex;

        handle,                                 //VkImage                    image;
        resourceRange                           //VkImageSubresourceRange    subresourceRange;
    };

    //Register barrier in command buffer.
    vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                0,
                0, NULL,
                0, NULL,
                1, &imageBarrier
                );
}


/**
 * Load data into the image buffer.
 */
void VkcImage::loadData(QImage uiImage)
{
    ///@todo
    (void)uiImage;
}


/**
 * Get the access mask specific to the image layout.
 */
void VkcImage::getAccessMask(VkAccessFlags &accessMask, VkImageLayout layout)
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
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                VK_ACCESS_MEMORY_READ_BIT;
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
