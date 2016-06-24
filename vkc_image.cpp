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

    logicalDevice =            VK_NULL_HANDLE;
}


/**
 * Initialize and create image.
 */
VkcImage::VkcImage(VkImageType type, VkFormat format, VkcDevice device)
{
    VkcImage();
    create(type, format, device);
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
void VkcImage::create(VkImageType type, VkFormat format, VkcDevice device)
{
    //Set flags according to format
    VkImageUsageFlags   usageMask =     0;
    VkImageAspectFlags  aspectMask =    0;

    if (format >= VK_FORMAT_R4G4_UNORM_PACK8 && format <= VK_FORMAT_B10G11R11_UFLOAT_PACK32)
    {
        usageMask =     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        aspectMask =    VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        usageMask =     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        aspectMask =    VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    //Get queue families.
    QVector<uint32_t> queueFamilies;
    for (int i = 0; i < device.queueFamilies.count(); i++)
        queueFamilies.append(device.queueFamilies[i].index);

    //Fill image create info.
    VkImageCreateInfo imageInfo =
    {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,            //VkStructureType          sType;
        NULL,                                           //const void*              pNext;
        0,                                              //VkImageCreateFlags       flags;

        type,                                           //VkImageType              imageType;
        format,                                         //VkFormat                 format;

        {WIDTH, HEIGHT, 1},                             //VkExtent3D               extent;
        1,                                              //uint32_t                 mipLevels;
        1,                                              //uint32_t                 arrayLayers;

        SAMPLE_COUNT,                                   //VkSampleCountFlagBits    samples;
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

    //Bind image to memory.
    this->bindMemory(device);

    //Create image view.
    this->createView(device);
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
            vkUnmapMemory(logicalDevice, memory);
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
 * Allocate memory and bind the image to it.
 */
void VkcImage::bindMemory(VkcDevice device)
{
    //Get image memory requirements.
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device.logical, handle, &memoryRequirements);

    uint32_t memoryTypeIndex = 0;
    VkMemoryPropertyFlags desiredMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    for (uint32_t i = 0; i < device.memoryProperties.memoryTypeCount; i++)
    {
        VkMemoryType memoryType = device.memoryProperties.memoryTypes[i];
        if ((memoryRequirements.memoryTypeBits >> i) & 1)
        {
            if ((memoryType.propertyFlags & desiredMemoryFlags) == desiredMemoryFlags)
            {
                memoryTypeIndex = i;
                break;
            }
        }
    }

    //Fill image memory allocate info.
    VkMemoryAllocateInfo memoryInfo =
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, //VkStructureType    sType;
        NULL,                                   //const void*        pNext;

        memoryRequirements.size,                //VkDeviceSize       allocationSize;
        memoryTypeIndex                         //uint32_t           memoryTypeIndex;
    };

    //Allocate image memory.
    vkAllocateMemory(device.logical, &memoryInfo, NULL, &memory);

    //Bind memory to image.
    vkBindImageMemory(device.logical, handle, memory, 0);
}
