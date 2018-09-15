#include "mgbuffer.h"

/**
 * Create the buffer.
 */
VkResult MgBuffer::create(VkDeviceSize size, VkBufferUsageFlags usageMask, const VkcDevice *device)
{
    this->device = device;

    // Get queue families.
    QVector<uint32_t> queueFamilies;
    device->getQueueFamilies(queueFamilies);

    // Fill buffer info.
    VkBufferCreateInfo bufferInfo =
    {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,           // VkStructureType        sType;
        nullptr,                                        // const void*            pNext;
        0,                                              // VkBufferCreateFlags    flags;

        size,                                           // VkDeviceSize           size;
        usageMask,                                      // VkBufferUsageFlags     usage;

        VK_SHARING_MODE_EXCLUSIVE,                      // VkSharingMode          sharingMode;

        static_cast<uint32_t>(queueFamilies.count()),   // uint32_t               queueFamilyIndexCount;
        queueFamilies.data()                            // const uint32_t*        pQueueFamilyIndices;
    };

    MG_ASSERT(vkCreateBuffer(device->logical, &bufferInfo, nullptr, &handle));

    // Get buffer memory requirements.
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device->logical, handle, &memoryRequirements);

    uint32_t memoryTypeIdx = 0;
    VkMemoryPropertyFlags memoryMask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    MG_ASSERT(device->getMemoryTypeIndex(memoryMask, memoryRequirements, &memoryTypeIdx));

    // Fill buffer memory allocate info.
    VkMemoryAllocateInfo memoryInfo =
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,     // VkStructureType    sType;
        nullptr,                                    // const void*        pNext;

        memoryRequirements.size,                    // VkDeviceSize       allocationSize;
        memoryTypeIdx                               // uint32_t           memoryTypeIndex;
    };

    // Allocate buffer memory.
    MG_ASSERT(vkAllocateMemory(device->logical, &memoryInfo, nullptr, &memory));

    // Bind memory to buffer.
    MG_ASSERT(vkBindBufferMemory(device->logical, handle, memory, 0));

    return VK_SUCCESS;
}

/**
 * Destroy the buffer.
 */
void MgBuffer::destroy()
{
    if (device != nullptr)
    {
        if (memory != VK_NULL_HANDLE)
            vkFreeMemory(device->logical, memory, nullptr);

        if (handle != VK_NULL_HANDLE)
            vkDestroyBuffer(device->logical, handle, nullptr);
    }
}
