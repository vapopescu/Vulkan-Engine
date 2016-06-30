#ifndef VKC_BUFFER_H
#define VKC_BUFFER_H

#if DEBUG == 1
#include <QDebug>
#endif

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
    VkBuffer                    handle;
    VkDeviceMemory              memory;

private:
    VkDevice                    logicalDevice;

    //Functions:
public:
    VkcBuffer();
    VkcBuffer(
            VkDeviceSize        size,
            VkBufferUsageFlags  usageMask,
            const VkcDevice     *device
            );

    ~VkcBuffer();
};

#endif // VKC_BUFFER_H
