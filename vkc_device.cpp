#include "vkc_device.h"


/**
 * Initialize with empty fields.
 */
VkcDevice::VkcDevice()
{
    physical =          VK_NULL_HANDLE;
    logical =           VK_NULL_HANDLE;
}


/**
 * Create the device.
 */
VkcDevice::VkcDevice(VkPhysicalDevice physicalDevice) : VkcDevice()
{
    // Get the number of queue properties.
    uint32_t familyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);

    // Get the queue property list.
    QVector<VkQueueFamilyProperties> queueProperties;
    queueProperties.resize(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, queueProperties.data());

    // Get the queue families that support graphic and surface present commands.
    for(int i = 0; i < queueProperties.size(); i++)
        if(queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            VkcQueueFamily queueFamilyStruct;
            queueFamilyStruct.index = i;
            queueFamilyStruct.properties = queueProperties[i];

            queueFamilies.append(queueFamilyStruct);
        }


    // Get physical device properties, features and memory properties.
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);


    // For each queue family...
    QVector<VkDeviceQueueCreateInfo> queueInfos;
    QVector<float> queuePriorities;
    for (int i = 0; i < queueFamilies.size(); i++)
    {
        uint32_t queueCount = queueFamilies[i].properties.queueCount;
        uint32_t familyIdx = queueFamilies[i].index;

        // Set queue priorities.
        uint32_t offset = queuePriorities.size();
        for (uint32_t i = 0; i < queueCount; i++)
            queuePriorities.append(1.0f);

        // Fill device queue info.
        VkDeviceQueueCreateInfo queueInfo =
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // VkStructureType             sType;
            nullptr,                                    // const void*                 pNext;
            0,                                          // VkDeviceQueueCreateFlags    flags;

            familyIdx,                                  // uint32_t                    queueFamilyIndex;
            queueCount,                                 // uint32_t                    queueCount;
            &queuePriorities.at(offset)                 // const float*                pQueuePriorities;
        };

        queueInfos.append(queueInfo);
    }

    // Setup device layers and extentions.
    QVector<const char*> deviceLayers =
    {

    };

    QVector<const char*> deviceExtentions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // Fill device info.
    VkDeviceCreateInfo deviceInfo =
    {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,       // VkStructureType                    sType;
        nullptr,                                    // const void*                        pNext;
        0,                                          // VkDeviceCreateFlags                flags;

        (uint32_t)queueInfos.size(),                // uint32_t                           queueCreateInfoCount;
        queueInfos.data(),                          // const VkDeviceQueueCreateInfo*     pQueueCreateInfos;

        (uint32_t)deviceLayers.size(),              // uint32_t                           enabledLayerCount;
        deviceLayers.data(),                        // const char* const*                 ppEnabledLayerNames;

        (uint32_t)deviceExtentions.size(),          // uint32_t                           enabledExtensionCount;
        deviceExtentions.data(),                    // const char* const*                 ppEnabledExtensionNames;

        &features                                   // const VkPhysicalDeviceFeatures*    pEnabledFeatures;
    };

    // Create device.
    vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &logical);


    // For each queue family...
    for (int i = 0; i < queueFamilies.size(); i++)
    {
        uint32_t queueCount = queueFamilies[i].properties.queueCount;
        uint32_t familyIdx = queueFamilies[i].index;

        // Get queues.
        queueFamilies[i].queues.resize(queueCount);
        for (uint32_t j = 0; j < queueCount; j++)
            vkGetDeviceQueue(logical, familyIdx, j, &queueFamilies[i].queues[j]);


        // Fill command pool info.
        VkCommandPoolCreateInfo commandPoolCreateInfo =
        {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,         // VkStructureType             sType;
            nullptr,                                               // const void*                 pNext;
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,    // VkCommandPoolCreateFlags    flags;

            familyIdx                                           // uint32_t                    queueFamilyIndex;
        };

        // Create command pool.
        vkCreateCommandPool(logical, &commandPoolCreateInfo, nullptr, &queueFamilies[i].commandPool);


        // Fill command buffer allocation info.
        queueFamilies[i].commandBuffers.resize(queueCount);
        VkCommandBufferAllocateInfo commandBufferAllocateInfo =
        {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,     // VkStructureType         sType;
            nullptr,                                               // const void*             pNext;

            queueFamilies[i].commandPool,                       // VkCommandPool           commandPool;
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,                    // VkCommandBufferLevel    level;
            (uint32_t)queueFamilies[i].commandBuffers.size()    // uint32_t                commandBufferCount;
        };

        // Allocate command buffers.
        vkAllocateCommandBuffers(logical, &commandBufferAllocateInfo, queueFamilies[i].commandBuffers.data());
    }


    // Store handle to physical device.
    this->physical = physicalDevice;
}


/**
 * Destroy the device.
 */
VkcDevice::~VkcDevice()
{
    if (logical != VK_NULL_HANDLE)
    {
        while (queueFamilies.size() > 0)
        {
            QVector<VkCommandBuffer> *commandBuffers = &queueFamilies[0].commandBuffers;

            if (commandBuffers->size() > 0)
            {
                vkFreeCommandBuffers(logical, queueFamilies[0].commandPool, commandBuffers->size(), commandBuffers->data());
                commandBuffers->clear();
            }

            if (queueFamilies[0].commandPool != VK_NULL_HANDLE)
                vkDestroyCommandPool(logical, queueFamilies[0].commandPool, nullptr);

            queueFamilies.removeFirst();
        }

        vkDestroyDevice(logical, nullptr);
    }
}


/**
 * Get the queue family indices.
 */
void VkcDevice::getQueueFamilies(QVector<uint32_t> &queueFamilies) const
{
    for (int i = 0; i < this->queueFamilies.count(); i++)
        queueFamilies.append(this->queueFamilies[i].index);
}


/**
 * Get memory type index.
 */
VkResult VkcDevice::getMemoryTypeIndex(VkMemoryPropertyFlags propertyMask, VkMemoryRequirements requirements, uint32_t* pTypeIdx) const
{
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        VkMemoryType memoryType = memoryProperties.memoryTypes[i];
        if ((requirements.memoryTypeBits >> i) & 1)
        {
            if ((memoryType.propertyFlags & propertyMask) == propertyMask)
            {
                *pTypeIdx = i;
                return VK_SUCCESS;
            }
        }
    }

    return VK_ERROR_OUT_OF_DEVICE_MEMORY;
}
