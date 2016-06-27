#include "vk_context.h"


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

    devices[ACTIVE_DEVICE].create(1, surface);
    swapchain.create(surface, devices[ACTIVE_DEVICE]);
    pipeline.create(swapchain.renderPass, devices[ACTIVE_DEVICE]);

    square.create(devices[ACTIVE_DEVICE]);

    camera.setProjectionMatrix(3.14159 / 2, (float)WIDTH / HEIGHT, 1, 100);

    setupRender(devices[ACTIVE_DEVICE]);
}


/**
 * Destroy the Vulkan context.
 */
VkContext::~VkContext()
{
    square.destroy();

    unsetupRender(devices[ACTIVE_DEVICE]);

    pipeline.destroy();
    swapchain.destroy();

    for (int i = 0; i < devices.size(); i++)
        devices[i].destroy();

    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, surface, NULL);

#if DEBUG == 1
    if (debugReport != VK_NULL_HANDLE)
        pfDestroyDebugReportCallbackEXT(instance, debugReport, NULL);
#endif

    if (instance != NULL)
        vkDestroyInstance(instance, NULL);
}

#if DEBUG == 1
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
        output.append("INFO:    ");
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
#endif


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

    //Setup instance layers and extentions.
    static QVector<const char*> instanceLayers =
    {
        #if DEBUG == 1
        "VK_LAYER_LUNARG_standard_validation"
        #endif
    };

    static QVector<const char*> instanceExtentions =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,

        #if DEBUG == 1
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
        #endif

        #ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME

        #elif __ANDROID__
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME

        #else
        VK_KHR_XCB_SURFACE_EXTENSION_NAME;
        #endif
    };

#if DEBUG == 1
    //Fill debug report callback info.
    VkDebugReportCallbackCreateInfoEXT  debugReportInfo =
    {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,    //VkStructureType                 sType;
        NULL,                                                       //const void*                     pNext;
        0 |                                                         //VkDebugReportFlagsEXT           flags;
        //VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        //VK_DEBUG_REPORT_DEBUG_BIT_EXT |
        0,

        (PFN_vkDebugReportCallbackEXT)vkDebugCallback,              //PFN_vkDebugReportCallbackEXT    pfnCallback;
        NULL                                                        //void*                           pUserData;
    };

    void *pDebugReportInfo = &debugReportInfo;
#else
    void *pDebugReportInfo = NULL;
#endif

    //Fill instance info.
    VkInstanceCreateInfo instanceInfo =
    {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,             //VkStructureType             sType;
        pDebugReportInfo,                                   //const void*                 pNext;
        0,                                                  //VkInstanceCreateFlags       flags;

        &applicationInfo,                                   //const VkApplicationInfo*    pApplicationInfo;

        (uint32_t)instanceLayers.size(),        //uint32_t                    enabledLayerCount;
        instanceLayers.data(),                  //const char* const*          ppEnabledLayerNames;

        (uint32_t)instanceExtentions.size(),    //uint32_t                    enabledExtensionCount;
        instanceExtentions.data(),              //const char* const*          ppEnabledExtensionNames;
    };

    //Create instance.
    vkCreateInstance(&instanceInfo, NULL, &instance);

#if DEBUG == 1
    //Get function pointers.
    GET_IPROC(instance, CreateDebugReportCallbackEXT);
    GET_IPROC(instance, DestroyDebugReportCallbackEXT);

    //Create the debug report callback.
    pfCreateDebugReportCallbackEXT(instance, &debugReportInfo, NULL, &debugReport);
#endif

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
 * Create render utility objects.
 */
void VkContext::setupRender(VkcDevice device)
{
    //Create uniform buffer.
    uniformBuffer.create(16 * sizeof(float), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, device);


    //Fill semaphore create info.
    VkSemaphoreCreateInfo semaphoreInfo =
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,    //VkStructureType           sType;
        NULL,                                       //const void*               pNext;
        0                                           //VkSemaphoreCreateFlags    flags;
    };

    //Create semaphores.
    vkCreateSemaphore(device.logical, &semaphoreInfo, NULL, &sphAcquire);
    vkCreateSemaphore(device.logical, &semaphoreInfo, NULL, &sphRender);


    //Fill fence create info.
    VkFenceCreateInfo fenceInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    //VkStructureType           sType;
        NULL,                                   //const void*               pNext;
        0                                       //VkFenceCreateFlags        flags;
    };

    //Create fence.
    vkCreateFence(device.logical, &fenceInfo, NULL, &fence);


    //Fill descriptor buffer info.
    VkDescriptorBufferInfo uniformBufferInfo =
    {
        uniformBuffer.handle,   //VkBuffer        buffer;
        0,                      //VkDeviceSize    offset;
        VK_WHOLE_SIZE           //VkDeviceSize    range;
    };

    //Fill write descriptor set info.
    VkWriteDescriptorSet writeSet =
    {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                            //VkStructureType                  sType;
        NULL,                               //const void*                      pNext;

        pipeline.descriptorSets[0],         //VkDescriptorSet                  dstSet;
        0,                                  //uint32_t                         dstBinding;
        0,                                  //uint32_t                         dstArrayElement;
        1,                                  //uint32_t                         descriptorCount;
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  //VkDescriptorType                 descriptorType;

        NULL,                               //const VkDescriptorImageInfo*     pImageInfo;
        &uniformBufferInfo,                 //const VkDescriptorBufferInfo*    pBufferInfo;
        NULL                                //const VkBufferView*              pTexelBufferView;
    };

    //Update descriptor set.
    vkUpdateDescriptorSets(device.logical, 1, &writeSet, 0, NULL);
}


