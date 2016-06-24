#include "vk_context.h"


///******************************       QT Functions        ******************************///

/**
 * Initialize the vulkan context
 */
VkContext::VkContext(QObject *parent, uint32_t wId) : QObject(parent)
{
    createInstance();
    createSurface(surface, wId);
    getPhysicalDevices();

    if (DEBUG)
        printDevices();

    devices[ACTIVE_DEVICE].create(surface, 1);
    swapchain.create(surface, devices[ACTIVE_DEVICE]);
}


/**
 * Destroy the Vulkan context.
 */
VkContext::~VkContext()
{
    swapchain.destroy();

    for (int i = 0; i < devices.size(); i++)
        devices[i].destroy();

    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, surface, NULL);

    if (DEBUG && debugReport != VK_NULL_HANDLE)
        pfDestroyDebugReportCallbackEXT(instance, debugReport, NULL);

    if (instance != NULL)
        vkDestroyInstance(instance, NULL);
}

/**
  * Used by the Vulkan debug layer.
  */

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
        VkDebugReportFlagsEXT                       flags,
        VkDebugReportObjectTypeEXT                  objectType,
        uint64_t                                    object,
        size_t                                      location,
        int32_t                                     messageCode,
        const char*                                 pLayerPrefix,
        const char*                                 pMessage,
        void*                                       pUserData)
{
    QString output = "";

    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        return VK_FALSE;
      //output.append("INFO:    ");
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        output.append("WARNING: ");
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        output.append("PERF:    ");
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        output.append("ERROR:   ");
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        output.append("DEBUG:   ");

    output = output.append("[@%1]").arg(pLayerPrefix).leftJustified(24, ' ').append("- %1").arg(pMessage);

    qDebug() << output.toStdString().data();

    return VK_FALSE;
}


///******************************       Vulkan Constructors        ******************************///

/**
 * Defines application and instance parameters and creates the Vulkan instance.
 */
void VkContext::createInstance()
{
    //Fill application info.
    VkApplicationInfo applicationInfo =
    {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, //VkStructureType    sType;
        NULL,                               //const void*        pNext;

        "Vulkan Engine",                    //const char*        pApplicationName;
        VK_MAKE_VERSION(0, 10, 0),          //uint32_t           applicationVersion;

        "vkEngine",                         //const char*        pEngineName;
        VK_MAKE_VERSION(0, 10, 0),          //uint32_t           engineVersion;

        VK_MAKE_VERSION(1, 0, 8)            //uint32_t           apiVersion;
    } ;

    //Fill debug report callback info.
    VkDebugReportCallbackCreateInfoEXT  debugReportInfo =
    {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,    //VkStructureType                 sType;
        NULL,                                                       //const void*                     pNext;
        VK_DEBUG_REPORT_INFORMATION_BIT_EXT |                       //VkDebugReportFlagsEXT           flags;
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_DEBUG_BIT_EXT |
        0,

        (PFN_vkDebugReportCallbackEXT)vkDebugCallback,              //PFN_vkDebugReportCallbackEXT    pfnCallback;
        NULL                                                        //void*                           pUserData;
    };

    //Fill instance info.
    VkInstanceCreateInfo instanceInfo =
    {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,             //VkStructureType             sType;
        DEBUG ? &debugReportInfo : NULL,                    //const void*                 pNext;
        0,                                                  //VkInstanceCreateFlags       flags;

        &applicationInfo,                                   //const VkApplicationInfo*    pApplicationInfo;

        (uint32_t)VkSettings::instanceLayers.size(),        //uint32_t                    enabledLayerCount;
        VkSettings::instanceLayers.data(),                  //const char* const*          ppEnabledLayerNames;

        (uint32_t)VkSettings::instanceExtentions.size(),    //uint32_t                    enabledExtensionCount;
        VkSettings::instanceExtentions.data(),              //const char* const*          ppEnabledExtensionNames;
    };

    //Create instance.
    vkCreateInstance(&instanceInfo, NULL, &instance);

    //Get function pointers.
    GET_IPROC(instance, CreateDebugReportCallbackEXT);
    GET_IPROC(instance, DestroyDebugReportCallbackEXT);

    //Create the debug report callback.
    if (DEBUG)
        pfCreateDebugReportCallbackEXT(instance, &debugReportInfo, NULL, &debugReport);
}


/**
 * Creates the surface on which Vulkan will draw to.
 */
