#ifndef VKC_SWAPCHAIN_H
#define VKC_SWAPCHAIN_H

#include "stable.h"
#include "vkc_device.h"
#include "mgimage.h"


/**
 * Class used for swap chains.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcSwapchain
{
    // Objects:
public:
    VkSwapchainKHR                  handle;
    VkRenderPass                    renderPass;

    QVector<MgImage*>               colorImages;
    MgImage                         depthStencilImage;

    QVector<VkFramebuffer>          framebuffers;

    VkExtent2D                      extent;
    VkClearValue                    clearValues[2];

protected:
    QVector<VkSurfaceFormatKHR>     surfaceFormats;
    int                             imageCount;
    const VkcDevice                 *device;

    // Functions:
public:
    VkcSwapchain();

    VkcSwapchain(
            VkSurfaceKHR            surface,
            const VkcDevice         *device
            );
    VkcSwapchain(
            VkRenderPass            renderPass,
            VkSurfaceKHR            surface,
            const VkcDevice         *device
            );
    ~VkcSwapchain();

protected:
    void createSwapchain(
            VkSurfaceKHR            surface,
            const VkcDevice         *device
            );
    void createImages();
    void createRenderPass();
    void createFramebuffers();
};

#endif // VKC_SWAPCHAIN_H
