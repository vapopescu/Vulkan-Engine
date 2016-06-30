#include "vkc_instance.h"


/**
 * Initialize the vulkan context.
 */
VkcInstance::VkcInstance(QWidget *parent) : QObject(parent)
{
    createInstance();
    createSurface((uint32_t)parent->winId());
    getDevices();

    swapchain = new VkcSwapchain(surface, devices[ACTIVE_DEVICE]);
    pipeline = new VkcPipeline(swapchain, devices[ACTIVE_DEVICE]);

    square = new VkcEntity(devices[ACTIVE_DEVICE]);

    width =     parent->width();
    height =    parent->height();

    camera = new VkcCamera();
    camera->setProjectionMatrix(3.14159 / 2, (float)width / (float)height, 1, 100);

    setupRender(devices[ACTIVE_DEVICE]);
}


/**
 * Destroy the Vulkan context.
 */
VkcInstance::~VkcInstance()
{
    if (square != NULL)
        delete square;

    if (camera != NULL)
        delete camera;

    unsetupRender(devices[ACTIVE_DEVICE]);

    if (pipeline != NULL)
    delete pipeline;

    if (swapchain != NULL)
    delete swapchain;

    for (int i = 0; i < devices.size(); i++)
        delete devices[i];

    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, surface, NULL);

#if DEBUG == 1
    if (debugReport != VK_NULL_HANDLE)
        pfDestroyDebugReportCallbackEXT(instance, debugReport, NULL);
#endif

    if (instance != NULL)
        vkDestroyInstance(instance, NULL);
}


/**
 * Call render (@todo on all contexts).
 */
void VkcInstance::render()
{
    render(devices[ACTIVE_DEVICE], swapchain);
}


/**
 * Print the property list of all physical devices.
 */
void VkcInstance::printDevices(QFile *file)
{
    file->open(QIODevice::WriteOnly);

    for (int i = 0; i < devices.size(); i++)
    {
        VkPhysicalDeviceProperties devProp = devices[i]->properties;

        //Interpret version data.
        struct
        {
            uint32_t a, b, c;
        } api, drv;

        api.a = VK_VERSION_MAJOR(devProp.apiVersion);
        api.b = VK_VERSION_MINOR(devProp.apiVersion);
        api.c = VK_VERSION_PATCH(devProp.apiVersion);

        drv.a = VK_VERSION_MAJOR(devProp.driverVersion);
        drv.b = VK_VERSION_MINOR(devProp.driverVersion) >> 2;

        file->write(QString("Device Name:       %1\r\n").arg(devProp.deviceName).toStdString().data());
        file->write(QString("Device Type:       %1\r\n").arg((int) devProp.deviceType).toStdString().data());
        file->write(QString("Driver Version:    %1.%2\r\n").arg(drv.a).arg(drv.b).toStdString().data());
        file->write(QString("API Version:       %1.%2.%3\r\n\r\n").arg(api.a).arg(api.b).arg(api.c).toStdString().data());
    }

    file->close();
}


/**
  * Used by the Vulkan debug layer.
  */
#if DEBUG == 1
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

    (void)objectType;
    (void)object;
    (void)location;
    (void)messageCode;
    (void)pUserData;

    return VK_FALSE;
}
#endif


/**
 * Defines application and instance parameters and creates the Vulkan instance.
 */
