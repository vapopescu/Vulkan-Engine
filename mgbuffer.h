#ifndef MGBUFFER_H
#define MGBUFFER_H

#include "stable.h"
#include "vkc_device.h"


/**
 * Class used for buffers.
 */
class MgBuffer
{
    // Objects:
public:
    VkBuffer                    handle =    VK_NULL_HANDLE;
    VkDeviceMemory              memory =    VK_NULL_HANDLE;

private:
    const VkcDevice             *device =   nullptr;

    // Functions:
public:
    VkResult create(
            VkDeviceSize        size,
            VkBufferUsageFlags  usageMask,
            const VkcDevice     *device
            );
    void destroy();
};

#endif // MGBUFFER_H
