#ifndef VK_CONTEXT_H
#define VK_CONTEXT_H

#include <QObject>
#include <QDebug>
#include <QFile>

#include <vk_settings.h>
#include <vulkan.h>
#include <vk_layer.h>

#include <vkc_device.h>
#include <vkc_image.h>
#include <vkc_swapchain.h>


/**
 * Class used as the Vulkan context.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".)
 */
class VkContext : public QObject
{
    //Qt components
    Q_OBJECT

public:
    explicit VkContext(QObject *parent = 0, uint32_t wId = 0);
    ~VkContext();

public slots:
    void render();
    void printDevices();

    //Vulkan components
private:
    //Objects:
    //Core
    VkInstance                  instance =      VK_NULL_HANDLE;
    VkSurfaceKHR                surface =       VK_NULL_HANDLE;
    QVector<VkcDevice>          devices =       {};
    VkcSwapchain                swapchain =     {};

    //Debug
    VkDebugReportCallbackEXT    debugReport =   VK_NULL_HANDLE;

    //Fetched functions
    PROC(CreateDebugReportCallbackEXT);
    PROC(DestroyDebugReportCallbackEXT);


    //Functions:
    //Creators
    void createInstance();
    void createSurface(
            VkSurfaceKHR        &surface,
            uint32_t            id
            );

    void createSemaphore(
            VkSemaphore         &semaphore,
            VkcDevice           device
            );
    void createFence(
            VkFence             &fence,
            VkcDevice           device
            );

    //Getters
    void getPhysicalDevices();
    void getQueueFamilies(
            QVector<uint32_t>   &queueFamilies,
            VkcDevice           device
            );
    void getAccessMask(
            VkAccessFlags       &accessMask,
            VkImageLayout       layout
            );

    //Methods
    void changeImageLayout(
            VkcImage            image,
            VkImageLayout       oldLayout,
            VkImageLayout       newLayout,
            VkCommandBuffer     commandBuffer
            );
    void render(
            VkcDevice           device,
            VkcSwapchain        swapchain,
            uint32_t            queueIdx,
            uint32_t            commandBufferIdx
            );
};

#endif // VK_CONTEXT_H
