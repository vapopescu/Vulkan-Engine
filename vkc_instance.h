#ifndef VKC_INSTANCE_H
#define VKC_INSTANCE_H

#include "stable.h"
#include "vkc_context.h"
#include "vkc_device.h"
#include "mgcamera.h"
#include "mgbuffer.h"
#include "vkc_entity.h"
#include "mgimage.h"

#define PROC(NAME) PFN_vk##NAME pf##NAME = nullptr
#define GET_IPROC(INSTANCE, NAME) pf##NAME = (PFN_vk##NAME)vkGetInstanceProcAddr(INSTANCE, "vk" #NAME)
#define GET_DPROC(INSTANCE, NAME) pf##NAME = (PFN_vk##NAME)vkGetDeviceProcAddr(INSTANCE, "vk" #NAME)

/**
 * Class used as the Vulkan instance.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcInstance : public QObject
{
    Q_OBJECT

    // Objects:
private:
    VkInstance                  instance;
    QVector<VkcDevice*>         devices;
    VkcContext                  *context;

    MgBuffer                    presentBuffer;
    void                        *pPresentBuffer;
    MgCamera                    camera;

    uint32_t                    width;
    uint32_t                    height;

    VkSemaphore                 sphAcquire;
    VkSemaphore                 sphRender;
    VkFence                     fence;

    VkcEntity                   sphere;

    VkDebugReportCallbackEXT    debugReport;

#ifdef QT_DEBUG
    PROC(CreateDebugReportCallbackEXT);
    PROC(DestroyDebugReportCallbackEXT);
#endif


    // Functions:
public:
    VkcInstance(QWidget *parent = 0);
    ~VkcInstance();

private:
    void createInstance();
    void getDevices();

public:
    void render();
    void resize();
    void printDevices(
            QFile               *file
            );

    void setupRender(
            const VkcDevice     *device
            );
    void unsetupRender(
            const VkcDevice     *device
            );
};

#endif // VKC_INSTANCE_H
