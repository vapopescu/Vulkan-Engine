#ifndef VKC_SWAPCHAIN_H
#define VKC_SWAPCHAIN_H

#include <QVector>

#include <vulkan.h>
#include <vk_utils.h>

#include <vkc_device.h>
#include <vkc_image.h>
#include <vkc_pipeline.h>


/**
 * Class used for swap chains.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
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
