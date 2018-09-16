#include "vkc_instance.h"

#define NV_VERSION_MAJOR(version) (static_cast<uint32_t>(version) >> 22)
#define NV_VERSION_MINOR(version) ((static_cast<uint32_t>(version) >> 14) & 0xff)
#define NV_VERSION_PATCH(version) (static_cast<uint32_t>(version) & 0x3ff)

#define AMD_VERSION_MAJOR(version) (static_cast<uint32_t>(version) >> 18)
#define AMD_VERSION_MINOR(version) ((static_cast<uint32_t>(version) >> 12) & 0x3f)
#define AMD_VERSION_PATCH(version) (static_cast<uint32_t>(version) & 0x1ff)

#define GAMMA 2.2f

/**
 * Initialize the vulkan context.
 */
VkcInstance::VkcInstance(QWidget *parent) : QObject(parent)
{
    createInstance();
    getDevices();

    context = new VkcContext(static_cast<uint32_t>(parent->winId()), devices[0], instance);

    sphere.create(devices[0], "sphere");

    width =     static_cast<uint32_t>(parent->width());
    height =    static_cast<uint32_t>(parent->height());

    camera.setProjectionMatrix(3.1416f / 2, static_cast<float>(width) / static_cast<float>(height), 1, 100);

    setupRender(devices[0]);

    timer.start();
}


/**
 * Destroy the Vulkan context.
 */
VkcInstance::~VkcInstance()
{
    sphere.destroy();

    unsetupRender(devices[0]);

    if (context != nullptr)
        delete context;

    for (int i = 0; i < devices.size(); i++)
        delete devices[i];

#ifdef QT_DEBUG
    if (debugReport != VK_NULL_HANDLE)
        pfDestroyDebugReportCallbackEXT(instance, debugReport, nullptr);
#endif

    if (instance != nullptr)
        vkDestroyInstance(instance, nullptr);
}


/**
  * Used by the Vulkan debug layer.
  */
#ifdef QT_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL mgDebugCallback(
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

    output = output.append("[@%1]").arg(pLayerPrefix).leftJustified(32, ' ').append("- %1").arg(pMessage);

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
    // Fill application info.
    VkApplicationInfo applicationInfo =
    {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,             // VkStructureType    sType;
        nullptr,                                        // const void*        pNext;

        "Magma Engine Demo",                            // const char*        pApplicationName;
        VK_MAKE_VERSION(0, 10, 0),                      // uint32_t           applicationVersion;

        "Magma Engine",                                 // const char*        pEngineName;
        VK_MAKE_VERSION(0, 10, 0),                      // uint32_t           engineVersion;

        VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION)        // uint32_t           apiVersion;
    } ;

    // Setup instance layers and extentions.
    static QVector<const char*> instanceLayers =
    {
        "VK_LAYER_LUNARG_standard_validation"
    };

    static QVector<const char*> instanceExtentions =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,

#ifdef QT_DEBUG
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
    };

#ifdef QT_DEBUG
    // Fill debug report callback info.
    VkDebugReportCallbackCreateInfoEXT  debugReportInfo =
    {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,        // VkStructureType                 sType;
        nullptr,                                                        // const void*                     pNext;
        0 |                                                             // VkDebugReportFlagsEXT           flags;
        // VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_DEBUG_BIT_EXT |
        0,

        static_cast<PFN_vkDebugReportCallbackEXT>(mgDebugCallback),     // PFN_vkDebugReportCallbackEXT    pfnCallback;
        nullptr                                                         // void*                           pUserData;
    };

    void *pDebugReportInfo = &debugReportInfo;
#else
    void *pDebugReportInfo = nullptr;
#endif

    // Fill instance info.
    VkInstanceCreateInfo instanceInfo =
    {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,                 // VkStructureType             sType;
        pDebugReportInfo,                                       // const void*                 pNext;
        0,                                                      // VkInstanceCreateFlags       flags;

        &applicationInfo,                                       // const VkApplicationInfo*    pApplicationInfo;

        static_cast<uint32_t>(instanceLayers.size()),           // uint32_t                    enabledLayerCount;
        instanceLayers.data(),                                  // const char* const*          ppEnabledLayerNames;

        static_cast<uint32_t>(instanceExtentions.size()),       // uint32_t                    enabledExtensionCount;
        instanceExtentions.data(),                              // const char* const*          ppEnabledExtensionNames;
    };

    // Create instance.
    vkCreateInstance(&instanceInfo, nullptr, &instance);

#ifdef QT_DEBUG
    // Get function pointers.
    GET_IPROC(instance, CreateDebugReportCallbackEXT);
    GET_IPROC(instance, DestroyDebugReportCallbackEXT);

    // Create the debug report callback.
    pfCreateDebugReportCallbackEXT(instance, &debugReportInfo, nullptr, &debugReport);
#endif
}


/**
 * Gets the physical device data.
 */
