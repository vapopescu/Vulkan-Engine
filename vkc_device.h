#ifndef VKC_DEVICE_H
#define VKC_DEVICE_H

#include <QVector>
#if DEBUG == 1
#include <QDebug>
#endif

#include <vulkan.h>

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
    //Objects:
public:
    VkPhysicalDevice                    physical;
    VkDevice                            logical;

    QVector<VkcQueueFamily>             queueFamilies;

    VkPhysicalDeviceProperties          properties;
    VkPhysicalDeviceFeatures            features;
    VkPhysicalDeviceMemoryProperties    memoryProperties;

    //Functions
public:
    VkcDevice();
    VkcDevice(
            VkPhysicalDevice            physicalDevice,
            VkSurfaceKHR                surface
            );
    ~VkcDevice();

    void getQueueFamilies(
            QVector<uint32_t>           &queueFamilies
            ) const;
    void getMemoryTypeIndex(
            uint32_t                    &typeIdx,
            VkMemoryPropertyFlags       propertyMask,
            VkMemoryRequirements        requirements
            ) const;
};

#endif // VKC_DEVICE_H
