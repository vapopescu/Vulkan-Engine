#include "vkc_buffer.h"


/**
 * Initialize with empty fields.
 */
VkcBuffer::VkcBuffer()
{
    handle =        VK_NULL_HANDLE;
    memory =        VK_NULL_HANDLE;
}


/**
 * Create the buffer.
 */
VkcBuffer::VkcBuffer(VkDeviceSize size, VkBufferUsageFlags usageMask, const VkcDevice *device) : VkcBuffer()
{
    //Get queue families.
    QVector<uint32_t> queueFamilies;
    device->getQueueFamilies(queueFamilies);

    //Fill buffer info.
    VkBufferCreateInfo bufferInfo =
    {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,   //VkStructureType        sType;
        NULL,                                   //const void*            pNext;
        0,                                      //VkBufferCreateFlags    flags;

        size,                                   //VkDeviceSize           size;
        usageMask,                              //VkBufferUsageFlags     usage;

        VK_SHARING_MODE_EXCLUSIVE,              //VkSharingMode          sharingMode;

        (uint32_t)queueFamilies.count(),        //uint32_t               queueFamilyIndexCount;
        queueFamilies.data()                    //const uint32_t*        pQueueFamilyIndices;
    };

    vkCreateBuffer(device->logical, &bufferInfo, NULL, &handle);

    //Get buffer memory requirements.
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device->logical, handle, &memoryRequirements);

    uint32_t memoryTypeIdx = 0;
    VkMemoryPropertyFlags memoryMask = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    device->getMemoryTypeIndex(memoryTypeIdx, memoryMask, memoryRequirements);

    //Fill buffer memory allocate info.
    VkMemoryAllocateInfo memoryInfo =
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,     //VkStructureType    sType;
        NULL,                                       //const void*        pNext;

        memoryRequirements.size,                    //VkDeviceSize       allocationSize;
        memoryTypeIdx                               //uint32_t           memoryTypeIndex;
    };

    //Allocate buffer memory.
    vkAllocateMemory(device->logical, &memoryInfo, NULL, &memory);

    //Bind memory to buffer.
    vkBindBufferMemory(device->logical, handle, memory, 0);

    this->logicalDevice = device->logical;
}


/**
 * Destroy the buffer.
 */
VkcBuffer::~VkcBuffer()
{
    if (logicalDevice != VK_NULL_HANDLE)
    {
        if (memory != VK_NULL_HANDLE)
            vkFreeMemory(logicalDevice, memory, NULL);

        if (handle != VK_NULL_HANDLE)
            vkDestroyBuffer(logicalDevice, handle, NULL);
    }
}
