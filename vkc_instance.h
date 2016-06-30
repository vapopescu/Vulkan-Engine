#ifndef VKC_INSTANCE_H
#define VKC_INSTANCE_H

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

#include <QWidget>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QMatrix4x4>
#if DEBUG == 1
#include <QDebug>
#endif

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
 * Class used as the Vulkan instance.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcInstance : public QObject
{
    Q_OBJECT

    //Objects:
private:
    VkInstance                  instance;
    VkSurfaceKHR                surface;
    QVector<VkcDevice*>         devices;
    VkcSwapchain                *swapchain;
    VkcPipeline                 *pipeline;


    VkcBuffer                   *uniformBuffer;
    VkcBuffer                   *presentBuffer;
    void                        *pPresentBuffer;
    VkcCamera                   *camera;

    uint32_t                    width;
    uint32_t                    height;

    VkSemaphore                 sphAcquire;
    VkSemaphore                 sphRender;
    VkFence                     fence;

    VkcEntity                   *square;

    VkDebugReportCallbackEXT    debugReport;

#if DEBUG == 1
    PROC(CreateDebugReportCallbackEXT);
    PROC(DestroyDebugReportCallbackEXT);
#endif


    //Functions:
public:
    VkcInstance(QWidget *parent = 0);
    ~VkcInstance();

    void render();
    void printDevices(
            QFile               *file
            );

private:
    void createInstance();
    void createSurface(
            uint32_t            id
            );
    void getDevices();

    void setupRender(
            const VkcDevice     *device
            );
    void unsetupRender(
            const VkcDevice     *device
            );
    void render(
            const VkcDevice     *device,
            const VkcSwapchain  *swapchain
            );
};

#endif // VKC_INSTANCE_H