void VkContext::createSurface(VkSurfaceKHR &surface, uint32_t id)
{
#ifdef _WIN32
    //Fill surface info.
    VkWin32SurfaceCreateInfoKHR surfaceInfo =
    {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,    //VkStructureType                 sType;
        NULL,                                               //const void*                     pNext;
        0,                                                  //VkWin32SurfaceCreateFlagsKHR    flags;

        NULL,                                               //HINSTANCE                       hinstance;
        (HWND)id                                            //HWND                            hwnd;
    };

    //Create surface.
    vkCreateWin32SurfaceKHR(instance, &surfaceInfo, NULL, &surface);

#elif __ANDROID__
    //Fill surface info.
    VkAndroidSurfaceCreateInfoKHR surfaceInfo =
    {
        VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,  //VkStructureType                   sType;
        NULL,                                               //const void*                       pNext;
        0,                                                  //VkAndroidSurfaceCreateFlagsKHR    flags;

        (ANativeWindow*)id                                  //ANativeWindow*                    window;
    };

    //Create surface.
    vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, NULL, &surface);

#else
    //Fill surface info.
    VkXcbSurfaceCreateInfoKHR surfaceInfo =
    {
        VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,      //VkStructureType               sType;
        NULL,                                               //const void*                   pNext;
        0,                                                  //VkXcbSurfaceCreateFlagsKHR    flags;

        NULL, //QX11Info::connection(),                     //xcb_connection_t*             connection;
        (xcb_window_t)id                                    //xcb_window_t                  window;
    };

    //Create surface.
    vkCreateXcbSurfaceKHR(instance, &surfaceInfo, NULL, &surface);

#endif
}


/**
 * Create a semaphore.
 */
void VkContext::createSemaphore(VkSemaphore &semaphore, VkcDevice device)
{
    //Fill semaphore create info.
    VkSemaphoreCreateInfo semaphoreInfo =
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,    //VkStructureType           sType;
        NULL,                                       //const void*               pNext;
        0                                           //VkSemaphoreCreateFlags    flags;
    };

    //Create semaphore.
    vkCreateSemaphore(device.logical, &semaphoreInfo, NULL, &semaphore);
}


/**
 * Create a fence.
 */
void VkContext::createFence(VkFence &fence, VkcDevice device)
{
    //Fill fence create info.
    VkFenceCreateInfo fenceInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    //VkStructureType           sType;
        NULL,                                   //const void*               pNext;
        0                                       //VkFenceCreateFlags        flags;
    };

    //Create fence.
    vkCreateFence(device.logical, &fenceInfo, NULL, &fence);
}


///******************************       Vulkan Getters        ******************************///

/**
 * Gets the physical device data.
 */