void VkcInstance::createInstance()
{
    //Fill application info.
    VkApplicationInfo applicationInfo =
    {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,     //VkStructureType    sType;
        NULL,                                   //const void*        pNext;

        "Vulkan Engine",                        //const char*        pApplicationName;
        VK_MAKE_VERSION(0, 10, 0),              //uint32_t           applicationVersion;

        "vkEngine",                             //const char*        pEngineName;
        VK_MAKE_VERSION(0, 10, 0),              //uint32_t           engineVersion;

        VK_MAKE_VERSION(1, 0, 8)                //uint32_t           apiVersion;
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
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,     //VkStructureType             sType;
        pDebugReportInfo,                           //const void*                 pNext;
        0,                                          //VkInstanceCreateFlags       flags;

        &applicationInfo,                           //const VkApplicationInfo*    pApplicationInfo;

        (uint32_t)instanceLayers.size(),            //uint32_t                    enabledLayerCount;
        instanceLayers.data(),                      //const char* const*          ppEnabledLayerNames;

        (uint32_t)instanceExtentions.size(),        //uint32_t                    enabledExtensionCount;
        instanceExtentions.data(),                  //const char* const*          ppEnabledExtensionNames;
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
void VkcInstance::createSurface(uint32_t id)
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
void VkcInstance::getDevices()
{
    //Get the number of devices.
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

    //Get the device list.
    QVector<VkPhysicalDevice> physicalDevices, vector;
    physicalDevices.resize(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    for (int i = 0; i < physicalDevices.size(); i++)
    {
        //Get properties.
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            vector.prepend(physicalDevices[i]);

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            vector.append(physicalDevices[i]);
    }

    physicalDevices.swap(vector);

    for (int i = 0; i < physicalDevices.size(); i++)
    {
        VkcDevice *device = new VkcDevice(physicalDevices[i], surface);
        devices.append(device);
    }
}


/**
 * Create render utility objects.
 */
void VkcInstance::setupRender(const VkcDevice *device)
{
    //Create uniform buffer.
    uniformBuffer = new VkcBuffer(16 * sizeof(float), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, device);

    //Create present buffer.
    presentBuffer = new VkcBuffer(width * height * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT, device);

    //Fill semaphore create info.
    VkSemaphoreCreateInfo semaphoreInfo =
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,    //VkStructureType           sType;
        NULL,                                       //const void*               pNext;
        0                                           //VkSemaphoreCreateFlags    flags;
    };

    //Create semaphores.
    vkCreateSemaphore(device->logical, &semaphoreInfo, NULL, &sphAcquire);
    vkCreateSemaphore(device->logical, &semaphoreInfo, NULL, &sphRender);


    //Fill fence create info.
    VkFenceCreateInfo fenceInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    //VkStructureType           sType;
        NULL,                                   //const void*               pNext;
        0                                       //VkFenceCreateFlags        flags;
    };

    //Create fence.
    vkCreateFence(device->logical, &fenceInfo, NULL, &fence);


    //Fill uniform buffer info.
    VkDescriptorBufferInfo uniformBufferInfo =
    {
        uniformBuffer->handle,      //VkBuffer        buffer;
        0,                          //VkDeviceSize    offset;
        VK_WHOLE_SIZE               //VkDeviceSize    range;
    };

    //Fill write descriptor set info.
    VkWriteDescriptorSet writeSet =
    {

        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,     //VkStructureType                  sType;
        NULL,                                       //const void*                      pNext;

        pipeline->descriptorSets[0],                //VkDescriptorSet                  dstSet;
        0,                                          //uint32_t                         dstBinding;
        0,                                          //uint32_t                         dstArrayElement;
        1,                                          //uint32_t                         descriptorCount;
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          //VkDescriptorType                 descriptorType;

        NULL,                                       //const VkDescriptorImageInfo*     pImageInfo;
        &uniformBufferInfo,                         //const VkDescriptorBufferInfo*    pBufferInfo;
        NULL                                        //const VkBufferView*              pTexelBufferView;

    };

    //Update descriptor set.
    vkUpdateDescriptorSets(device->logical, 1, &writeSet, 0, NULL);
}


/**
 * Destroy render utility objects.
 */
void VkcInstance::unsetupRender(const VkcDevice *device)
{
    //Destroy uniform buffer.
    if (uniformBuffer != NULL)
        delete uniformBuffer;

    //Destroy present buffer.
    if (presentBuffer != NULL)
        delete presentBuffer;

    //Destroy semaphores.
    vkDestroySemaphore(device->logical, sphAcquire, NULL);
    vkDestroySemaphore(device->logical, sphRender, NULL);

    //Destroy fence.
    vkDestroyFence(device->logical, fence, NULL);
}


/**
 * Display objects to surface.
 */
void VkcInstance::render(const VkcDevice *device, const VkcSwapchain *swapchain)
{
    //Get the queue and command buffer.
    ///@todo For this thread.
    VkQueue activeQueue = device->queueFamilies[ACTIVE_FAMILY].queues[0];
    VkCommandBuffer commandBuffer = device->queueFamilies[ACTIVE_FAMILY].commandBuffers[0];

    //Get the next image available.
    uint32_t nextImageIdx;
    VkResult result = vkAcquireNextImageKHR(device->logical, swapchain->handle, UINT64_MAX, sphAcquire, VK_NULL_HANDLE, &nextImageIdx);
    VkcImage *nextImage = swapchain->colorImages[nextImageIdx];


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
    nextImage->changeLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);


    //Fill render pass begin info.
    VkRenderPassBeginInfo renderPassBeginInfo =
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,       //VkStructureType        sType;
        NULL,                                           //const void*            pNext;

        swapchain->renderPass,                          //VkRenderPass           renderPass;
        swapchain->frameBuffers[nextImageIdx],          //VkFramebuffer          framebuffer;
        {0, 0, width, height},                          //VkRect2D               renderArea;
        2,                                              //uint32_t               clearValueCount;
        swapchain->clearValues                          //const VkClearValue*    pClearValues;
    };

    //Begin render pass.
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    //Bind the graphics pipeline.
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

    //Bind descriptor sets.
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0,
                            1, &pipeline->descriptorSets[0], 0, NULL);


    //Get the view-projection matrix.
    QMatrix4x4 vpMatrix;
    camera->getViewProjectionMatrix(vpMatrix);

    //Render our entities.
    square->render(commandBuffer, uniformBuffer, vpMatrix, device);

    //End render pass.
    vkCmdEndRenderPass(commandBuffer);


    //Change image layout to present.
    nextImage->changeLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, commandBuffer);

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

    vkWaitForFences(device->logical, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->logical, 1, &fence);

    vkResetCommandBuffer(commandBuffer, 0);


    //Fill queue present info.
    VkPresentInfoKHR presentInfo =
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, //VkStructureType          sType;
        NULL,                               //const void*              pNext;

        1,                                  //uint32_t                 waitSemaphoreCount;
        &sphRender,                         //const VkSemaphore*       pWaitSemaphores;

        1,                                  //uint32_t                 swapchainCount;
        &swapchain->handle,                 //const VkSwapchainKHR*    pSwapchains;

        &nextImageIdx,                      //const uint32_t*          pImageIndices;
        &result                             //VkResult*                pResults;
    };

    //Now present.
    vkQueuePresentKHR(activeQueue, &presentInfo);
}
