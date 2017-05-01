#include "vkc_context.h"


/**
 * Initialize with empty fields.
 */
VkcContext::VkcContext()
{
    surface =       VK_NULL_HANDLE;
    instance =      VK_NULL_HANDLE;
}


/**
 * Create the context.
 */
VkcContext::VkcContext(uint32_t id, const VkcDevice *device, const VkInstance instance) : VkcContext()
{
    this->instance =    instance;
    this->device =      device;

    createSurface(id);
    getCommandChains();

    swapchain = new VkcSwapchain(surface, device);
    pipeline = new VkcPipeline(swapchain, device);
}


/**
 * Destroy the context.
 */
VkcContext::~VkcContext()
{
    if (pipeline != nullptr)
        delete pipeline;

    if (swapchain != nullptr)
        delete swapchain;

    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, surface, nullptr);
}


/**
 * Creates the surface on which Vulkan will draw to.
 */
void VkcContext::createSurface(uint64_t id)
{
    // Fill surface info.
    VkWin32SurfaceCreateInfoKHR surfaceInfo =
    {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,    // VkStructureType                 sType;
        nullptr,                                            // const void*                     pNext;
        0,                                                  // VkWin32SurfaceCreateFlagsKHR    flags;

        nullptr,                                            // HINSTANCE                       hinstance;
        (HWND)id                                            // HWND                            hwnd;
    };

    // Create surface.
    vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
}


/**
 * Get the queue and command buffer pairs compatible with the surface.
 */
void VkcContext::getCommandChains()
{
    for (int i = 0; i < device->queueFamilies.size(); i++)
    {
        VkcQueueFamily family = device->queueFamilies[i];

        VkBool32 ok;
        vkGetPhysicalDeviceSurfaceSupportKHR(device->physical, family.index, surface, &ok);

        if (ok)
            for (int j = 0; j < family.queues.size(); j++)
                commandChain.append({family.queues[j], family.commandBuffers[j]});
    }
}


/**
 * Resize the swapchain.
 */
void VkcContext::resize()
{
    if (swapchain != nullptr)
    {
        // Destroy swapchain.
        VkRenderPass oldRenderPass = swapchain->renderPass;
        swapchain->renderPass = VK_NULL_HANDLE;
        delete swapchain;

        // Recreate swapchain using old render pass.
        swapchain = new VkcSwapchain(oldRenderPass, surface, device);
    }
}