void VkcInstance::getDevices()
{
    // Get the number of devices.
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    // Get the device list.
    QVector<VkPhysicalDevice> physicalDevices, vector;
    physicalDevices.resize(static_cast<int>(deviceCount));
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    for (int i = 0; i < physicalDevices.size(); i++)
    {
        // Get properties.
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
        VkcDevice *device = new VkcDevice(physicalDevices[i]);
        devices.append(device);
    }
}


/**
 * Display objects on screen.
 */
void VkcInstance::render()
{
    // Calculate time delta from last frame in seconds.
    float delta = timer.nsecsElapsed() * 1e-9f;
    timer.start();

    // Get the queue and command buffer.
    // TODO For this thread.
    const VkcDevice         *device =           context->device;
    const VkcSwapchain      *swapchain =        context->swapchain;
    const VkcPipeline       *pipeline =         context->pipeline;
    VkQueue                 activeQueue =       context->commandChain[0].queue;
    VkCommandBuffer         commandBuffer =     context->commandChain[0].buffer;

    // Get the next image available.
    int nextImageIdx;
    VkResult result = vkAcquireNextImageKHR(device->logical, swapchain->handle,
            UINT64_MAX, sphAcquire, VK_NULL_HANDLE, reinterpret_cast<uint32_t *>(&nextImageIdx));
    // TODO Solve VK_ERROR_OUT_OF_DATE_KHR when resizing.
    MgImage *nextImage = swapchain->colorImages[nextImageIdx];


    // Fill commmand buffer begin info.
    VkCommandBufferBeginInfo commandBeginInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,    // VkStructureType                          sType;
        nullptr,                                        // const void*                              pNext;
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,    // VkCommandBufferUsageFlags                flags;

        nullptr                                         // const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    };

    // Begin command recording.
    vkBeginCommandBuffer(commandBuffer, &commandBeginInfo);

    // Change image layout to color attachment optimal.
    nextImage->changeLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);

    // Fill render pass begin info.
    VkRenderPassBeginInfo renderPassBeginInfo =
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,       // VkStructureType        sType;
        nullptr,                                        // const void*            pNext;

        swapchain->renderPass,                          // VkRenderPass           renderPass;
        swapchain->framebuffers[nextImageIdx],          // VkFramebuffer          framebuffer;
        {                                               // VkRect2D               renderArea;
            {                                               // VkOffset2D    offset;
                0,                                              // int32_t    x;
                0                                               // int32_t    y;
            },
            {                                               // VkExtent2D    extent;
                static_cast<uint32_t>(width),                   // uint32_t    width;
                static_cast<uint32_t>(height)                   // uint32_t    height;
            }
        },
        2,                                              // uint32_t               clearValueCount;
        swapchain->clearValues                          // const VkClearValue*    pClearValues;
    };

    // Begin render pass.
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind the graphics pipeline.
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

    // Resolve dynamic states.
    VkViewport viewport =
    {
        0.0f,                           // float    x;
        0.0f,                           // float    y;
        static_cast<float>(width),      // float    width;
        static_cast<float>(height),     // float    height;
        0.0f,                           // float    minDepth;
        1.0f                            // float    maxDepth;
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor =
    {
        {0, 0},             // VkOffset2D    offset;
        {width, height}     // VkExtent2D    extent;
    };

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Bind gamma value. // TODO Settings class
    float gamma = GAMMA;
    context->pipeline->bindFloatv(5, &gamma, 1);

    // Get the view-projection matrix.
    QMatrix4x4 vpMatrix;
    camera.getViewProjectionMatrix(&vpMatrix);

    // Render our entities.
    sphere.render(context, commandBuffer, vpMatrix, delta);

    // End render pass.
    vkCmdEndRenderPass(commandBuffer);

    // Change image layout to present.
    nextImage->changeLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, commandBuffer);

    // Stop command recording.
    vkEndCommandBuffer(commandBuffer);

    // Fill queue submit info.
    VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo =
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,      // VkStructureType                sType;
        nullptr,                            // const void*                    pNext;

        1,                                  // uint32_t                       waitSemaphoreCount;
        &sphAcquire,                        // const VkSemaphore*             pWaitSemaphores;

        &stageMask,                         // const VkPipelineStageFlags*    pWaitDstStageMask;

        1,                                  // uint32_t                       commandBufferCount;
        &commandBuffer,                     // const VkCommandBuffer*         pCommandBuffers;

        1,                                  // uint32_t                       signalSemaphoreCount;
        &sphRender                          // const VkSemaphore*             pSignalSemaphores;
    };

    // Submit queue.
    vkQueueSubmit(activeQueue, 1, &submitInfo, fence);

    vkWaitForFences(device->logical, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->logical, 1, &fence);

    vkResetCommandBuffer(commandBuffer, 0);

    // Fill queue present info.
    VkPresentInfoKHR presentInfo =
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                 // VkStructureType          sType;
        nullptr,                                            // const void*              pNext;

        1,                                                  // uint32_t                 waitSemaphoreCount;
        &sphRender,                                         // const VkSemaphore*       pWaitSemaphores;

        1,                                                  // uint32_t                 swapchainCount;
        &swapchain->handle,                                 // const VkSwapchainKHR*    pSwapchains;

        reinterpret_cast<uint32_t *>(&nextImageIdx),        // const uint32_t*          pImageIndices;
        &result                                             // VkResult*                pResults;
    };

    // Now present.
    vkQueuePresentKHR(activeQueue, &presentInfo);
}