void VkContext::getPhysicalDevices()
{
    //Get the number of devices.
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

    //Get the device list.
    QVector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(deviceCount);
    devices.resize(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    for (int i = 0; i < devices.size(); i++)
    {
        devices[i].destroy();
        devices[i].physical = physicalDevices[i];

        //Get properties.
        vkGetPhysicalDeviceProperties(devices[i].physical, &devices[i].properties);
        vkGetPhysicalDeviceFeatures(devices[i].physical, &devices[i].features);
        vkGetPhysicalDeviceMemoryProperties(devices[i].physical, &devices[i].memoryProperties);
    }

    //Order the devices (DISCRETE_GPU < INTEGRATED_GPU); remove other types.
    uint32_t descreteIdx = 0;
    uint32_t integratedIdx = 0;
    for (int i = 0; i < devices.size(); i++)
    {
        switch(devices[i].properties.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            if (devices[i].properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                devices.move(i, descreteIdx);
            descreteIdx++;

        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            if (devices[i].properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                devices.move(i, integratedIdx);
            integratedIdx++;

            break;
        default:
            devices.removeAt(i--);
        }
    }
}


/**
 * Get the access mask specific to the layout.
 */
void VkContext::getAccessMask(VkAccessFlags &accessMask, VkImageLayout layout)
{
    switch (layout)
    {
    case VK_IMAGE_LAYOUT_GENERAL:
        accessMask =
                VK_ACCESS_MEMORY_READ_BIT |
                VK_ACCESS_MEMORY_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        accessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        accessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        accessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                VK_ACCESS_MEMORY_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        accessMask =
                VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    default:
        accessMask = 0;
    }
}


///******************************       Vulkan Methods        ******************************///

/**
 * Changes image layout.
 */
void VkContext::changeImageLayout(VkcImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer)
{
    //Setup access masks.
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;

    getAccessMask(srcAccessMask, oldLayout);
    getAccessMask(dstAccessMask, newLayout);

    //Setup image barrier.
    VkImageMemoryBarrier imageBarrier =
    {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, //VkStructureType            sType;
        NULL,                                   //const void*                pNext;

        srcAccessMask,                          //VkAccessFlags              srcAccessMask;
        dstAccessMask,                          //VkAccessFlags              dstAccessMask;

        oldLayout,                              //VkImageLayout              oldLayout;
        newLayout,                              //VkImageLayout              newLayout;

        VK_QUEUE_FAMILY_IGNORED,                //uint32_t                   srcQueueFamilyIndex;
        VK_QUEUE_FAMILY_IGNORED,                //uint32_t                   dstQueueFamilyIndex;

        image.handle,                           //VkImage                    image;
        image.resourceRange                     //VkImageSubresourceRange    subresourceRange;
    };

    //Register barrier in command buffer.
    vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                0,
                0, NULL,
                0, NULL,
                1, &imageBarrier
                );
}


/**
 * Display objects to surface.
 */
void VkContext::render(VkcDevice device, VkcSwapchain swapchain, uint32_t queueIdx, uint32_t commandBufferIdx)
{
    //Create semaphores and a fence.
    VkSemaphore semaphore1, semaphore2;
    createSemaphore(semaphore1, device);
    createSemaphore(semaphore2, device);

    VkFence fence;
    createFence(fence, device);

    VkQueue activeQueue = device.queueFamilies[ACTIVE_FAMILY].queues[queueIdx];
    VkCommandBuffer commandBuffer = device.commandBuffers[commandBufferIdx];

    //Get the next image available.
    uint32_t nextImageIdx;
    VkResult result = vkAcquireNextImageKHR(device.logical, swapchain.handle, UINT64_MAX, semaphore1, fence, &nextImageIdx);
    VkcImage nextImage = swapchain.colorImages[nextImageIdx];

    //Fill commmand buffer begin info.
    VkCommandBufferBeginInfo beginInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,    //VkStructureType                          sType;
        NULL,                                           //const void*                              pNext;
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,    //VkCommandBufferUsageFlags                flags;

        NULL                                            //const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    };

    //Fill queue submit info.
    VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo =
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,      //VkStructureType                sType;
        NULL,                               //const void*                    pNext;

        1,                                  //uint32_t                       waitSemaphoreCount;
        &semaphore1,                        //const VkSemaphore*             pWaitSemaphores;

        &stageMask,                         //const VkPipelineStageFlags*    pWaitDstStageMask;

        1,                                  //uint32_t                       commandBufferCount;
        &commandBuffer,                     //const VkCommandBuffer*         pCommandBuffers;

        1,                                  //uint32_t                       signalSemaphoreCount;
        &semaphore2                         //const VkSemaphore*             pSignalSemaphores;
    };

    //Fill queue present info.
    VkPresentInfoKHR presentInfo =
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, //VkStructureType          sType;
        NULL,                               //const void*              pNext;

        1,                                  //uint32_t                 waitSemaphoreCount;
        &semaphore2,                        //const VkSemaphore*       pWaitSemaphores;

        1,                                  //uint32_t                 swapchainCount;
        &swapchain.handle,                  //const VkSwapchainKHR*    pSwapchains;

        &nextImageIdx,                      //const uint32_t*          pImageIndices;
        &result                             //VkResult*                pResults;
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    changeImageLayout(nextImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);

    vkCmdClearColorImage(commandBuffer, nextImage.handle, VK_IMAGE_LAYOUT_GENERAL, &VkSettings::clearColor, 1, &nextImage.resourceRange);

    changeImageLayout(nextImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    vkQueueSubmit(activeQueue, 1, &submitInfo, fence);

    vkWaitForFences(device.logical, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device.logical, 1, &fence);

    vkResetCommandBuffer(commandBuffer, 0);

    //Now present.
    vkQueuePresentKHR(activeQueue, &presentInfo);

    //Destroy semaphores and the fence.
    vkDestroySemaphore(device.logical, semaphore1, NULL);
    vkDestroySemaphore(device.logical, semaphore2, NULL);
    vkDestroyFence(device.logical, fence, NULL);
}

/**
 * Stot that calls the render function.
 */
void VkContext::render()
{
    render(devices[ACTIVE_DEVICE], swapchain, 0, 0);
}


/**
 * Slot that prints the property list of all physical devices.
 */
void VkContext::printDevices()
{
    //Pruint32_t properties for each device.
    QFile file("devices.txt");
    file.open(QIODevice::WriteOnly);

    for (int i = 0; i < devices.size(); i++)
    {
        VkPhysicalDeviceProperties devProp = devices[i].properties;

        //Interpret version data.
        struct
        {
            uint32_t a, b, c;
        } v;

        v.a = VK_VERSION_MAJOR(devProp.apiVersion);
        v.b = VK_VERSION_MINOR(devProp.apiVersion);
        v.c = VK_VERSION_PATCH(devProp.apiVersion);

        file.write(QString("Driver Version: %1\r\n").arg(devProp.driverVersion).toStdString().data());
        file.write(QString("Device Name:    %1\r\n").arg(devProp.deviceName).toStdString().data());
        file.write(QString("Device Type:    %1\r\n").arg((int) devProp.deviceType).toStdString().data());
        file.write(QString("API Version:    %1.%2.%3\r\n\r\n").arg(v.a).arg(v.b).arg(v.c).toStdString().data());
    }
}
