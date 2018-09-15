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
    MgImageType                 type;
    QString                     filePath;
    VkExtent3D                  extent;
    VkFormat                    format;

    VkImage                     image;
    VkBool32                    createView;
    VkBool32                    createSampler;
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
    VkImageLayout               layout =        VK_IMAGE_LAYOUT_UNDEFINED;

protected:
    VkImageType                 imageType =     VK_IMAGE_TYPE_2D;
    VkImageUsageFlags           usageMask =     0;
    VkImageAspectFlags          aspectMask =    0;

    uint32_t                    mipLevels =     0;
    QVector<VkBufferImageCopy>  regions =       {};

    VkDeviceMemory              memory =        VK_NULL_HANDLE;

    const VkcDevice             *pDevice =      nullptr;

    // Functions:
public:
    VkResult create(
            const VkcDevice     *pDevice,
            MgImageInfo         *pCreateInfo
            );
    VkResult createView();
    VkResult createSampler();
    void destroy();

    void changeLayout(
            VkImageLayout       oldLayout,
            VkImageLayout       newLayout,
            VkCommandBuffer     commandBuffer
            );

protected:
    VkResult loadImage(
            MgBuffer            *pImageBuffer
            );
    VkResult loadImageData(
            MgBuffer            *pImageBuffer
            );

    void getImageData(
            MgBuffer            buffer,
            VkCommandBuffer     commandBuffer
            );
    void getAccessMask(
            VkAccessFlags       &accessMask,
            VkImageLayout       layout
            );
};

#endif // MGIMAGE_H
