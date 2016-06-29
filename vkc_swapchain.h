#ifndef VKC_SWAPCHAIN_H
#define VKC_SWAPCHAIN_H

#include <QVector>
#if DEBUG == 1
#include <QDebug>
#endif

#include <vulkan.h>

#include <vkc_device.h>
#include <vkc_image.h>


/**
 * Class used for swap chains.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcSwapchain
{
    //Objects:
public:
    VkSwapchainKHR                  handle;
    VkRenderPass                    renderPass;

    QVector<VkcColorImage*>         colorImages;
    VkcDepthImage                   *depthImage;

    QVector<VkFramebuffer>          frameBuffers;

    VkExtent2D                      extent;
    VkClearValue                    clearValues[2];

private:
    QVector<VkSurfaceFormatKHR>     surfaceFormats;
    VkDevice                        logicalDevice;

    //Functions:
public:
    VkcSwapchain();
    VkcSwapchain(
            VkSurfaceKHR            surface,
            const VkcDevice         *device
            );
    ~VkcSwapchain();
};

#endif // VKC_SWAPCHAIN_H
