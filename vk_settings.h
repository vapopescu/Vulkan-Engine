#ifndef VK_SETTINGS_H
#define VK_SETTINGS_H


#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 1

#elif __ANDROID__
#define VK_USE_PLATFORM_ANDROID_KHR 1

#else
#define VK_USE_PLATFORM_XCB_KHR 1

#endif

#include <vulkan.h>


#ifndef DEBUG
#define DEBUG 0
#endif

#define PROC(NAME) PFN_vk##NAME pf##NAME = NULL
#define GET_IPROC(INSTANCE, NAME) pf##NAME = (PFN_vk##NAME)vkGetInstanceProcAddr(INSTANCE, "vk" #NAME)
#define GET_DPROC(INSTANCE, NAME) pf##NAME = (PFN_vk##NAME)vkGetDeviceProcAddr(INSTANCE, "vk" #NAME)

#define ACTIVE_DEVICE 0
#define ACTIVE_FAMILY 0

typedef enum Resolution{
    WIDTH = 1600,
    HEIGHT = 900
} Resolution;

#define SAMPLE_COUNT VK_SAMPLE_COUNT_1_BIT


#include <QVector>

namespace VkSettings
{
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

//Setup device layers and extentions.
static QVector<const char*> deviceLayers =
{

};

static QVector<const char*> deviceExtentions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//Setup clear color.
static VkClearColorValue clearColor = {0.8f, 0.8f, 0.9f, 1.1f};
}

#endif // VK_SETTINGS_H
