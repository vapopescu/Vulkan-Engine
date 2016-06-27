#ifndef VK_CONTEXT_H
#define VK_CONTEXT_H

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 1

#elif __ANDROID__
#define VK_USE_PLATFORM_ANDROID_KHR 1

#else
#define VK_USE_PLATFORM_XCB_KHR 1

#endif

#define PROC(NAME) PFN_vk##NAME pf##NAME = NULL
#define GET_IPROC(INSTANCE, NAME) pf##NAME = (PFN_vk##NAME)vkGetInstanceProcAddr(INSTANCE, "vk" #NAME)
#define GET_DPROC(INSTANCE, NAME) pf##NAME = (PFN_vk##NAME)vkGetDeviceProcAddr(INSTANCE, "vk" #NAME)

#include <QObject>
#include <QFile>
#include <QMatrix4x4>

#include <vk_utils.h>
#include <vulkan.h>
#include <vk_layer.h>

#include <vkc_device.h>
#include <vkc_image.h>
#include <vkc_swapchain.h>
#include <vkc_pipeline.h>

#include <vkc_camera.h>
#include <vkc_buffer.h>
#include <vkc_entity.h>

#define ACTIVE_DEVICE 0

/**
 * Class used as the Vulkan context.
 */
class VkContext : public QObject
{
    //Qt components:
    Q_OBJECT

public:
    explicit VkContext(QObject *parent = 0, uint32_t wId = 0);
    ~VkContext();

public slots:
    void render();
    void printDevices();

    //Vulkan components:
    //Objects:
private:
    //Primary
    VkInstance                  instance =          VK_NULL_HANDLE;
    VkSurfaceKHR                surface =           VK_NULL_HANDLE;
    QVector<VkcDevice>          devices =           {};
    VkcSwapchain                swapchain =         {};
    VkcPipeline                 pipeline =          {};

    //Secondary
    VkcCamera                   camera =            {};
    VkcBuffer                   uniformBuffer =     {};

    //Utils
    VkSemaphore                 sphAcquire =        VK_NULL_HANDLE;
    VkSemaphore                 sphRender =         VK_NULL_HANDLE;
    VkFence                     fence =             VK_NULL_HANDLE;

    //Entities
    VkcEntity                   square =          {};

    //Debug
    VkDebugReportCallbackEXT    debugReport =       VK_NULL_HANDLE;

    //Fetched functions
#if DEBUG == 1
    PROC(CreateDebugReportCallbackEXT);
    PROC(DestroyDebugReportCallbackEXT);
#endif


    //Functions:
private:
    void createInstance();
    void createSurface(
            VkSurfaceKHR        &surface,
            uint32_t            id
            );

    void getPhysicalDevices();

    void setupRender(
            VkcDevice           device
            );
    void unsetupRender(
            VkcDevice           device
            );
    void render(
            VkcDevice           device,
            VkcSwapchain        swapchain,
            uint32_t            queueIdx,
            uint32_t            commandBufferIdx
            );
};

#endif // VK_CONTEXT_H
