#ifndef VKC_DEVICE_H
#define VKC_DEVICE_H

#include <QVector>

#include <vk_settings.h>
#include <vulkan.h>


/**
 * Struct used for device queues.
 */
struct VkcQueueFamily
{
    uint32_t                            index =             UINT32_MAX;
    QVector<VkQueue>                    queues =            {};

    VkQueueFamilyProperties             properties =        {};
};


/**
 * Class used for devices.
 */
class VkcDevice
{
    //Objects:
public:
    VkPhysicalDevice                    physical;
    VkDevice                            logical;
    VkCommandPool                       commandPool;

    QVector<VkcQueueFamily>             queueFamilies;
    QVector<VkCommandBuffer>            commandBuffers;

    VkPhysicalDeviceProperties          properties;
    VkPhysicalDeviceFeatures            features;
    VkPhysicalDeviceMemoryProperties    memoryProperties;

    //Functions
public:
    VkcDevice();
    VkcDevice(
            uint32_t        commandBufferNumber,
            VkSurfaceKHR    surface
            );
    ~VkcDevice();

    void create(
            uint32_t        commandBufferNumber,
            VkSurfaceKHR    surface
            );
    void destroy();
};

#endif // VKC_DEVICE_H
