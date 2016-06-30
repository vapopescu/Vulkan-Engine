#ifndef VKC_IMAGE_H
#define VKC_IMAGE_H

#include <QImage>
#if DEBUG == 1
#include <QDebug>
#endif

#include <vulkan.h>

#include <vkc_device.h>
#include <vkc_buffer.h>


/**
 * Class used for image handling.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcImage
{
    //Objects:
public:
    VkImage                     handle;
    VkImageView                 view;
    VkSampler                   sampler;

    VkImageType                 type;
    VkExtent3D                  extent;
    VkFormat                    format;
    VkImageLayout               layout;
    VkImageUsageFlags           usage;
    VkImageSubresourceRange     resourceRange;

protected:
    VkcBuffer                   *buffer;
    VkDeviceMemory              memory;
    VkDevice                    logicalDevice;

    //Functions:
public:
    VkcImage();
    ~VkcImage();

    void loadData(
            QImage              image,
            const VkcDevice     *device
            );
    void getData(
            VkcBuffer           *buffer,
            VkCommandBuffer     commandBuffer
            );
    void changeLayout(
            VkImageLayout       oldLayout,
            VkImageLayout       newLayout,
            VkCommandBuffer     commandBuffer
            );

protected:
    void createImage(
            const VkcDevice     *device
            );
    void prepareImage(
            const VkcDevice     *device
            );
    void createView();
    void createSampler();

private:
    void getAccessMask(
            VkAccessFlags       &accessMask,
            VkImageLayout       layout
            );
};


/**
 * Subclass used for swapchain color images.
 */
class VkcColorImage : public VkcImage
{
public:
    VkcColorImage() : VkcImage()
    {
        type =              VK_IMAGE_TYPE_2D;
        layout =            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        resourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,      //VkImageAspectFlags    aspectMask;
            0,                              //uint32_t              baseMipLevel;
            1,                              //uint32_t              levelCount;
            0,                              //uint32_t              baseArrayLayer;
            1,                              //uint32_t              layerCount;
        };
    }

    VkcColorImage(
            VkImage             handle,
            VkExtent3D          extent,
            VkFormat            format,
            VkDevice            logicalDevice
            ) : VkcColorImage()
    {
        this->handle =          handle;
        this->extent =          extent;
        this->format =          format;
        this->logicalDevice =   logicalDevice;

        createView();
    }
};


/**
 * Subclass used for swapchain depth images.
 */
class VkcDepthImage : public VkcImage
{
public:
    VkcDepthImage() : VkcImage()
    {
        type =              VK_IMAGE_TYPE_2D;
        format =            VK_FORMAT_D24_UNORM_S8_UINT;
        layout =            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        usage =             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        resourceRange = {
            VK_IMAGE_ASPECT_DEPTH_BIT |     //VkImageAspectFlags    aspectMask;
            VK_IMAGE_ASPECT_STENCIL_BIT,
            0,                              //uint32_t              baseMipLevel;
            1,                              //uint32_t              levelCount;
            0,                              //uint32_t              baseArrayLayer;
            1,                              //uint32_t              layerCount;
        };
    }

    VkcDepthImage(VkExtent3D extent, const VkcDevice *device) : VkcDepthImage()
    {
        this->extent =      extent;
        logicalDevice =     device->logical;

        createImage(device);
        prepareImage(device);
        createView();
    }
};


/**
 * Subclass used for 2D textures.
 */
class VkcTexture2D : public VkcImage
{
public:
    VkcTexture2D() : VkcImage()
    {
        type =              VK_IMAGE_TYPE_2D;
        format =            VK_FORMAT_R8G8B8A8_UNORM;
        layout =            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        usage =             0 |
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT;

        resourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,      //VkImageAspectFlags    aspectMask;
            0,                              //uint32_t              baseMipLevel;
            1,                              //uint32_t              levelCount;
            0,                              //uint32_t              baseArrayLayer;
            1,                              //uint32_t              layerCount;
        };
    }

    VkcTexture2D(QImage image, const VkcDevice *device) : VkcTexture2D()
    {
        extent.width =      image.width();
        extent.height =     image.height();
        extent.depth =      1;
        logicalDevice =     device->logical;

        loadData(image, device);
        createImage(device);
        prepareImage(device);
        createView();
        createSampler();
    }
};

#endif // VKC_IMAGE_H
