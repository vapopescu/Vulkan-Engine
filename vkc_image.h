#ifndef VKC_IMAGE_H
#define VKC_IMAGE_H

#include <vulkan.h>

#include <vkc_device.h>


/**
 * Class used for image handling.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
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
            VkExtent3D      extent,
            VkFormat        format,
            VkcDevice       logicalDevice
            );
    ~VkcImage();

    void create(
            VkImageType     type,
            VkExtent3D      extent,
            VkFormat        format,
            VkcDevice       device
            );
    void destroy();

    void createView(
            VkcDevice       device
            );
    void changeLayout(
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            VkCommandBuffer commandBuffer
            );

private:
    void getAccessMask(
            VkAccessFlags &accessMask,
            VkImageLayout layout
            );
};

#endif // VKC_IMAGE_H