/**
 * Recreate the swapchain and pipeline to fit window.
 */
void VkcInstance::resize()
{
    QWidget *parent = static_cast<QWidget*>(this->parent());
    if (width != static_cast<uint32_t>(parent->width()) || height != static_cast<uint32_t>(parent->height()))
    {
        // Update resolution fields.
        width = static_cast<uint32_t>(parent->width());
        height = static_cast<uint32_t>(parent->height());

        // Resize context.
        context->resize();

        // Update projection matrix.
        camera.setProjectionMatrix(3.1416f / 2, static_cast<float>(width) / static_cast<float>(height), 1, 100);
    }
}


/**
 * Pruint32_t the property list of all physical devices.
 */
void VkcInstance::printDevices(QFile *file)
{
    file->open(QIODevice::WriteOnly);

    for (int i = 0; i < devices.size(); i++)
    {
        VkPhysicalDeviceProperties devProp = devices[i]->properties;

        // Interpret version data.
        struct
        {
            uint32_t a, b, c;
        } vk, drv, nv, amd;

        // Interpret device type.
        QString deviceType;
        switch (devProp.deviceType) {
        default:
            deviceType = "Other";
            break;
        case 1:
            deviceType = "Integrated GPU";
            break;
        case 2:
            deviceType = "Discrete GPU";
            break;
        case 3:
            deviceType = "Virtual GPU";
            break;
        case 4:
            deviceType = "CPU";
            break;
        }

        vk.a = VK_VERSION_MAJOR(devProp.apiVersion);
        vk.b = VK_VERSION_MINOR(devProp.apiVersion);
        vk.c = VK_VERSION_PATCH(devProp.apiVersion);

        drv.a = VK_VERSION_MAJOR(devProp.driverVersion);
        drv.b = VK_VERSION_MINOR(devProp.driverVersion);
        drv.c = VK_VERSION_PATCH(devProp.driverVersion);

        nv.a = NV_VERSION_MAJOR(devProp.driverVersion);
        nv.b = NV_VERSION_MINOR(devProp.driverVersion);
        nv.c = NV_VERSION_PATCH(devProp.driverVersion);

        amd.a = AMD_VERSION_MAJOR(devProp.driverVersion);
        amd.b = AMD_VERSION_MINOR(devProp.driverVersion);
        amd.c = AMD_VERSION_PATCH(devProp.driverVersion);

        file->write(QString("Device Name:           %1\r\n").arg(devProp.deviceName).toStdString().data());
        file->write(QString("Device Type:           %1\r\n").arg(deviceType).toStdString().data());
        file->write(QString("Driver Version:        %1.%2.%3\r\n").arg(drv.a).arg(drv.b).arg(drv.c).toStdString().data());
        file->write(QString("   nVidia GeForce:     %1.%2.%3\r\n").arg(nv.a).arg(nv.b).arg(nv.c).toStdString().data());
        file->write(QString("   AMD Radeon:         %1.%2.%3\r\n").arg(amd.a).arg(amd.b).arg(amd.c).toStdString().data());
        file->write(QString("API Version:           %1.%2.%3\r\n\r\n").arg(vk.a).arg(vk.b).arg(vk.c).toStdString().data());
    }

    file->close();
}


/**
 * Create render utility objects.
 */
void VkcInstance::setupRender(const VkcDevice *device)
{
    // Create present buffer.
    presentBuffer.create(width * height * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT, device);

    // Fill semaphore create info.
    VkSemaphoreCreateInfo semaphoreInfo =
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,    // VkStructureType           sType;
        nullptr,                                    // const void*               pNext;
        0                                           // VkSemaphoreCreateFlags    flags;
    };

    // Create semaphores.
    vkCreateSemaphore(device->logical, &semaphoreInfo, nullptr, &sphAcquire);
    vkCreateSemaphore(device->logical, &semaphoreInfo, nullptr, &sphRender);

    // Fill fence create info.
    VkFenceCreateInfo fenceInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,        // VkStructureType           sType;
        nullptr,                                    // const void*               pNext;
        0                                           // VkFenceCreateFlags        flags;
    };

    // Create fence.
    vkCreateFence(device->logical, &fenceInfo, nullptr, &fence);
}


/**
 * Destroy render utility objects.
 */
void VkcInstance::unsetupRender(const VkcDevice *device)
{
    // Destroy present buffer.
    presentBuffer.destroy();

    // Destroy semaphores.
    vkDestroySemaphore(device->logical, sphAcquire, nullptr);
    vkDestroySemaphore(device->logical, sphRender, nullptr);

    // Destroy fence.
    vkDestroyFence(device->logical, fence, nullptr);
}
