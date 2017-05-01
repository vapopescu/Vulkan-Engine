#ifndef MGIMAGE_H
#define MGIMAGE_H

#include "stable.h"
#include "vkc_device.h"
#include "mgbuffer.h"


enum MgImageType
{
    MG_IMAGE_TYPE_SWAPCHAIN_COLOR,
    MG_IMAGE_TYPE_SWAPCHAIN_DEPTH_STENCIL,
    MG_IMAGE_TYPE_TEXTURE_2D
};

/**
 * Struct used for creating a MgImage.
 */
struct MgImageInfo
{
    VkImageType                 type;
    VkExtent3D                  extent;
    VkFormat                    format;
    VkImageLayout               layout;
    VkImageUsageFlags           usage;
    VkImageSubresourceRange     resourceRange;

    VkImage                     image;
    bool                        createView;
    bool                        createSampler;
};

/**
 * Class used for image handling.
 */
class MgImage
{
    // Objects:
public:
    VkImage                     handle =        VK_NULL_HANDLE;
    VkImageView                 view =          VK_NULL_HANDLE;
    VkSampler                   sampler =       VK_NULL_HANDLE;

    MgImageInfo                 info;

protected:
    MgBuffer                    imageBuffer;
    VkDeviceMemory              memory =        VK_NULL_HANDLE;

    bool                        sharedImage =   true;

    // Functions:
public:
    VkResult create(
            const VkcDevice     *pDevice,
            MgImageInfo         *pCreateInfo
            );
    void destroy(
            const VkcDevice     *pDevice
            );

    void getImageData(
            MgBuffer           imageBuffer,
            VkCommandBuffer     commandBuffer
            );
    void changeLayout(
            VkImageLayout       oldLayout,
            VkImageLayout       newLayout,
            VkCommandBuffer     commandBuffer
            );

protected:
    VkResult loadImage(
            const VkcDevice     *pDevice
            );
    void getAccessMask(
            VkAccessFlags       &accessMask,
            VkImageLayout       layout
            );
};

#endif // MGIMAGE_H
