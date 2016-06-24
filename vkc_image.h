#ifndef VKC_IMAGE_H
#define VKC_IMAGE_H

#include <vk_settings.h>
#include <vulkan.h>

#include <vkc_device.h>


/**
 * Class used for image handling.
 */
class VkcImage
{
    //Objects:
public:
    VkImage                         handle =                VK_NULL_HANDLE;
    VkImageView                     view =                  VK_NULL_HANDLE;
    VkDeviceMemory                  memory =                VK_NULL_HANDLE;

    VkImageType                     type =                  VK_IMAGE_TYPE_2D;
    VkFormat                        format =                VK_FORMAT_UNDEFINED;
    VkImageSubresourceRange         resourceRange =         {};

private:
    VkDevice                        logicalDevice =         VK_NULL_HANDLE;

    //Functions:
public:
    VkcImage();
    VkcImage(
            VkImageType     type,
            VkFormat        format,
            VkcDevice       logicalDevice
            );
    ~VkcImage();

    void create(
            VkImageType     type,
            VkFormat        format,
            VkcDevice       device
            );
    void destroy();

    void createView(
            VkcDevice       device
            );

    void bindMemory(
            VkcDevice       device
            );
};

#endif // VKC_IMAGE_H
