#ifndef VKC_SWAPCHAIN_H
#define VKC_SWAPCHAIN_H

#include <QVector>

#include <vk_settings.h>
#include <vulkan.h>

#include <vkc_device.h>
#include <vkc_image.h>


/**
 * Class used for swap chains.
 */
class VkcSwapchain
{
    //Objects:
public:
    VkSwapchainKHR                  handle =                VK_NULL_HANDLE;
    VkRenderPass                    renderPass =            VK_NULL_HANDLE;

    QVector<VkcImage>               colorImages =           {};
    VkcImage                        depthImage =            {};

    QVector<VkFramebuffer>          frameBuffers =          {};

    VkSurfaceCapabilitiesKHR        surfaceCapabilities =   {};

    QVector<VkSurfaceFormatKHR>     surfaceFormats =        {};
    QVector<VkPresentModeKHR>       presentModes =          {};

private:
    VkDevice                        logicalDevice =         VK_NULL_HANDLE;

    //Functions:
public:
    VkcSwapchain();
    VkcSwapchain(
            VkSurfaceKHR    surface,
            VkcDevice       device
            );
    ~VkcSwapchain();

    void create(
            VkSurfaceKHR    surface,
            VkcDevice       device
            );
    void destroy();
};

#endif // VKC_SWAPCHAIN_H
