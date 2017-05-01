#ifndef VKC_CONTEXT_H
#define VKC_CONTEXT_H

#include "stable.h"
#include "vkc_device.h"
#include "mgimage.h"
#include "vkc_swapchain.h"
#include "vkc_pipeline.h"


struct VkcCommandChain
{
    VkQueue                     queue;
    VkCommandBuffer             buffer;
};


/**
 * Class used as the Vulkan context.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcContext
{
    // Objects:
public:
    VkSurfaceKHR                surface;
    VkcSwapchain                *swapchain;
    VkcPipeline                 *pipeline;

    QVector<VkcCommandChain>    commandChain;

    const VkcDevice             *device;
private:
    VkInstance                  instance;


    // Functions:
public:
    VkcContext();
    VkcContext(
            uint32_t            id,
            const VkcDevice     *device,
            const VkInstance    instance
            );
    ~VkcContext();

private:
    void createSurface(uint64_t id);
    void getCommandChains();

public:
    void setupRender();
    void unsetupRender();
    void resize();
};

#endif // VKC_CONTEXT_H
