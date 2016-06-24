#include "vkc_device.h"


/**
 * Initialize with empty fields.
 */
VkcDevice::VkcDevice()
{
    physical =          VK_NULL_HANDLE;
    logical =           VK_NULL_HANDLE;
    commandPool =       VK_NULL_HANDLE;

    queueFamilies =     {};
    commandBuffers =    {};

    properties =        {};
    features =          {};
    memoryProperties =  {};
}


/**
 * Initialize and create device.
 */
VkcDevice::VkcDevice(uint32_t commandBufferNumber, VkSurfaceKHR surface)
{
    VkcDevice();
    create(commandBufferNumber, surface);
}


/**
 * We will handle the cleanup ourselves.
 */
VkcDevice::~VkcDevice()
{
    //destroy();
}


/**
 * Create the device.
 */
void VkcDevice::create(uint32_t commandBufferNumber, VkSurfaceKHR surface)
{
    //Get the number of queue properties.
    uint32_t familyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &familyCount, NULL);

    //Get the queue property list.
    QVector<VkQueueFamilyProperties> queueProperties;
    queueProperties.resize(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &familyCount, queueProperties.data());

    for(int i = 0; i < queueProperties.size(); i++)
    {
        VkBool32 ok;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, surface, &ok);

        //If queue properties are ok, add it to the list.
        if((queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && ok)
        {
            VkcQueueFamily queueFamilyStruct;
            queueFamilyStruct.index = i;
            queueFamilyStruct.properties = queueProperties[i];

            queueFamilies.append(queueFamilyStruct);
        }
    }

    uint32_t queueCount = queueFamilies[ACTIVE_FAMILY].properties.queueCount;
    uint32_t activeFamily = queueFamilies[ACTIVE_FAMILY].index;

    //Set queue priorities.
    QVector<float> queuePriorities;
    for (uint32_t i = 0; i < queueCount; i++)
        queuePriorities.append(1.0f);

    //Fill device queue info.
    VkDeviceQueueCreateInfo queueInfo =
    {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, //VkStructureType             sType;
        NULL,                                       //const void*                 pNext;
        0,                                          //VkDeviceQueueCreateFlags    flags;

        activeFamily,                               //uint32_t                    queueFamilyIndex;
        (uint32_t)queuePriorities.size(),           //uint32_t                    queueCount;
        queuePriorities.data()                      //const float*                pQueuePriorities;
    };

    //Fill device info.
    VkDeviceCreateInfo deviceInfo =
    {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,           //VkStructureType                    sType;
        NULL,                                           //const void*                        pNext;
        0,                                              //VkDeviceCreateFlags                flags;

        1,                                              //uint32_t                           queueCreateInfoCount;
        &queueInfo,                                     //const VkDeviceQueueCreateInfo*     pQueueCreateInfos;

        (uint32_t)VkSettings::deviceLayers.size(),      //uint32_t                           enabledLayerCount;
        VkSettings::deviceLayers.data(),                //const char* const*                 ppEnabledLayerNames;

        (uint32_t)VkSettings::deviceExtentions.size(),  //uint32_t                           enabledExtensionCount;
        VkSettings::deviceExtentions.data(),            //const char* const*                 ppEnabledExtensionNames;

        &features                                       //const VkPhysicalDeviceFeatures*    pEnabledFeatures;
    };

    //Create device.
    vkCreateDevice(physical, &deviceInfo, NULL, &logical);

    //Get queues.
    queueFamilies[ACTIVE_FAMILY].queues.resize(queueCount);
    for (uint32_t i = 0; i < queueCount; i++)
        vkGetDeviceQueue(logical, activeFamily, i, &queueFamilies[ACTIVE_FAMILY].queues[i]);

    //Fill command pool info.
    VkCommandPoolCreateInfo commandPoolCreateInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,         //VkStructureType             sType;
        NULL,                                               //const void*                 pNext;
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,    //VkCommandPoolCreateFlags    flags;

        activeFamily                                        //uint32_t                    queueFamilyIndex;
    };

    //Create command pool.
    vkCreateCommandPool(logical, &commandPoolCreateInfo, NULL, &commandPool);

    //Fill command buffer allocation info.
    commandBuffers.resize(commandBufferNumber);
    VkCommandBufferAllocateInfo commandBufferAllocateInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, //VkStructureType         sType;
        NULL,                                           //const void*             pNext;

        commandPool,                                    //VkCommandPool           commandPool;
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                //VkCommandBufferLevel    level;
        (uint32_t)commandBuffers.size()                 //uint32_t                commandBufferCount;
    };

    //Allocate command buffers.
    vkAllocateCommandBuffers(logical, &commandBufferAllocateInfo, commandBuffers.data());
}


/**
 * Destroy the device.
 */
void VkcDevice::destroy()
{
    if (commandBuffers.size() > 0)
    {
        vkFreeCommandBuffers(logical, commandPool, commandBuffers.size(), commandBuffers.data());
        commandBuffers.clear();
    }

    if (commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(logical, commandPool, NULL);
        commandPool = VK_NULL_HANDLE;
    }

    queueFamilies.clear();

    if (logical != VK_NULL_HANDLE)
    {
        vkDestroyDevice(logical, NULL);
        logical = VK_NULL_HANDLE;
    }
}
