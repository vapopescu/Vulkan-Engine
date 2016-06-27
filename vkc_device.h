#ifndef VKC_DEVICE_H
#define VKC_DEVICE_H

#include <QVector>

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
    VkPhysicalDevice                    physical =          VK_NULL_HANDLE;
    VkDevice                            logical =           VK_NULL_HANDLE;
    VkCommandPool                       commandPool =       VK_NULL_HANDLE;

    QVector<VkcQueueFamily>             queueFamilies =     {};
    QVector<VkCommandBuffer>            commandBuffers =    {};

    VkPhysicalDeviceProperties          properties =        {};
    VkPhysicalDeviceFeatures            features =          {};
    VkPhysicalDeviceMemoryProperties    memoryProperties =  {};

    //Functions
public:
    VkcDevice();
    VkcDevice(
            uint32_t                commandBufferNumber,
            VkSurfaceKHR            surface
            );
    ~VkcDevice();

    void create(
            uint32_t                commandBufferNumber,
            VkSurfaceKHR            surface
            );
    void destroy();

    void getQueueFamilies(
            QVector<uint32_t>       &queueFamilies
            );
    void getMemoryTypeIndex(
            uint32_t                &memoryTypeIdx,
            VkMemoryPropertyFlags   memoryMask,
            VkMemoryRequirements    memoryRequirements
            );
};

#endif // VKC_DEVICE_H