/**
 * Destroy render utility objects.
 */
void VkContext::unsetupRender(VkcDevice device)
{
    //Destroy uniform buffer.
    uniformBuffer.destroy();

    //Destroy semaphores.
    vkDestroySemaphore(device.logical, sphAcquire, NULL);
    vkDestroySemaphore(device.logical, sphRender, NULL);

    //Destroy fence.
    vkDestroyFence(device.logical, fence, NULL);
}


/**
 * Display objects to surface.
 */
void VkContext::render(VkcDevice device, VkcSwapchain swapchain, uint32_t queueIdx, uint32_t commandBufferIdx)
{
    //Get the queue and command buffer (for this thread <- to implement).
    VkQueue activeQueue = device.queueFamilies[ACTIVE_FAMILY].queues[queueIdx];
    VkCommandBuffer commandBuffer = device.commandBuffers[commandBufferIdx];

    //Get the next image available.
    uint32_t nextImageIdx;
    VkResult result = vkAcquireNextImageKHR(device.logical, swapchain.handle, UINT64_MAX, sphAcquire, fence, &nextImageIdx);
    VkcImage nextImage = swapchain.colorImages[nextImageIdx];


    //Fill commmand buffer begin info.
    VkCommandBufferBeginInfo commandBeginInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,    //VkStructureType                          sType;
        NULL,                                           //const void*                              pNext;
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,    //VkCommandBufferUsageFlags                flags;

        NULL                                            //const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    };

    //Begin command recording.
    vkBeginCommandBuffer(commandBuffer, &commandBeginInfo);

    //Change image layout to color attachment optimal.
    nextImage.changeLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);


    //Fill render pass begin info.
    VkClearValue clearValue [2] = {CLEAR_COLOR, {1.0f, 0.0f}};
    VkRenderPassBeginInfo renderPassBeginInfo =
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,       //VkStructureType        sType;
        NULL,                                           //const void*            pNext;

        swapchain.renderPass,                           //VkRenderPass           renderPass;
        swapchain.frameBuffers[nextImageIdx],           //VkFramebuffer          framebuffer;
        {0, 0, WIDTH, HEIGHT},                          //VkRect2D               renderArea;
        2,                                              //uint32_t               clearValueCount;
        clearValue                                      //const VkClearValue*    pClearValues;
    };

    //Begin render pass.
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    //Bind the graphics pipeline.
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);

    //Bind descriptor sets.
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0,
                            1, &pipeline.descriptorSets[0], 0, NULL);


    //Get the view-projection matrix.
    QMatrix4x4 vpMatrix;
    camera.getViewProjectionMatrix(vpMatrix);

    //Render our entities.
    square.render(commandBuffer, uniformBuffer, vpMatrix, device);

    //End render pass.
    vkCmdEndRenderPass(commandBuffer);


    //Change image layout to present.
    nextImage.changeLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, commandBuffer);

    //Stop command recording.
    vkEndCommandBuffer(commandBuffer);


    //Fill queue submit info.
    VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo =
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,      //VkStructureType                sType;
        NULL,                               //const void*                    pNext;

        1,                                  //uint32_t                       waitSemaphoreCount;
        &sphAcquire,                        //const VkSemaphore*             pWaitSemaphores;

        &stageMask,                         //const VkPipelineStageFlags*    pWaitDstStageMask;

        1,                                  //uint32_t                       commandBufferCount;
        &commandBuffer,                     //const VkCommandBuffer*         pCommandBuffers;

        1,                                  //uint32_t                       signalSemaphoreCount;
        &sphRender                          //const VkSemaphore*             pSignalSemaphores;
    };

    //Submit queue.
    vkQueueSubmit(activeQueue, 1, &submitInfo, fence);

    vkWaitForFences(device.logical, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device.logical, 1, &fence);

    vkResetCommandBuffer(commandBuffer, 0);


    //Fill queue present info.
    VkPresentInfoKHR presentInfo =
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, //VkStructureType          sType;
        NULL,                               //const void*              pNext;

        1,                                  //uint32_t                 waitSemaphoreCount;
        &sphRender,                         //const VkSemaphore*       pWaitSemaphores;

        1,                                  //uint32_t                 swapchainCount;
        &swapchain.handle,                  //const VkSwapchainKHR*    pSwapchains;

        &nextImageIdx,                      //const uint32_t*          pImageIndices;
        &result                             //VkResult*                pResults;
    };

    //Now present.
    vkQueuePresentKHR(activeQueue, &presentInfo);
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
