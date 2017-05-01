#ifndef VKC_DEVICE_H
#define VKC_DEVICE_H

#include "stable.h"

#define ACTIVE_FAMILY 0


/**
 * Struct used for device queues.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
struct VkcQueueFamily
{
    uint32_t                            index =             UINT32_MAX;
    VkCommandPool                       commandPool =       VK_NULL_HANDLE;

    QVector<VkQueue>                    queues =            {};
    QVector<VkCommandBuffer>            commandBuffers =    {};

    VkQueueFamilyProperties             properties =        {};
};


/**
 * Class used for devices.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcDevice
{
    // Objects:
public:
    VkPhysicalDevice                    physical;
    VkDevice                            logical;

    QVector<VkcQueueFamily>             queueFamilies;

    VkPhysicalDeviceProperties          properties;
    VkPhysicalDeviceFeatures            features;
    VkPhysicalDeviceMemoryProperties    memoryProperties;

    // Functions
public:
    VkcDevice();
    VkcDevice(
            VkPhysicalDevice            physicalDevice
            );
    ~VkcDevice();

    void getQueueFamilies(
            QVector<uint32_t>           &queueFamilies
            ) const;
    VkResult getMemoryTypeIndex(
            VkMemoryPropertyFlags       propertyMask,
            VkMemoryRequirements        requirements,
            uint32_t*                   pTypeIdx
            ) const;
};

#endif // VKC_DEVICE_H
