#ifndef VKC_BUFFER_H
#define VKC_BUFFER_H

#include <vulkan.h>

#include <vkc_device.h>


/**
 * Class used for buffers.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcBuffer
{
    //Objects:
public:
    VkBuffer            handle =            VK_NULL_HANDLE;
    VkDeviceMemory      memory =            VK_NULL_HANDLE;

private:
    VkDevice            logicalDevice =     VK_NULL_HANDLE;

    //Functions:
public:
    VkcBuffer();
    VkcBuffer(
            VkDeviceSize            size,
            VkBufferUsageFlags      usageMask,
            VkcDevice               device
            );

    ~VkcBuffer();

    void create(
            VkDeviceSize            size,
            VkBufferUsageFlags      usageMask,
            VkcDevice               device
            );
    void destroy();
};

#endif // VKC_BUFFER_H
