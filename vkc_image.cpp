#include "vkc_image.h"


/**
 * Initialize with empty fields.
 */
VkcImage::VkcImage()
{
    handle =            VK_NULL_HANDLE;
    view =              VK_NULL_HANDLE;
    memory =            VK_NULL_HANDLE;

    type =              VK_IMAGE_TYPE_2D;
    format =            VK_FORMAT_UNDEFINED;
    resourceRange =     {};

    logicalDevice =     VK_NULL_HANDLE;
}


/**
 * Initialize and create image.
 */
VkcImage::VkcImage(VkImageType type, VkExtent3D extent, VkFormat format, VkcDevice device)
{
    VkcImage();
    create(type, extent, format, device);
}


/**
 * We will handle the cleanup ourselves.
 */
VkcImage::~VkcImage()
{
    //destroy();
}


/**
 * Create an image with given type and format.
 */
void VkcImage::create(VkImageType type, VkExtent3D extent, VkFormat format, VkcDevice device)
{
    //Set flags according to format
    VkImageUsageFlags   usageMask =     0;
    VkImageAspectFlags  aspectMask =    0;
    VkImageLayout       layout  =       VK_IMAGE_LAYOUT_UNDEFINED;

    if (format >= VK_FORMAT_R4G4_UNORM_PACK8 && format <= VK_FORMAT_B10G11R11_UFLOAT_PACK32)
    {
        usageMask =     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        aspectMask =    VK_IMAGE_ASPECT_COLOR_BIT;
        layout  =       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    if (format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        usageMask =     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        aspectMask =    VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        layout  =       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    //Get queue families.
    QVector<uint32_t> queueFamilies;
    device.getQueueFamilies(queueFamilies);

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
        usageMask,                                      //VkImageUsageFlags        usage;
        VK_SHARING_MODE_EXCLUSIVE,                      //VkSharingMode            sharingMode;

        (uint32_t)queueFamilies.size(),                 //uint32_t                 queueFamilyIndexCount;
        queueFamilies.data(),                           //const uint32_t*          pQueueFamilyIndices;

        VK_IMAGE_LAYOUT_UNDEFINED,                      //VkImageLayout            initialLayout;
    };

    //Create image.
    vkCreateImage(device.logical, &imageInfo, NULL, &handle);

    //Setup data fields.
    this->type = type;
    this->format = format;
    this->logicalDevice = device.logical;

    resourceRange = {
        aspectMask,     //VkImageAspectFlags    aspectMask;
        0,              //uint32_t              baseMipLevel;
        1,              //uint32_t              levelCount;
        0,              //uint32_t              baseArrayLayer;
        1,              //uint32_t              layerCount;
    };

    //Get image memory requirements.
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device.logical, handle, &memoryRequirements);

    //Get memory type index.
    uint32_t memoryTypeIdx = 0;
    VkMemoryPropertyFlags memoryMask = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    device.getMemoryTypeIndex(memoryTypeIdx, memoryMask, memoryRequirements);

    //Fill image memory allocate info.
    VkMemoryAllocateInfo memoryInfo =
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, //VkStructureType    sType;
        NULL,                                   //const void*        pNext;

        memoryRequirements.size,                //VkDeviceSize       allocationSize;
        memoryTypeIdx                           //uint32_t           memoryTypeIndex;
    };

    //Allocate image memory.
    vkAllocateMemory(device.logical, &memoryInfo, NULL, &memory);

    //Bind memory to image.
    vkBindImageMemory(device.logical, handle, memory, 0);

    //Create image view.
    this->createView(device);

    //Fill fence create info.
    VkFenceCreateInfo fenceInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    //VkStructureType           sType;
        NULL,                                   //const void*               pNext;
        0                                       //VkFenceCreateFlags        flags;
    };

    //Create fence.
    VkFence fence;
    vkCreateFence(device.logical, &fenceInfo, NULL, &fence);

    VkQueue activeQueue = device.queueFamilies[ACTIVE_FAMILY].queues[0];
    VkCommandBuffer commandBuffer = device.commandBuffers[0];

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

    vkWaitForFences(device.logical, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device.logical, 1, &fence);

    vkResetCommandBuffer(commandBuffer, 0);

    //Destroy the fence.
    vkDestroyFence(device.logical, fence, NULL);
}


/**
 * Destroy the image.
 */
void VkcImage::destroy()
{
    if (logicalDevice != VK_NULL_HANDLE)
    {

        if (view != VK_NULL_HANDLE)
        {
            vkDestroyImageView(logicalDevice, view, NULL);
            view = VK_NULL_HANDLE;
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
 * Creates a view for the image.
 */
void VkcImage::createView(VkcDevice device)
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

    //Destroy old image view (if existent).
    if (view != VK_NULL_HANDLE)
        vkDestroyImageView(device.logical, view, NULL);

    //Create image view.
    vkCreateImageView(device.logical, &viewInfo, NULL, &view);
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

    default:
        accessMask = 0;
    }
}
